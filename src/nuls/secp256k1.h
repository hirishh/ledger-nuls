#include <inttypes.h>
#include <stdbool.h>
#include "os.h"

#define MAX_BIP32_PATH 10

/**
 *
 * @param bip32Path
 * @param bip32PathLength
 * @param privateKey
 * @param publicKey
 * @return read data to derive private public
 */
void nuls_private_derive_keypair(
        uint32_t *bip32Path, uint8_t bip32PathLength,
        cx_ecfp_private_key_t *privateKey,
        cx_ecfp_public_key_t *publicKey,
        uint8_t *out_chainCode);

void nuls_bip32_buffer_to_array(
        uint8_t *bip32DataBuffer,
        uint8_t bip32PathLength,
        uint32_t *out_bip32Path);

/**
 * Signs or verify an arbitrary message given the privateKey and the info
 * @param privateKey: the privateKey to be used
 * @param in: the message to sign
 * @param inlen: the length of the message ot sign
 * @param out: destination buffer
 * @param outlen: destination buffer length
 */
void nuls_signverify_finalhash(
        cx_ecfp_private_key_t *privateKey, unsigned char sign,
        unsigned char *in, unsigned short inlen,
        unsigned char *out, unsigned short outlen);