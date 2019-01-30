#include "../../io.h"
#ifndef PROJECT_SIGNTX_H
#define PROJECT_SIGNTX_H

// typedef void (*transaction_chunk_handler)(commPacket_t packet);

#define NEED_NEXT_CHUNK 0x6866

#define ADDRESS_LENGTH 23
#define HASH_LENGTH 34

/**
 * Current state of an untrusted transaction hashing
 */

enum transaction_parsing_group_e {
    /** No transaction in progress */
    COMMON = 0x00,
    TX_SPECIFIC = 0x01,
    COIN_INPUT = 0x02,
    COIN_OUTPUT = 0x03,
    CHECK_SANITY_BEFORE_SIGN = 0x04
};
typedef enum transaction_parsing_group_e transaction_parsing_group_t;

enum transaction_parsing_state_e {
    /** No transaction in progress. Used also as group start*/
    BEGINNING = 0x00,
    /** Commmon Fields */
    FIELD_TYPE
    FIELD_TIME
    FIELD_REMARK_LENGTH
    FIELD_REMARK
    /** Data Fields - TX Specifics */
    PLACEHOLDER
    //TODO for other TXs
    /** COIN: Input & Output */
    INPUT_OWNER_DATA_LENGTH
    INPUT_DATA

    FIELD_COIN_OUTPUT_SIZE
    PARSE_COIN_DATA_LENGTH
    PARSE_COIN_DATA
    /** COIN: Input & Output */
    //TODO
    /** Ready to be signed */
    READY_TO_SIGN
};
typedef enum transaction_parsing_state_e transaction_parsing_state_t;

typedef struct transaction_context {

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

    unsigned char inputOutputValue[8];

    /** Computed sum of transaction inputs */
    unsigned char totalInputAmount[8];

    /** Computed sum of transaction outputs */
    unsigned char totalOutputAmount[8];

    /** Bytes parsed */
    uint64_t bytesRead;

    /** Bytes to be parsed (in the chunk) */
    uint64_t bytesChunkRemaining;

    /** Bytes to be parsed current chunk */
    unsigned char buffer[300];

    /** Bytes to be parsed (in the next chunk) */
    unsigned char saveBufferForNextChunk[255];
    unsigned char saveBufferLength;

    /** Total Tx Bytes */
    uint64_t totalTxBytes;

    /** Fields to Display  */
    unsigned char remark[30];
    uint64_t remarkSize;
    unsigned char fees[8];
    unsigned char changeAmount[8];
    unsigned char outputAddress[21];
    unsigned char changeAddress[21];

} transaction_context_t;

extern transaction_context_t txContext;


typedef void (*ui_processor_fn)(uint8_t curStep);
typedef uint8_t (*step_processor_fn)(uint8_t curStep);
extern step_processor_fn step_processor;
extern ui_processor_fn ui_processor;

void handleSignTxPacket(commPacket_t *packet, commContext_t *context);
void finalizeSignTx(volatile unsigned int *flags);

#endif //PROJECT_SIGNTX_H
