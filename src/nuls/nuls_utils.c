#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "nuls_utils.h"
#include "os.h"
#include "cx.h"


void nuls_compress_publicKey(cx_ecfp_public_key_t WIDE *publicKey, uint8_t *out_encoded) {
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


void printAccountInfo(local_address_t *account) {
  PRINTF("account->pathLength %d\n", account->pathLength);
  if(account->pathLength == 0) {
    return;
  }
  PRINTF("account->type %d\n", account->type);
  for(unsigned int i=0; i < account->pathLength; i++) {
    PRINTF("account->path[%u] -> %u\n", i, account->path[i]^0x80000000);
  }
  PRINTF("account->chainId %d\n", account->chainId);
  PRINTF("account->address %s\n", account->address);
}

uint32_t extractAccountInfo(uint8_t *data, local_address_t *account) {
  uint32_t readCounter = 0;

  //PathLength
  account->pathLength = data[0];
  readCounter++;

  if(account->pathLength == 0) {
    return readCounter;
  }

  if(account->pathLength > MAX_BIP32_PATH) {
    THROW(INVALID_PARAMETER);
  }

  //AddressType
  account->type = data[1];
  readCounter++;

  //TODO Implement P2SH. At the moment is not supported
  if(account->type != 0x01) {
    THROW(NOT_SUPPORTED);
  }

  //Path
  nuls_bip32_buffer_to_array(data + 2, account->pathLength, account->path);
  readCounter += account->pathLength * 4;

  //Set chainId from account->path[1]
  account->chainId = (uint16_t) account->path[1]^0x80000000;

  deriveAccountAddress(account);

  return readCounter;
}

uint32_t setReqContextForSign(commPacket_t *packet) {
  reqContext.signableContentLength = 0;
  uint32_t headerBytesRead = 0;

  //AccountFrom
  headerBytesRead += extractAccountInfo(packet->data, &(reqContext.accountFrom));
  printAccountInfo(&(reqContext.accountFrom));

  //AccountChange
  headerBytesRead += extractAccountInfo(packet->data + headerBytesRead, &(reqContext.accountChange));
  printAccountInfo(&(reqContext.accountChange));


  //Check chainId is the same if (any) change address
  if(reqContext.accountChange.pathLength > 0 && (reqContext.accountChange.chainId != reqContext.accountFrom.chainId)) {
    THROW(INVALID_PARAMETER);
  }

  //Data Length
  reqContext.signableContentLength = nuls_read_u16(packet->data + headerBytesRead, 1, 0);
  headerBytesRead += 2;
  PRINTF("reqContext.signableContentLength %d\n", reqContext.signableContentLength);
  // Check signable content length if is correct
  if (reqContext.signableContentLength >= commContext.totalAmount) {
    THROW(0x6700); // INCORRECT_LENGTH
  }

  PRINTF("packet->length %d\n", packet->length - headerBytesRead);
  PRINTF("packet-data %.*H\n", packet->length - headerBytesRead, &packet->data + headerBytesRead);
  return headerBytesRead;
}

uint32_t setReqContextForGetPubKey(commPacket_t *packet) {
  reqContext.showConfirmation = packet->data[0];
  return extractAccountInfo(packet->data + 1, &reqContext.accountFrom);
}


void deriveAccountAddress(local_address_t *account) {

  // Derive pubKey
  nuls_private_derive_keypair(account->path, account->pathLength, account->chainCode);
  //Paranoid
  os_memset(&private_key, 0, sizeof(private_key));
  //Gen Compressed PubKey
  nuls_compress_publicKey(&public_key, account->compressedPublicKey);

  //Compressed PubKey -> Address
  nuls_public_key_to_encoded_base58(account->compressedPublicKey, account->chainId,
                                    account->type, account->address);
  account->address[32] = '\0';
}