#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "os.h"
#include "cx.h"
#include "nuls_internals.h"
#include "../io.h"

void nuls_compress_publicKey(cx_ecfp_public_key_t *publicKey, uint8_t *out_encoded) {
  os_memmove(out_encoded, publicKey->W, 33);
  out_encoded[0] = ((publicKey->W[64] & 1) ? 0x03 : 0x02);
}

void nuls_public_key_hash160(unsigned char *in, unsigned short inlen, unsigned char *out) {
  union {
      cx_sha256_t shasha;
      cx_ripemd160_t riprip;
  } u;
  unsigned char buffer[32];

  cx_sha256_init(&u.shasha);
  cx_hash(&u.shasha.header, CX_LAST, in, inlen, buffer, 32);
  cx_ripemd160_init(&u.riprip);
  cx_hash(&u.riprip.header, CX_LAST, buffer, 32, out, 20);
}

uint8_t getxor(uint8_t *buffer, uint8_t length) {
  uint8_t xor = 0;
  for (int i = 0; i < length; i++) {
    xor = xor ^ buffer[i];
  }
  return xor;
}

unsigned short nuls_public_key_to_encoded_base58 (
        uint8_t *compressedPublicKey,
        uint16_t chainId, uint8_t addressType,
        uint8_t *out_address) {

  unsigned char tmpBuffer[23];
  tmpBuffer[0] = chainId;
  tmpBuffer[1] = (chainId >> 8);
  tmpBuffer[2] = addressType;

//  PRINTF("RAW PubKey %.*H\n", 65, &reqContext.publicKey->W);
//  PRINTF("COMPRESSED %.*H\n", 33, compressedPublicKey);
  nuls_public_key_hash160(compressedPublicKey, 33, tmpBuffer + 3);
  return nuls_address_to_encoded_base58(tmpBuffer, out_address);
}

unsigned short nuls_address_to_encoded_base58(
        uint8_t *nulsRipemid160, //chainId + addresstype + ripemid160
        uint8_t *out_address) {
  unsigned char tmpBuffer[24];
  os_memmove(tmpBuffer, nulsRipemid160, 23);
  //  PRINTF("tmpbuffer0,1,2 %.*H\n", 3, tmpBuffer);
  //  PRINTF("hash %.*H\n", 23, tmpBuffer);
  tmpBuffer[23] = getxor(tmpBuffer, 23);
  //  PRINTF("XOR %d\n", tmpBuffer[23]);
  size_t outputLen = 32;
  if (nuls_encode_base58(tmpBuffer, 24, out_address, &outputLen) < 0) {
    THROW(EXCEPTION);
  }
  //  PRINTF("Base58 %s\n", out_address);
  return outputLen;
}

void resetAccountInfo(local_address_t *account) {
  os_memset(&(account->privateKey), 0, sizeof(account->privateKey));
  os_memset(&(account->publicKey), 0, sizeof(account->publicKey));
  os_memset(account->compressedPublicKey, 0, sizeof(account->compressedPublicKey));
  os_memset(account->chainCode, 0, sizeof(account->chainCode));
  account->chainId = 0;
  account->pathLength = 0;
  account->type = 0;
  os_memset(account->path, 0, sizeof(account->path));
  os_memset(account->address, 0, sizeof(account->address));
}

uint32_t extractAccountInfo(uint8_t *data, local_address_t *account) {

  //Reset
  resetAccountInfo(account);

  uint32_t readCounter = 0;

  //PathLength
  account->pathLength = data[0];
  readCounter++;
  PRINTF("account->pathLength %d\n", account->pathLength);

  if(account->pathLength == 0) {
    return readCounter;
  }

  if(account->pathLength > MAX_BIP32_PATH) {
    THROW(INVALID_PARAMETER);
  }

  //AddressType
  account->type = data[1];
  readCounter++;
  PRINTF("account->type %d\n", account->type);
  //TODO Implement P2SH. At the moment is not supported
  if(account->type != 0x01) {
    THROW(NOT_SUPPORTED);
  }

  //Path
  uint8_t bip32buffer[45];
  os_memmove(bip32buffer, data + 2, account->pathLength * 4);
  readCounter += account->pathLength * 4;
  PRINTF("bip32buffer %.*H\n", account->pathLength * 4, bip32buffer);
  nuls_bip32_buffer_to_array(bip32buffer, account->pathLength, account->path);

  for(unsigned int i=0; i < MAX_BIP32_PATH; i++) {
    PRINTF("account->path[%u] -> %u\n", i, account->path[i]);
  }

  //Set chainId from account->path[1]
  account->chainId = (uint16_t) account->path[1];
  PRINTF("account->chainId %d\n", account->chainId);

  // Derive pubKey
  nuls_private_derive_keypair(account->path, account->pathLength,
                              &account->privateKey, &account->publicKey, account->chainCode);
  //Paranoid
  os_memset(&account->privateKey, 0, sizeof(account->privateKey));
  //Gen Compressed PubKey
  nuls_compress_publicKey(&account->publicKey, account->compressedPublicKey);
  //Compressed PubKey -> Address
  nuls_public_key_to_encoded_base58(account->compressedPublicKey, account->chainId,
                                    account->type, account->address);
  account->address[32] = '\0';
  PRINTF("account->address %s\n", account->address);


  return readCounter;
}

void setReqContextForSign(commPacket_t *packet) {
  reqContext.signableContentLength = 0;

  uint32_t headerBytesRead = 0;
  //AccountFrom
  headerBytesRead += extractAccountInfo(packet->data, &reqContext.accountFrom);
  //AccountChange
  headerBytesRead += extractAccountInfo(packet->data + headerBytesRead, &reqContext.accountChange);

  //Check chainId is the same if (any) change address
  if(reqContext.accountChange.pathLength > 0 && (reqContext.accountChange.chainId != reqContext.accountFrom.chainId)) {
    THROW(INVALID_PARAMETER);
  }

  //Data Length
  reqContext.signableContentLength = nuls_read_u16(packet->data + headerBytesRead, 1, 0);
  PRINTF("reqContext.signableContentLength %d\n", reqContext.signableContentLength);
  // Check signable content length if is correct
  if (reqContext.signableContentLength >= commContext.totalAmount) {
    THROW(0x6700); // INCORRECT_LENGTH
  }
  headerBytesRead += 2;

  // clean up packet->data by removing the consumed content (sign context)
  uint8_t tmpBuffer[256];
  os_memmove(tmpBuffer, packet->data + headerBytesRead, packet->length - headerBytesRead);
  os_memmove(packet->data, tmpBuffer, packet->length - headerBytesRead);
  packet->length = packet->length - headerBytesRead;
  PRINTF("packet->length %d\n", packet->length);
  PRINTF("packet-data %.*H\n", packet->length, &packet->data);
  PRINTF("First byte packet %d\n", packet->data[0]);
  PRINTF("Second byte packet %d\n", packet->data[1]);
}

void setReqContextForGetPubKey(commPacket_t *packet) {
  reqContext.showConfirmation = packet->data[0];
  extractAccountInfo(packet->data + 1, &reqContext.accountFrom);
}