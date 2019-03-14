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
static unsigned char  BLACK_HOLE_ADDRESS[ADDRESS_LENGTH] =
        { 0x04, 0x23, 0x01, 0xb5, 0x0e, 0x18, 0xcb, 0xcd, 0x18, 0x91, 0x49, 0x94, 0x50,
          0xf1, 0x25, 0xde, 0x50, 0x85, 0x1b, 0xd0, 0x9a, 0x6d, 0xac};
static unsigned char BLACK_HOLE_ALIAS_AMOUNT[AMOUNT_LENGTH] =
        { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};




#endif
