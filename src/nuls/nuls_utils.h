#ifndef NULS_UTILS_H
#define NULS_UTILS_H

#include <stdbool.h>
#include <inttypes.h>
#include "nuls_internals.h"
#include "nuls_context.h"
#include "os.h"
#include "../io.h"



/**
 * Gets a bigendian representation of the usable publicKey
 * @param publicKey the raw public key containing both coordinated for the elliptic curve
 * @param encoded result holder
 */
void nuls_compress_publicKey(cx_ecfp_public_key_t WIDE *publicKey, uint8_t *out_encoded);

/**
 * Derive address associated to the specific publicKey.
 * @param publicKey original publicKey
 * @param the chainId.
 * @param the addressType.
 * @param output address in ripemid160 format.
 * @param output address in base58 format.
 * @return number of base58 address bytes
 */
unsigned short nuls_public_key_to_encoded_base58(
        uint8_t WIDE *compressedPublicKey,
        uint16_t chainId,
        uint8_t addressType,
        uint8_t *out_address,
        uint8_t *out_addressBase58);

bool is_p2pkh_addr(uint8_t addr_type);

bool is_contract_addr(uint8_t addr_type);

bool is_p2sh_addr(uint8_t addr_type);

bool is_contract_tx(uint16_t tx_type);

/**
 * Derive encoded base58 associated to the specific address (chainId + addresstype + ripemid160).
 * @param chainId + addresstype + ripemid160 of compressedpubkey
 * @param the encoded base58 address.
 * @return number of base58 address bytes
 */
unsigned short nuls_address_to_encoded_base58(uint8_t WIDE *address, uint8_t *out_address);

/**
 * Reads the packet for Sign requests (tx and msg), sets the reqContext and patches the packet data values by skipping the header.
 * @param packet the  buffer of communication packet.
 * @return number of bytes read
 */
uint32_t setReqContextForSign(commPacket_t *packet);

/**
 * Reads the packet for getPubKey requests, sets the reqContext and patches the packet data values by skipping the header.
 * @param packet the  buffer of communication packet.
 * @return number of bytes read
 */
uint32_t setReqContextForGetPubKey(commPacket_t *packet);

/**
 * Kill Private key and reset all the contexts (reqContext, txContext, commContext, commPacket)
 */
void reset_contexts();

#endif