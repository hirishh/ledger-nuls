#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "os.h"
#include "cx.h"
#include "nuls_utils.h"
#include "nuls_helpers.h"
#include "nuls_base58.h"
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
        uint16_t chainId, uint8_t addressVersion,
        uint8_t *out_address) {

  unsigned char tmpBuffer[23];
  tmpBuffer[0] = chainId;
  tmpBuffer[1] = (chainId >> 8);
  tmpBuffer[2] = addressVersion;

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

uint32_t extractBip32Data(uint8_t *data) {
  os_memset(&reqContext.bip32path, 0, sizeof(reqContext.bip32path));
  reqContext.bip32pathLength = 0;

  //AddressType
  reqContext.addressVersion = data[0];
  PRINTF("addressVersion %d\n", reqContext.addressVersion);
  //TODO Implement P2SH. At the moment is not supported
  if(reqContext.addressVersion != 0x01) {
    THROW(NOT_SUPPORTED);
  }

  //Read bip32path
  reqContext.bip32pathLength = data[1];
  PRINTF("bip32pathLength %d\n", reqContext.bip32pathLength);
  uint8_t bip32DataBuffer[256];
  os_memmove(bip32DataBuffer, data + 2, reqContext.bip32pathLength * 4);
  PRINTF("bip32DataBuffer %.*H\n", reqContext.bip32pathLength * 4, bip32DataBuffer);
  nuls_bip32_buffer_to_array(bip32DataBuffer, reqContext.bip32pathLength, reqContext.bip32path);

  for(unsigned int i=0; i < MAX_BIP32_PATH; i++) {
    PRINTF("bip32path[%u] -> %u\n", i, reqContext.bip32path[i]);
  }

  //Set chainId from bip32path[1]
  reqContext.chainId = (uint16_t) reqContext.bip32path[1];
  PRINTF("reqContext.chainId %d\n", reqContext.chainId);

  // Derive pubKey
  nuls_private_derive_keypair(reqContext.bip32path, reqContext.bip32pathLength,
                              &reqContext.privateKey, &reqContext.publicKey, reqContext.chainCode);
  //Paranoid
  os_memset(&reqContext.privateKey, 0, sizeof(reqContext.privateKey));
  //Gen Compressed PubKey
  nuls_compress_publicKey(&reqContext.publicKey, reqContext.compressedPublicKey);
  //Compressed PubKey -> Address
  nuls_public_key_to_encoded_base58(reqContext.compressedPublicKey, reqContext.chainId,
                                    reqContext.addressVersion, reqContext.address);
  reqContext.address[32] = '\0';
  PRINTF("reqContext.address %s\n", reqContext.address);

  return /* AddressType */ 1 + /* pathLength */ 1 + reqContext.bip32pathLength * 4;
}

void setReqContextForSign(commPacket_t *packet) {

  //reset digest
  os_memset(&reqContext.digest, 0, 32);
  reqContext.signableContentLength = 0;

  uint32_t headerBytesRead = extractBip32Data(packet->data);

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
  extractBip32Data(packet->data + 1);
}