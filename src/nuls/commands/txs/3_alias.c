#include "common_parser.h"
#include "3_alias.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_3_alias_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Alias for address", 0x01),
  TITLE_ITEM("Alias", 0x02),
  TITLE_ITEM("Fees", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_3_alias(uint8_t step) {
  return step + 1;
}

static void uiProcessor_3_alias(uint8_t step) {
  unsigned short amountTextSize;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      //Alias for address
      os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, BASE58_ADDRESS_LENGTH);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 2:
      // Alias
      os_memmove(lineBuffer, &txContext.tx_specific_fields.alias.alias, txContext.tx_specific_fields.alias.aliasSize);
      lineBuffer[txContext.tx_specific_fields.alias.aliasSize] = '\0';
      break;
    case 3:
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
   * COMMON
   * - type -> 2 Bytes
   * - time -> 6 Bytes
   * - remarkLength -> 1 Byte
   * - remark -> remarkLength Bytes (max 30 bytes)
   *
   * TX_SPECIFIC (handled here)
   * - len:address
   * - len:alias
   *
   * COIN_INPUT (multiple)
   * - owner (hash + index)
   * - amount
   * - locktime
   * COIN_OUTPUT (change + blackhole)
   * - owner (address only)
   * - amount
   * - locktime
   * */

  uint64_t tmpVarInt = 0;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      PRINTF("-- BEGINNING\n");

    case _3_ALIAS_ADDRESS_LENGTH:
      txContext.tx_parsing_state = _3_ALIAS_ADDRESS_LENGTH;
      PRINTF("-- 3_ALIAS_ADDRESS_LENGTH\n");
      tmpVarInt = transaction_get_varint();
      if(tmpVarInt != ADDRESS_LENGTH) {
        THROW(INVALID_PARAMETER);
      }

    case _3_ALIAS_ADDRESS:
      txContext.tx_parsing_state = _3_ALIAS_ADDRESS;
      PRINTF("-- 3_ALIAS_ADDRESS\n");
      is_available_to_parse(ADDRESS_LENGTH);
      PRINTF("address: %.*H\n", ADDRESS_LENGTH, txContext.bufferPointer);
      //Save the address
      os_memmove(txContext.tx_specific_fields.alias.address, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);


    case _3_ALIAS_ALIAS_LENGTH:
      txContext.tx_parsing_state = _3_ALIAS_ALIAS_LENGTH;
      PRINTF("-- 3_ALIAS_ALIAS_LENGTH\n");
      tmpVarInt = transaction_get_varint();
      if(tmpVarInt > MAX_ALIAS_LENGTH || tmpVarInt == 0) {
        THROW(INVALID_PARAMETER);
      }
      txContext.tx_specific_fields.alias.aliasSize = (unsigned char) tmpVarInt;

    case _3_ALIAS_ALIAS:
      txContext.tx_parsing_state = _3_ALIAS_ALIAS;
      PRINTF("-- 3_ALIAS_ALIAS\n");
      is_available_to_parse(txContext.tx_specific_fields.alias.aliasSize);
      PRINTF("alias: %.*H\n", txContext.tx_specific_fields.alias.aliasSize, txContext.bufferPointer);
      //Save the alias
      os_memmove(txContext.tx_specific_fields.alias.alias, txContext.bufferPointer, txContext.tx_specific_fields.alias.aliasSize);
      txContext.tx_specific_fields.alias.alias[txContext.tx_specific_fields.alias.aliasSize] = '\0';
      PRINTF("aliasStr: %s\n", txContext.tx_specific_fields.alias.alias);
      transaction_offset_increase(txContext.tx_specific_fields.alias.aliasSize);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_3_alias() {
  PRINTF("tx_finalize_3_alias\n");

  //Throw if:

  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0 || (reqContext.accountChange.pathLength > 0 && !txContext.changeFound)) {
    // PRINTF(("Change not provided!\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_3_alias - A\n");

  // - addresFrom is different from alias.address
  if(nuls_secure_memcmp(reqContext.accountFrom.address, txContext.tx_specific_fields.alias.address, ADDRESS_LENGTH) != 0) {
    // PRINTF(("Alias address is different from account provided in input!\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_3_alias - B\n");

  /* Not Really necessary
  // - changeFound is parsed correctly but it's not equal to alias.address
  if(reqContext.accountChange.pathLength > 0 && txContext.changeFound &&
     nuls_secure_memcmp(reqContext.accountChange.address, txContext.tx_specific_fields.alias.address, ADDRESS_LENGTH) != 0) {
    // PRINTF(("Change Address provided but it's different from tx specific alias address\n"));
    THROW(INVALID_PARAMETER);
  }
  */

  PRINTF("tx_finalize_3_alias - nOut %d\n", txContext.nOut);
  PRINTF("tx_finalize_3_alias - txContext.outputAddress %.*H\n", ADDRESS_LENGTH, txContext.outputAddress[0]);
  PRINTF("tx_finalize_3_alias - BLACK_HOLE_ADDRESS %.*H\n", ADDRESS_LENGTH, BLACK_HOLE_ADDRESS);
  PRINTF("tx_finalize_3_alias - txContext.outputAmount %.*H\n", AMOUNT_LENGTH, txContext.outputAmount[0]);
  PRINTF("tx_finalize_3_alias - BLACK_HOLE_ALIAS_AMOUNT %.*H\n", AMOUNT_LENGTH, BLACK_HOLE_ALIAS_AMOUNT);
  PRINTF("tx_finalize_3_alias - nuls_secure_memcmp address %d\n", nuls_secure_memcmp(txContext.outputAddress[0], BLACK_HOLE_ADDRESS, ADDRESS_LENGTH));
  PRINTF("tx_finalize_3_alias - nuls_secure_memcmp amount %d\n", nuls_secure_memcmp(txContext.outputAmount[0], BLACK_HOLE_ALIAS_AMOUNT, AMOUNT_LENGTH));

  // - should be only 1 output (excluding the change one) and it's a blackhole output with specific amount of 1 Nuls
  if(txContext.nOut != 1) {
    // PRINTF(("Number of output is wrong. Only 1 normal output and must be a black hole)\n"));
    THROW(INVALID_PARAMETER);
  }
  if(
      (txContext.nOut == 1 && nuls_secure_memcmp(txContext.outputAddress[0], BLACK_HOLE_ADDRESS, ADDRESS_LENGTH) != 0) ||
      (txContext.nOut == 1 && nuls_secure_memcmp(txContext.outputAmount[0], BLACK_HOLE_ALIAS_AMOUNT, AMOUNT_LENGTH) != 0)
    ) {
    // PRINTF(("Blackhole output is not corret (wrong address or amount)\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_3_alias - C\n");

  //Calculate fees (input - output)
  if (transaction_amount_sub_be(txContext.fees, txContext.totalInputAmount, txContext.totalOutputAmount)) {
    // L_DEBUG_APP(("Fee amount not consistent\n"));
    THROW(INVALID_PARAMETER);
  }
  //Add blackhole output amount to fees
  if (transaction_amount_add_be(txContext.fees, txContext.fees, txContext.outputAmount[0])) {
    // L_DEBUG_APP(("Fee amount not consistent - blackhole\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_3_alias - D\n");

  PRINTF("finalize. Fees: %.*H\n", AMOUNT_LENGTH, txContext.fees);

  ux.elements = ui_3_alias_nano;
  ux.elements_count = 9;
  totalSteps = 3;
  step_processor = stepProcessor_3_alias;
  ui_processor = uiProcessor_3_alias;
}
