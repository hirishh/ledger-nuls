//
// Created by andrea on 09/12/18.
//

#include "common_parser.h"
#include "2_transfer.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_send_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Send from", 0x01),
  TITLE_ITEM("To", 0x02),
  TITLE_ITEM("Remark", 0x03),
  TITLE_ITEM("Amount", 0x04),
  TITLE_ITEM("Fees", 0x05),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_ARROW_RIGHT(0x04),
  ICON_CHECK(0x05),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_send(uint8_t step) {
  if (step == 2 && txContext.remarkSize == 0) {
    return 4;
  }
  return step + 1;
}

static void uiProcessor_send(uint8_t step) {
  uint8_t addressToShow[32];
  unsigned short amountTextSize;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      //deriveAccountAddress(&reqContext.accountFrom);
      os_memmove(lineBuffer, &reqContext.accountFrom.address, 32);
      lineBuffer[32] = '\0';
      break;
    case 2:
      nuls_address_to_encoded_base58(txContext.outputAddress, addressToShow);
      os_memmove(lineBuffer, addressToShow, 32);
      lineBuffer[32] = '\0';
      break;
    case 3:
      os_memmove(lineBuffer, &txContext.remark, txContext.remarkSize);
      break;
    case 4:
      amountTextSize = nuls_hex_amount_to_displayable(txContext.outputAmount, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 5:
      amountTextSize = nuls_hex_amount_to_displayable(txContext.fees, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    default:
      THROW(INVALID_STATE);
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

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      //PRINTF("-- BEGINNING\n");
      // Only PLACEHOLDER for this TX type
      txContext.tx_parsing_state = PLACEHOLDER;

    case PLACEHOLDER:
      //PRINTF("-- PLACEHOLDER\n");
      is_available_to_parse(4);
      uint32_t placeholder = nuls_read_u32(txContext.bufferPointer, 1, 0);
      //PRINTF("placeholder %.*H\n", 4, &placeholder);
      if(placeholder != 0xFFFFFFFF)
        THROW(INVALID_PARAMETER);
      transaction_offset_increase(4);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }

}

void tx_finalize_2_transfer() {

  if (transaction_amount_sub_be(txContext.fees, txContext.totalInputAmount, txContext.totalOutputAmount)) {
    // L_DEBUG_APP(("Fee amount not consistent\n"));
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_send_nano;
  ux.elements_count = 13;
  totalSteps = 5;
  step_processor = stepProcessor_send;
  ui_processor = uiProcessor_send;
}
