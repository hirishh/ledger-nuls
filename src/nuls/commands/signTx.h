#ifndef PROJECT_SIGNTX_H
#define PROJECT_SIGNTX_H

#include "../../io.h"
#include "../nuls_utils.h"

// typedef void (*transaction_chunk_handler)(commPacket_t packet);

#define NEED_NEXT_CHUNK 0x6866

#define REMARK_LENGTH 30
#define HASH_LENGTH 34
#define ADDRESS_LENGTH 23 // chainid (2) + addressType (1) + RIPEMID160 (20)
#define AMOUNT_LENGTH 8
#define LOCKTIME_LENGTH 6
#define DIGEST_LENGTH 32

/**
 * Current state of an untrusted transaction hashing
 */

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

extern transaction_context_t txContext;


typedef void (*ui_processor_fn)(uint8_t curStep);
typedef uint8_t (*step_processor_fn)(uint8_t curStep);
extern step_processor_fn step_processor;
extern ui_processor_fn ui_processor;

void handleSignTxPacket(commPacket_t *packet, commContext_t *context);
void finalizeSignTx(volatile unsigned int *flags);

#endif //PROJECT_SIGNTX_H
