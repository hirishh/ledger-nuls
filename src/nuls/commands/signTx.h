#include "../../io.h"
#ifndef PROJECT_SIGNTX_H
#define PROJECT_SIGNTX_H

// typedef void (*transaction_chunk_handler)(commPacket_t packet);

#define NEED_NEXT_CHUNK 0x6866

/**
 * Current state of an untrusted transaction hashing
 */

enum transaction_parsing_state_e {
    /** No transaction in progress */
    BEGINNING = 0x00,
    /** Commmon Fields */
    FIELD_TYPE
    FIELD_TIME
    FIELD_REMARK_LENGTH
    FIELD_REMARK
    /** Data Fields */
    PLACEHOLDER
    //TODO for other TXs
    /** COIN: Input & OUTPUT */
    FIELD_INPUT_SIZE
    //TODO
    /** Ready to be signed */
    READY_TO_SIGN
};
typedef enum transaction_parsing_state_e transaction_parsing_state_t;

typedef struct transaction_context {

    /** Type of the transaction */
    uint16_t type;

    /** State of the transaction parsing, type transaction_parsing_state_t */
    uint8_t tx_parsing_state;

    /** Remaining number of inputs/outputs to process for this transaction */
    unsigned long int transactionRemainingInputsOutputs;

    /** Index of the currently processed input/output for this transaction */
    unsigned long int transactionCurrentInputOutput;

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
    unsigned char fees[8];           // only in wallet mode
    unsigned char changeAmount[8];   // only in wallet mode
    unsigned char outputAddress[21]; // only in wallet mode
    unsigned char changeAddress[21]; // only in wallet mode

} transaction_context_t;

extern transaction_context_t txContext;


typedef void (*ui_processor_fn)(uint8_t curStep);
typedef uint8_t (*step_processor_fn)(uint8_t curStep);
extern step_processor_fn step_processor;
extern ui_processor_fn ui_processor;

void handleSignTxPacket(commPacket_t *packet, commContext_t *context);
void finalizeSignTx(volatile unsigned int *flags);

// Parser Utils

void transaction_offset_increase(unsigned char value);
void is_available_to_parse(unsigned char x);
unsigned long int transaction_get_varint(void);

#endif //PROJECT_SIGNTX_H
