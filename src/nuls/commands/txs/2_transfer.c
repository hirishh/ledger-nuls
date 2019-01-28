//
// Created by andrea on 09/12/18.
//

#include "2_transfer.h"
#include "../../dposutils.h"
#include "../../../io.h"
#include "../../../ui_utils.h"
#include "../../ui_elements_s.h"
#include "../signTx.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_send_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Send from", 0x01),
  TITLE_ITEM("To", 0x02),
  TITLE_ITEM("Message", 0x03),
  TITLE_ITEM("Amount", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_send(uint8_t step) {
  if (step == 2 && curLength == 0) {
    return 4;
  }
  return step + 1;
}

static void uiProcessor_send(uint8_t step) {
  uint64_t address;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      os_memset(lineBuffer, 0, 50);
      os_memmove(lineBuffer, &reqContext.address, 32);
      lineBuffer[32] = '\0';
      break;
    case 2:
      deriveAddressStringRepresentation(transaction.recipientId, lineBuffer);
      break;
    case 3:
      os_memmove(lineBuffer, message, MIN(50, curLength));
      // ellipsis
      if (curLength > 46) {
        os_memmove(lineBuffer + 46, "...\0", 4);
      }
      break;
    case 4:
      satoshiToString(transaction.amountSatoshi, lineBuffer);
  }
}

void tx_chunk_transfer() {

  /* TX Structure:
   *
   * - type -> 2 Bytes
   * - time -> 6 Bytes
   * - remarkLength -> 1 Byte
   * - remark -> remarkLength Bytes (max 30 bytes)
   * Then, for this TX:
   * - placeholder -> 4 bytes (0xFFFFFFFF)
   * Then, coin info
   * -
   * */

  //TODO Handle saveBufferForNextChunk

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      // Reset transaction state
      txContext.transactionRemainingInputsOutputs = 0;
      txContext.transactionCurrentInputOutput = 0;

      os_memset(
              txContext.totalInputAmount,
              0, sizeof(txContext.totalInputAmount));
      os_memset(
              txContext.totalInputAmount,
              0, sizeof(txContext.totalOutputAmount));
      //Done, start with the next field
      txContext.tx_parsing_state = FIELD_TYPE;

    case FIELD_TYPE:
      //already parsed..
      is_available_to_parse(2);
      transaction_offset_increase(2);
      txContext.tx_parsing_state =  FIELD_TIME;

    case FIELD_TIME:
      is_available_to_parse(6);
      transaction_offset_increase(6);
      txContext.tx_parsing_state = FIELD_REMARK_LENGTH;

    case FIELD_REMARK_LENGTH:
      //Size
      txContext.remarkSize = transaction_get_varint();
      txContext.tx_parsing_state = FIELD_REMARK;

    case FIELD_REMARK:
      if(txContext.remarkSize != 0) {
        is_available_to_parse(txContext.remarkSize)
        os_memmove(txContext.remark, txContext.buffer, txContext.remarkSize)
        transaction_offset_increase(txContext.remarkSize);
      }
      txContext.tx_parsing_state = PLACEHOLDER;

    case PLACEHOLDER:
      is_available_to_parse(4)
      transaction_offset_increase(4);
      txContext.tx_parsing_state = FIELD_INPUT_SIZE;

    case FIELD_COIN_INPUT_LENGTH:
      txContext.transactionRemainingInputsOutputs = transaction_get_varint();
      txContext.tx_parsing_state = FIELD_INPUT;

      //TODO
    default:
      break;
  }

}

void tx_end_transfer() {

  //Check tx_parsing_state

  ux.elements = ui_send_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_send;
  ui_processor = uiProcessor_send;
}
