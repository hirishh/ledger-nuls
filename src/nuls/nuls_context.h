#ifndef NULS_CONTEXT_H
#define NULS_CONTEXT_H

#include "nuls_constants.h"
#include "stdbool.h"
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
    uint8_t address[23];
    uint8_t addressBase58[33];
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
    _3_ALIAS_ADDRESS_LENGTH = 0x06,
    _3_ALIAS_ADDRESS = 0x07,
    _3_ALIAS_ALIAS_LENGTH = 0x08,
    _3_ALIAS_ALIAS = 0x09,
    _5_JOIN_CONS_DEPOSIT = 0x10,
    _5_JOIN_CONS_ADDRESS = 0x11,
    _5_JOIN_CONS_AGENTHASH = 0x12,
    _6_LEAVE_CONS_TXHASH = 0x13,
    //TODO for other TXs
    /** COIN: Input & Output */
    COIN_OWNER_DATA_LENGTH = 0x20,
    COIN_DATA = 0x21,
    /** Ready to be signed */
    READY_TO_SIGN = 0x30
};
typedef enum transaction_parsing_state_e transaction_parsing_state_t;

typedef struct tx_type_specific_3_alias {
    unsigned char address[ADDRESS_LENGTH];
    unsigned char alias[MAX_ALIAS_LENGTH];
    unsigned char aliasSize;
} tx_type_specific_3_alias_t;

typedef struct tx_type_specific_5_join_consensus {
    unsigned char deposit[AMOUNT_LENGTH];
    unsigned char address[ADDRESS_LENGTH];
    unsigned char agentHash[HASH_LENGTH];
} tx_type_specific_5_join_consensus_t;

typedef struct tx_type_specific_6_leave_consensus {
    unsigned char txHash[HASH_LENGTH];
} tx_type_specific_6_leave_consensus_t;

typedef union tx_specific_fields {
    tx_type_specific_3_alias_t alias;
    tx_type_specific_5_join_consensus_t join_consensus;
    tx_type_specific_6_leave_consensus_t leave_consensus;
} tx_specific_fields_t;

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
    unsigned char totalInputAmount[AMOUNT_LENGTH];

    /** Computed sum of transaction outputs */
    unsigned char totalOutputAmount[AMOUNT_LENGTH];

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

    /** TX Specific fields **/
    tx_specific_fields_t tx_specific_fields;

    unsigned char remark[MAX_REMARK_LENGTH];
    unsigned char remarkSize;

    uint8_t nOut;
    unsigned char outputAddress[MAX_OUTPUT_TO_CHECK][ADDRESS_LENGTH];
    unsigned char outputAmount[MAX_OUTPUT_TO_CHECK][AMOUNT_LENGTH];
    uint8_t nOutCursor; //Used during UX

    bool changeFound;
    unsigned char changeAmount[AMOUNT_LENGTH];

    unsigned char fees[AMOUNT_LENGTH];
    unsigned char amountSpent[AMOUNT_LENGTH];

} transaction_context_t;

#endif
