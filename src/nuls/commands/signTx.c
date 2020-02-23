#include "signTx.h"
#include "../nuls_internals.h"
#include "os.h"

#include "txs/common_parser.h"
#include "txs/2_transfer.h"
#include "txs/3_alias.h"
#include "txs/4_register_agent.h"
#include "txs/5_join_consensus.h"
#include "txs/6_leave_consensus.h"
#include "txs/9_unregister_agent.h"
#include "txs/10_data.h"
#include "txs/100_create_contract.h"
#include "txs/101_call_contract.h"
#include "txs/102_delete_contract.h"

typedef void (*tx_parse_fn)();
typedef void (*tx_end_fn)();

tx_parse_fn tx_parse;
tx_end_fn tx_end;
ui_processor_fn ui_processor;
step_processor_fn step_processor;

static unsigned int ui_sign_tx_button(unsigned int button_mask, unsigned int button_mask_counter) {
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
  return 0;
}


void handleSignTxPacket(commPacket_t *packet, commContext_t *context) {

  uint32_t headerBytesRead = 0;

  // if first packet with signing header
  if ( packet->first ) {
    // Reset sha256 and context
    os_memset(&reqContext, 0, sizeof(reqContext));
    os_memset(&txContext, 0, sizeof(txContext));
    cx_sha256_init(&txContext.txHash);
    txContext.tx_parsing_state = BEGINNING;
    txContext.tx_parsing_group = COMMON;
    txContext.bufferPointer = NULL;

    // IMPORTANT this logic below only works if the first packet contains the needed information (Which it should)
    // Set signing context by parsing bip32paths and other info. function returns number of bytes read (not part of TX)
    headerBytesRead = setReqContextForSign(packet);

    txContext.type = nuls_read_u16(packet->data + headerBytesRead, 0, 0);
    txContext.totalTxBytes = reqContext.signableContentLength;

    switch (txContext.type) {
      case TX_TYPE_2_TRANSFER_TX:
        tx_parse = tx_parse_specific_2_transfer;
        tx_end = tx_finalize_2_transfer;
        break;
      case TX_TYPE_3_SET_ALIAS:
        tx_parse = tx_parse_specific_3_alias;
        tx_end = tx_finalize_3_alias;
        break;
#if 0
      case TX_TYPE_4_REGISTER_CONSENSUS_NODE:
        tx_parse = tx_parse_specific_4_register_agent;
        tx_end = tx_finalize_4_register_agent;
        break;
      case TX_TYPE_5_JOIN_CONSENSUS:
        tx_parse = tx_parse_specific_5_join_consensus;
        tx_end = tx_finalize_5_join_consensus;
        break;
      case TX_TYPE_6_CANCEL_CONSENSUS:
        tx_parse = tx_parse_specific_6_leave_consensus;
        tx_end = tx_finalize_6_leave_consensus;
        break;
      case TX_TYPE_9_UNREGISTER_CONSENSUS_NODE:
        tx_parse = tx_parse_specific_9_unregister_agent;
        tx_end = tx_finalize_9_unregister_agent;
        break;
      case TX_TYPE_10_BUSINESS_DATA:
        tx_parse = tx_parse_specific_10_data;
        tx_end = tx_finalize_10_data;
        break;
      case TX_TYPE_100_CREATE_CONTRACT:
        tx_parse = tx_parse_specific_100_create_contract;
        tx_end = tx_finalize_100_create_contract;
        break;
      case TX_TYPE_101_CALL_CONTRACT:
        tx_parse = tx_parse_specific_101_call_contract;
        tx_end = tx_finalize_101_call_contract;
        break;
      case TX_TYPE_102_DELETE_CONTRACT:
        tx_parse = tx_parse_specific_102_delete_contract;
        tx_end = tx_finalize_102_delete_contract;
        break;
#endif
      default:
        THROW(NOT_SUPPORTED);
    }

  }

  //insert at beginning saveBufferForNextChunk if present
  if(txContext.saveBufferLength > 0) {
    //Shift TX payload (without header) of saveBufferLength bytes on the right
    os_memmove(
            packet->data + headerBytesRead + txContext.saveBufferLength,
            packet->data + headerBytesRead,
            packet->length - headerBytesRead
            );
    //Copy saved buffer in the correct position (beginning of new tx data)
    os_memmove(
            packet->data + headerBytesRead,
            txContext.saveBufferForNextChunk,
            txContext.saveBufferLength
            );
    packet->length += txContext.saveBufferLength;
    txContext.saveBufferLength = 0;
    os_memset(txContext.saveBufferForNextChunk, 0, sizeof(txContext.saveBufferForNextChunk));
  }

  txContext.bufferPointer = packet->data + headerBytesRead;
  txContext.bytesChunkRemaining = packet->length - headerBytesRead;

  BEGIN_TRY {
      TRY {
          switch(txContext.tx_parsing_group) {
            case COMMON:
              parse_group_common();
            case TX_SPECIFIC:
              if(txContext.tx_parsing_group != TX_SPECIFIC) {
                THROW(INVALID_STATE);
              }
              tx_parse();
            case COIN_INPUT:
              parse_group_coin_input();
            case COIN_OUTPUT:
              parse_group_coin_output();
            case CHECK_SANITY_BEFORE_SIGN:
              check_sanity_before_sign();
              break;
            default:
              THROW(INVALID_STATE);
          }
        }
      CATCH_OTHER(e) {
          if(e == NEED_NEXT_CHUNK) {
            os_memmove(txContext.saveBufferForNextChunk, txContext.bufferPointer, txContext.bytesChunkRemaining);
            txContext.saveBufferLength = txContext.bytesChunkRemaining;
          } else {
            //Unexpected Error during parsing. Let the client know
            THROW(e);
          }
      }
      FINALLY {}
    }
  END_TRY;
}

static uint8_t default_step_processor(uint8_t cur) {
  return cur + 1;
}


void finalizeSignTx(volatile unsigned int *flags) {
  if(txContext.tx_parsing_group != TX_PARSED || txContext.tx_parsing_state != READY_TO_SIGN)
    THROW(INVALID_STATE);

  // Close sha256 and hash again
  cx_hash_finalize(txContext.digest, DIGEST_LENGTH);

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
