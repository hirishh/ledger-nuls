#include "common_parser.h"
#include "2_transfer.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

static const bagl_element_t ui_2_transfer_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Send from", 0x01),
  TITLE_ITEM("Output", 0x02),
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

static uint8_t stepProcessor_2_transfer(uint8_t step) {
  uint8_t nextStep = step + 1;
  if(step == 2) {
    if(txContext.nOutCursor < txContext.nOut)
      return 2; //Loop on outputs
    else if(txContext.remarkSize == 0)
      nextStep++; // no remark
  }
  return nextStep;
}

static void uiProcessor_2_transfer(uint8_t step) {
  uint8_t addressToShow[32] = {0};
  unsigned short amountTextSize;
  os_memset(lineBuffer, 0, sizeof(lineBuffer));
  switch (step) {
    case 1:
      //Send From
      os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, 32);
      lineBuffer[32] = '\0';
      break;
    case 2:
      // Output
      lineBuffer[0] = '#';
      amountTextSize = nuls_int_to_string(txContext.nOutCursor, lineBuffer + 1);
      lineBuffer[1 + amountTextSize] = ':';
      lineBuffer[1 + amountTextSize + 1] = ' ';
      nuls_address_to_encoded_base58(txContext.outputAddress[txContext.nOutCursor], addressToShow);
      os_memmove(lineBuffer + 1 + amountTextSize + 2, addressToShow, BASE58_ADDRESS_LENGTH);
      lineBuffer[BASE58_ADDRESS_LENGTH + 3 + amountTextSize] = '\0';
      txContext.nOutCursor++;
      break;
    case 3:
      //Remark
      os_memmove(lineBuffer, &txContext.remark, txContext.remarkSize);
      lineBuffer[txContext.remarkSize] = '\0';
      break;
    case 4:
      //Amount Spent (without change)
      amountTextSize = nuls_hex_amount_to_displayable(txContext.amountSpent, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 5:
      //Fees
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
   * TX_SPECIFIC
   * - reuse PLACEHOLDER for txData, only /w 1 byte for len = 0
   * */

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:

    case PLACEHOLDER: //txData null, only has txData len is 0
      txContext.tx_parsing_state = PLACEHOLDER;
      uint32_t txDataLenInt = transaction_get_varint();
      if(txDataLenInt != 0) { 
        THROW(INVALID_PARAMETER);
      }

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_2_transfer() {
  os_memmove(txContext.amountSpent, txContext.totalOutputAmount, AMOUNT_LENGTH);
  if(txContext.changeFound) {
    if (transaction_amount_sub_be(txContext.amountSpent, txContext.amountSpent, txContext.changeAmount)) {
      THROW(EXCEPTION_OVERFLOW);
    }
  }

  ux.elements = ui_2_transfer_nano;
  ux.elements_count = 13;
  totalSteps = 5;
  step_processor = stepProcessor_2_transfer;
  ui_processor = uiProcessor_2_transfer;
}
