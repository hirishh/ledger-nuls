//
// Created by andrea on 09/12/18.
//

#include "2_transfer.h"
#include "../../../io.h"
#include "../../../ui_utils.h"
#include "../../ui_elements_s.h"
#include "../../nuls_utils.h"
#include "../../nuls_helpers.h"
#include "nuls_tx_parser.h"
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

void tx_parse_specific_2_transfer() {

  /* TX Structure:
   *
   * COMMON
   * - type -> 2 Bytes
   * - time -> 6 Bytes
   * - remarkLength -> 1 Byte
   * - remark -> remarkLength Bytes (max 30 bytes)
   *
   * TX_SPECIFIC (handled here)
   * - placeholder -> 4 bytes (0xFFFFFFFF)
   *
   * COIN_INPUT
   * - owner (hash + index)
   * - amount
   * - locktime
   * COIN_OUTPUT
   * - owner (address or script)
   * - amount
   * - locktime
   * */

  //TODO Handle saveBufferForNextChunk


  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      // Only PLACEHOLDER for this TX type
      txContext.tx_parsing_state = PLACEHOLDER;

    case PLACEHOLDER:
      is_available_to_parse(4)
      uint32_t placeholder = nuls_read_u32(txContext.buffer, 1, 0);
      if(placeholder != 0xFFFFFFFF)
        THROW(INVALID_PARAMETER);
      transaction_offset_increase(4);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT
      txContext.tx_parsing_state = COIN_INPUT_SIZE;
      break;

    default:
      THROW(INVALID_STATE)
  }

}

void tx_finalize_2_transfer() {

  if(txContext.tx_parsing_state != READY_TO_SIGN)
    THROW(INVALID_STATE);

  ux.elements = ui_send_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_send;
  ui_processor = uiProcessor_send;
}
