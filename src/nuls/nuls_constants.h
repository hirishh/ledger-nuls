#ifndef NULS_CONSTANTS_H
#define NULS_CONSTANTS_H

#define NEED_NEXT_CHUNK 0x6866

#define MAX_BIP32_PATH 10
#define MAX_BIP32_PATH_LENGTH (4 * MAX_BIP32_PATH) + 1

#define MAX_OUTPUT_TO_CHECK 10


#define HASH_LENGTH 34
#define ADDRESS_LENGTH 23 // chainid (2) + addressType (1) + RIPEMID160 (20)
#define BASE58_ADDRESS_LENGTH 32

#define P2PKH_ADDRESS_TYPE 1
#define CONTRACT_ADDRESS_TYPE 2
#define P2SH_ADDRESS_TYPE 3

#define MAX_REMARK_LENGTH 30
#define MAX_ALIAS_LENGTH 20
#define AMOUNT_LENGTH 8
#define LOCKTIME_LENGTH 6
#define DIGEST_LENGTH 32

//Hash of address Nse5FeeiYk1opxdc5RqYpEWkiUDGNuLs
extern const unsigned char BLACK_HOLE_ADDRESS[ADDRESS_LENGTH];
extern const unsigned char BLACK_HOLE_ALIAS_AMOUNT[AMOUNT_LENGTH];

extern const unsigned char MIN_DEPOSIT_JOIN_CONSENSUS[AMOUNT_LENGTH];

#endif