#ifndef NULS_CONTEXT_H
#define NULS_CONTEXT_H

#include "nuls_internals.h"
#include "os.h"
#include "cx.h"

/**
 * Request Context
 */

typedef struct local_address {
    uint8_t compressedPublicKey[33];
    uint32_t path[MAX_BIP32_PATH];
    uint8_t pathLength;
    uint8_t chainCode[32];
    uint16_t chainId;
    uint8_t type;
    uint8_t address[33];
} local_address_t;

typedef struct request_context {
    uint8_t showConfirmation;

    local_address_t accountFrom;
    local_address_t accountChange;

    //For signature
    uint16_t signableContentLength;
} request_context_t;


/**
 * Transaction Context
 */

/** Current state of an untrusted transaction hashing */
enum transaction_parsing_group_e {
    /** No transaction in progress */
    COMMON = 0x00,
    TX_SPECIFIC = 0x01,
    COIN_INPUT = 0x02,
    COIN_OUTPUT = 0x03,
    CHECK_SANITY_BEFORE_SIGN = 0x04,
    TX_PARSED = 0x05
};
typedef enum transaction_parsing_group_e transaction_parsing_group_t;

enum transaction_parsing_state_e {
    /** No transaction in progress. Used also as group start*/
    BEGINNING = 0x00,
    /** Commmon Fields */
    FIELD_TYPE = 0x01,
    FIELD_TIME = 0x02,
    FIELD_REMARK_LENGTH = 0x03,
    FIELD_REMARK = 0x04,
    /** Data Fields - TX Specifics */
    PLACEHOLDER = 0x05,
    //TODO for other TXs
    /** COIN: Input & Output */
    COIN_OWNER_DATA_LENGTH = 0x20,
    COIN_DATA = 0x21,
    /** Ready to be signed */
    READY_TO_SIGN = 0x30
};
typedef enum transaction_parsing_state_e transaction_parsing_state_t;

typedef struct transaction_context {

    /** Full transaction hash context */
    cx_sha256_t txHash;

    /** Holds digest to sign */
    uint8_t digest[32];

    /** Type of the transaction */
    uint16_t type;

    /** Group of the transaction parsing, type transaction_parsing_group_t */
    uint8_t tx_parsing_group;

    /** State of the transaction parsing, type transaction_parsing_state_t */
    uint8_t tx_parsing_state;

    /** Remaining number of inputs/outputs to process for this transaction */
    unsigned long int remainingInputsOutputs;

    /** Index of the currently processed input/output for this transaction */
    unsigned long int currentInputOutput;

    /** Length of the currently processed input/output for this transaction */
    unsigned long int currentInputOutputOwnerLength;

    /** Computed sum of transaction inputs */
    unsigned char totalInputAmount[8];

    /** Computed sum of transaction outputs */
    unsigned char totalOutputAmount[8];

    /** Bytes parsed */
    uint16_t bytesRead;

    /** Bytes to be parsed (in the chunk) */
    unsigned char bytesChunkRemaining;

    /** Current pointer to the transaction buffer for the transaction parser */
    unsigned char *bufferPointer;

    /** Bytes to be parsed (in the next chunk) */
    unsigned char saveBufferForNextChunk[250];
    uint16_t saveBufferLength;

    /** Total Tx Bytes */
    uint16_t totalTxBytes;

    /** Fields to Display  */
    unsigned char remark[REMARK_LENGTH];
    unsigned char remarkSize;
    unsigned char fees[AMOUNT_LENGTH];
    unsigned char outputAddress[ADDRESS_LENGTH];
    unsigned char outputAmount[AMOUNT_LENGTH];
    unsigned char changeAddress[ADDRESS_LENGTH];
    unsigned char changeAmount[AMOUNT_LENGTH];

} transaction_context_t;

#endif
