#include <stdbool.h>
#include "os.h"
#include <inttypes.h>
#include "../io.h"
#include "secp256k1.h"
#ifndef STRUCT_TX
#define STRUCT_TX

typedef struct reqContext_t {
    cx_ecfp_private_key_t privateKey;
    cx_ecfp_public_key_t publicKey;
    uint8_t compressedPublicKey[33];
    uint8_t chainCode[32];
    uint8_t bip32pathLength;
    uint32_t bip32path[MAX_BIP32_PATH];
    uint8_t showConfirmation;
    uint16_t chainId;
    uint8_t addressVersion;
    uint8_t address[33];
    uint16_t signableContentLength;
    uint8_t reserved;

    // Holds digest to sign
    uint8_t digest[32];
} reqContext_t;

extern reqContext_t reqContext;

#endif

/**
 * Gets a bigendian representation of the usable publicKey
 * @param publicKey the raw public key containing both coordinated for the elliptic curve
 * @param encoded result holder
 */
void nuls_compress_publicKey(cx_ecfp_public_key_t *publicKey, uint8_t *out_encoded);


/**
 * Derive address associated to the specific publicKey.
 * @param publicKey original publicKey
 * @param the encoded address.
 */
unsigned short nuls_public_key_to_encoded_base58(uint8_t *compressedPublicKey, uint16_t chainId, uint8_t addressVersion, uint8_t *out_address);

/**
 * Reads the packet for Sign requests (tx and msg), sets the reqContext and patches the packet data values by skipping the header.
 * @param packet the  buffer of communication packet.
 * @return the amount of bytesRead
 */
uint32_t setReqContextForSign(commPacket_t *packet);

/**
 * Reads the packet for getPubKey requests, sets the reqContext and patches the packet data values by skipping the header.
 * @param packet the  buffer of communication packet.
 */
void setReqContextForGetPubKey(commPacket_t *packet);
