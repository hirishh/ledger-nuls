#include "common_parser.h"
#include "3_alias.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

static const bagl_element_t ui_3_alias_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Alias for address", 0x01),
  TITLE_ITEM("Alias", 0x02),
  TITLE_ITEM("Remark", 0x03),
  TITLE_ITEM("Fees", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_3_alias(uint8_t step) {
  uint8_t nextStep = step + 1;
  if(step == 2 && txContext.remarkSize == 0) {
      nextStep++; // no remark
  }
  return nextStep;
}

static tx_type_specific_3_alias_t *cc = &(txContext.tx_fields.alias);

static void uiProcessor_3_alias(uint8_t step) {
  unsigned short amountTextSize;
  os_memset(lineBuffer, 0, sizeof(lineBuffer));
  switch (step) {
    case 1:
      //Alias for address
      os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, BASE58_ADDRESS_LENGTH);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 2:
      // Alias
      os_memmove(lineBuffer, &cc->alias, cc->aliasSize);
      lineBuffer[cc->aliasSize] = '\0';
      break;
    case 3:
      //Remark
      os_memmove(lineBuffer, &txContext.remark, txContext.remarkSize);
      lineBuffer[txContext.remarkSize] = '\0';
      break;
    case 4:
      //Fees
      amountTextSize = nuls_hex_amount_to_displayable(txContext.fees, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    default:
      THROW(INVALID_STATE);
  }
}

void tx_parse_specific_3_alias() {

  /* TX Structure:
   *
   * TX_SPECIFIC
   * - len:address
   * - len:alias
   *
   * */

  uint64_t tmpVarInt = 0;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:

    case _3_ALIAS_ADDRESS_LENGTH:
      txContext.tx_parsing_state = _3_ALIAS_ADDRESS_LENGTH;
      tmpVarInt = transaction_get_varint();
      if(tmpVarInt > ADDRESS_LENGTH) {
        THROW(INVALID_PARAMETER);
      }

    case _3_ALIAS_ADDRESS:
      txContext.tx_parsing_state = _3_ALIAS_ADDRESS;
      is_available_to_parse(tmpVarInt);
      //Save the address
      os_memmove(cc->address, txContext.bufferPointer, tmpVarInt);
      transaction_offset_increase(tmpVarInt);


    case _3_ALIAS_ALIAS_LENGTH:
      txContext.tx_parsing_state = _3_ALIAS_ALIAS_LENGTH;
      tmpVarInt = transaction_get_varint();
      if(tmpVarInt > MAX_ALIAS_LENGTH || tmpVarInt == 0) {
        THROW(INVALID_PARAMETER);
      }
      cc->aliasSize = (unsigned char) tmpVarInt;

    case _3_ALIAS_ALIAS:
      txContext.tx_parsing_state = _3_ALIAS_ALIAS;
      is_available_to_parse(cc->aliasSize);
      //Save the alias
      os_memmove(cc->alias, txContext.bufferPointer, cc->aliasSize);
      cc->alias[cc->aliasSize] = '\0';
      transaction_offset_increase(cc->aliasSize);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_3_alias() {
  //Throw if:
  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    THROW(INVALID_PARAMETER);
  }

  // - addressFrom is different from alias.address
  if(nuls_secure_memcmp(reqContext.accountFrom.address, cc->address, ADDRESS_LENGTH) != 0) {
    THROW(INVALID_PARAMETER);
  }

  // - should be only 1 output (excluding the change one) and it's a blackhole output with specific amount of 1 Nuls
  if(txContext.nOut != 1) {
    THROW(INVALID_PARAMETER);
  }
  if(
      (txContext.nOut == 1 && nuls_secure_memcmp(txContext.outputAddress[0], BLACK_HOLE_ADDRESS, ADDRESS_LENGTH) != 0) ||
      (txContext.nOut == 1 && nuls_secure_memcmp(txContext.outputAmount[0], BLACK_HOLE_ALIAS_AMOUNT, AMOUNT_LENGTH) != 0)
    ) {
    THROW(INVALID_PARAMETER);
  }

  //Add blackhole output amount to fees
  if (transaction_amount_add_be(txContext.fees, txContext.fees, txContext.outputAmount[0])) {
    THROW(EXCEPTION_OVERFLOW);
  }

  ux.elements = ui_3_alias_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_3_alias;
  ui_processor = uiProcessor_3_alias;
}
