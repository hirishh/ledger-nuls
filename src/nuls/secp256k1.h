#include <inttypes.h>
#include <stdbool.h>
#include "os.h"

#define MAX_BIP32_PATH 10

/**
 * Derive private and public from bip32
 */
void nuls_private_derive_keypair(uint32_t *bip32Path, uint8_t bip32PathLength,
        cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey, uint8_t *out_chainCode);

void nuls_bip32_buffer_to_array(uint8_t *bip32DataBuffer, uint8_t bip32PathLength, uint32_t *out_bip32Path);

void sign(cx_ecfp_private_key_t *privateKey, void *whatToSign, uint32_t length, unsigned char *output);