#include "stdbool.h"
#include "os.h"
#include "cx.h"
#include "secp256k1.h"

#define LIBN_CURVE CX_CURVE_SECP256K1

unsigned long int nuls_read_u32(unsigned char *buffer, unsigned char be, unsigned char skipSign) {
  unsigned char i;
  unsigned long int result = 0;
  unsigned char shiftValue = (be ? 24 : 0);
  for (i = 0; i < 4; i++) {
    unsigned char x = (unsigned char)buffer[i];
    if ((i == 0) && skipSign) {
      x &= 0x7f;
    }
    result += ((unsigned long int)x) << shiftValue;
    if (be) {
      shiftValue -= 8;
    } else {
      shiftValue += 8;
    }
  }
  return result;
}

/**
 *
 * @param bip32Path
 * @param bip32PathLength
 * @param privateKey
 * @param publicKey
 * @return read data to derive private public
 */
void nuls_private_derive_keypair(uint32_t *bip32Path, uint8_t bip32PathLength, cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey, uint8_t *out_chainCode) {
  uint8_t privateKeyData[32];
  os_perso_derive_node_bip32(LIBN_CURVE, bip32Path, bip32PathLength, privateKeyData, out_chainCode);

  BEGIN_TRY {
    TRY {
        cx_ecdsa_init_private_key(LIBN_CURVE, privateKeyData, 32, privateKey);
        cx_ecfp_generate_pair(LIBN_CURVE, publicKey, privateKey, 1);
      }
    FINALLY {}
  }
  END_TRY;

  // Clean up!
  os_memset(privateKeyData, 0, sizeof(privateKeyData));
  os_memset(privateKey, 0, sizeof(privateKey));
}

void nuls_bip32_buffer_to_array(uint8_t *bip32DataBuffer, uint8_t bip32PathLength, uint32_t *out_bip32Path) {
  if ((bip32PathLength < 0x01) || (bip32PathLength > MAX_BIP32_PATH)) {
    THROW(0x6a80 + bip32PathLength);
  }

  uint32_t i;
  for (i = 0; i < bip32PathLength; i++) {
    out_bip32Path[i] = nuls_read_u32(bip32DataBuffer, 1, 0);
    bip32DataBuffer += 4;
  }
}

/**
 * Signs an arbitrary message given the privateKey and the info
 * @param privateKey the privateKey to be used
 * @param whatToSign the message to sign
 * @param length the length of the message ot sign
 * @param isTx wether we're signing a tx or a text
 * @param output
 */
void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output) {
  // 2nd param was null
  cx_eddsa_sign(privateKey, 0, CX_SHA512, whatToSign, length, NULL, 0, output, CX_SHA512_SIZE, NULL);
}
