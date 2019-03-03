#ifndef SECP256K1_H
#define SECP256K1_H

#include <inttypes.h>
#include <stdbool.h>
#include "os.h"

/**
 * @param bip32Path
 * @param bip32PathLength
 * @param privateKey
 * @param publicKey
 * @param out_chainCode
 * @return read data to derive private public
 */
void nuls_private_derive_keypair(
        uint32_t WIDE *bip32Path, uint8_t bip32PathLength,
        uint8_t *out_chainCode);

/**
 * @param bip32PathBuffer
 * @param bip32PathLength
 * @param out_bip32PathArray
 * @return read data to derive private public
 */
void nuls_bip32_buffer_to_array(
        uint8_t *bip32DataBuffer,
        uint8_t bip32PathLength,
        uint32_t *out_bip32Path);

/**
 * Signs or verify an arbitrary message given the privateKey and the info
 * @param privateKey: the privateKey to be used
 * @param sign: 1 = sign, 0 = verify
 * @param in: the message to sign
 * @param inlen: the length of the message ot sign
 * @param out: destination buffer
 * @param outlen: destination buffer length
 * @return signLength if signMode, 0 or 1 if verify mode
 */
unsigned short nuls_signverify_finalhash(
        void WIDE *keyContext, unsigned char sign,
        unsigned char WIDE *in, unsigned int inlen,
        unsigned char *out, unsigned int outlen);

#endif