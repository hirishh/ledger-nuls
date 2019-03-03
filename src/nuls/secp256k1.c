#include "stdbool.h"
#include "os.h"
#include "cx.h"
#include "secp256k1.h"
#include "nuls_internals.h"

#define LIBN_CURVE CX_CURVE_256K1

void nuls_private_derive_keypair(uint32_t WIDE *bip32Path, uint8_t bip32Length, uint8_t *out_chainCode) {
  uint8_t privateKeyData[32];
  os_perso_derive_node_bip32(LIBN_CURVE, bip32Path, bip32Length, privateKeyData, out_chainCode);

  BEGIN_TRY {
    TRY {
        cx_ecdsa_init_private_key(LIBN_CURVE, privateKeyData, 32, &private_key);
        cx_ecfp_generate_pair(LIBN_CURVE, &public_key, &private_key, 1);
      }
    FINALLY {}
  }
  END_TRY;

  // Clean up!
  os_memset(privateKeyData, 0, sizeof(privateKeyData));
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

unsigned short nuls_signverify_finalhash(
        void WIDE *keyContext, unsigned char sign,
        unsigned char WIDE *in, unsigned int inlen,
        unsigned char *out, unsigned int outlen) {
  int result = 0;
  if(sign) {
    unsigned int info = 0;
    result = cx_ecdsa_sign((cx_ecfp_private_key_t WIDE *) keyContext, CX_LAST | CX_RND_RFC6979, CX_SHA256, in, inlen, out, outlen, &info);
    if (info & CX_ECCINFO_PARITY_ODD) {
      out[0] |= 0x01;
    }
  } else {
    result = cx_ecdsa_verify((cx_ecfp_public_key_t WIDE *) keyContext, CX_LAST, CX_SHA256, in, inlen, out, outlen);
  }
  return result;
}
