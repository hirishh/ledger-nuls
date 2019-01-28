//
// Created by andrea on 09/12/18.
//

#include "signTx.h"
#include "../../io.h"
#include "../nuls_utils.h"
#include "txs/2_transfer.h"
// #include "./txs/voteTx.h"
// #include "./txs/createMultiSig.h"
// #include "./txs/createSignatureTx.h"
// #include "./txs/registerDelegateTx.h"
#include "../approval.h"
#include "../nuls_helpers.h"
#include "../../ui_utils.h"

#define TX_TYPE_CONSENSUS_REWARD 1
#define TX_TYPE_TRANSFER_TX 2
#define TX_TYPE_SET_ALIAS 3
#define TX_TYPE_REGISTER_CONSENSUS_NODE 4
#define TX_TYPE_JOIN_CONSENSUS 5
#define TX_TYPE_CANCEL_CONSENSUS 6
#define TX_TYPE_YELLOW_CARD 7
#define TX_TYPE_RED_CARD 8
#define TX_TYPE_UNREGISTER_CONSENSUS_NODE 9
#define TX_TYPE_BUSINESS_DATA 10
#define TX_TYPE_CREATE_CONTRACT 100
#define TX_TYPE_CALL_CONTRACT 101
#define TX_TYPE_DELETE_CONTRACT 102
#define TX_TYPE_TRANSFER_CONTRACT 103

typedef void (*tx_chunk_fn)();
typedef void (*tx_end_fn)();

tx_init_fn tx_init;
tx_chunk_fn tx_chunk;
tx_end_fn tx_end;
ui_processor_fn ui_processor;
step_processor_fn step_processor;

static cx_sha256_t txHash;
transaction_context_t txContext;

static void ui_sign_tx_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      if (currentStep < totalSteps) {
        currentStep = step_processor(currentStep);
        ui_processor(currentStep);
        UX_REDISPLAY();
      } else {
        touch_approve();
      }
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny(NULL);
      break;
  }
}

void handleSignTxPacket(commPacket_t *packet, commContext_t *context) {
  // if first packet with signing header
  if ( packet->first ) {
    // Reset sha256 and tx
    cx_sha256_init(&txHash);

    // IMPORTANT this logic below only works if the first packet contains the needed information (Which it should)
    // Set signing context from first packet and patches the .data and .length by removing header length
    uint32_t headersLength = setReqContextForSign(packet);

    // fetch transaction type and init txContext for signing
    txContext.type = nuls_read_u16(packet->data, 1, 0);
    txContext.totalTxBytes = reqContext.signableContentLength;
    txContext.tx_parsing_state = BEGINNING;
    txContext.bytesRead = 0;
    txContext.saveBufferLength = 0;

    switch (txContext.type) {
      case TX_TYPE_TRANSFER_TX:
        tx_chunk = tx_chunk_transfer;
        tx_end = tx_end_transfer;
        break;
      default:
        THROW(NOT_SUPPORTED);
    }

  }

  os_memmove(txContext.buffer, packet->data, packet->length);
  txContext.bytesChunkRemaining = packet->length;

  BEGIN_TRY {
      TRY {
          tx_chunk();
        }
      CATCH_OTHER(e) {
          if(e == NEED_NEXT_CHUNK) {
            //TODO Save last bytes and use them in the next chunk


          } else {
            //Unexpected Error during parsing. Let the client know
            THROW(e);
          }
        }
      FINALLY {
      }
    }
  END_TRY;

  //No throw, do incremental hash of the buffer
  cx_hash(&txHash, NULL, packet->data, packet->length, NULL, NULL);

}
static uint8_t default_step_processor(uint8_t cur) {
  return cur + 1;
}


void finalizeSignTx(volatile unsigned int *flags) {
  uint8_t finalHash[32];

  // Close first sha256
  cx_hash(&txHash, CX_LAST, finalHash, 0, NULL, NULL);

  os_memmove(&signContext.digest, txHash.acc, 32);

  // Init user flow.
  step_processor = default_step_processor;
  ui_processor = NULL;

  tx_end();

  currentStep = 1;
  *flags |= IO_ASYNCH_REPLY;

  ux.button_push_handler = ui_sign_tx_button;
  ux.elements_preprocessor = uiprocessor;

  ui_processor(1);
  UX_WAKE_UP();
  UX_REDISPLAY();
}


// Parser Utils

void transaction_offset_increase(unsigned char value) {
  txContext.buffer += value;
  txContext.bytesChunkRemaining -= value;
}

void is_available_to_parse(unsigned char x) {
  if(txContext.bytesChunkRemaining >= x)
    return true;
  else
    THROW(NEED_NEXT_CHUNK);
}

unsigned long int transaction_get_varint(void) {
  unsigned char firstByte;
  is_available_to_parse(1);
  firstByte = *txContext.buffer;
  if (firstByte < 0xFD) {
    transaction_offset_increase(1);
    return firstByte;
  } else if (firstByte == 0xFD) {
    unsigned long int result;
    transaction_offset_increase(1);
    is_available_to_parse(2);
    result = (unsigned long int)(*txContext.buffer) |
             ((unsigned long int)(*(txContext.buffer + 1)) << 8);
    transaction_offset_increase(2);
    return result;
  } else if (firstByte == 0xFE) {
    unsigned long int result;
    transaction_offset_increase(1);
    is_available_to_parse(4);
    result = nuls_read_u32(txContext.buffer, 0, 0);
    transaction_offset_increase(4);
    return result;
  } else {
    L_DEBUG_APP(("Varint parsing failed\n"));
    THROW(INVALID_PARAMETER);
  }
}