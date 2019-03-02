#ifndef NULS_UTILS_H
#define NULS_UTILS_H

#include <stdbool.h>
#include "os.h"
#include <inttypes.h>
#include "../io.h"
#include "secp256k1.h"


/**
 * Gets a bigendian representation of the usable publicKey
 * @param publicKey the raw public key containing both coordinated for the elliptic curve
 * @param encoded result holder
 */
void nuls_compress_publicKey(cx_ecfp_public_key_t WIDE *publicKey, uint8_t *out_encoded);


/**
 * Derive address associated to the specific publicKey.
 * @param publicKey original publicKey
 * @param the encoded address.
 * @return number of address bytes
 */
unsigned short nuls_public_key_to_encoded_base58(uint8_t *compressedPublicKey, uint16_t chainId, uint8_t addressType, uint8_t *out_address);

/**
 * Derive encoded base58 associated to the specific address (chainId + addresstype + ripemid160).
 * @param chainId + addresstype + ripemid160 of compressedpubkey
 * @param the encoded address.
 * @return number of address bytes
 */
unsigned short nuls_address_to_encoded_base58(uint8_t *address, uint8_t *out_address);
/**
 * Reads the packet for Sign requests (tx and msg), sets the reqContext and patches the packet data values by skipping the header.
 * @param packet the  buffer of communication packet.
 */
void setReqContextForSign(commPacket_t *packet);

/**
 * Reads the packet for getPubKey requests, sets the reqContext and patches the packet data values by skipping the header.
 * @param packet the  buffer of communication packet.
 */
void setReqContextForGetPubKey(commPacket_t *packet);

void deriveAccountAddress(local_address_t WIDE *account);

#endif