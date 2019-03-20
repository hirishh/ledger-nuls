#include "common_parser.h"
#include "6_leave_consensus.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_6_leave_consensus_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Leave Consensus for", 0x01),
  TITLE_ITEM("TX Hash", 0x02),
  TITLE_ITEM("Fees", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_6_leave_consensus(uint8_t step) {
  return step + 1;
}

static void uiProcessor_6_leave_consensus(uint8_t step) {
  unsigned short amountTextSize;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      //Join Consensus for address
      os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, BASE58_ADDRESS_LENGTH);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 2:
      //TX Hash
      //snprintf(lineBuffer, 50, "%.*X", txContext.tx_fields.join_consensus.agentHash);
      //os_memmove(lineBuffer + 46, "...\0", 4);
      snprintf(lineBuffer, 50, "%.*H...%.*H",
              8, txContext.tx_fields.leave_consensus.txHash,
              8, txContext.tx_fields.leave_consensus.txHash + HASH_LENGTH - 8);
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

void tx_parse_specific_6_leave_consensus() {

  /* TX Structure:
   *
   * COMMON
   * - type -> 2 Bytes
   * - time -> 6 Bytes
   * - remarkLength -> 1 Byte
   * - remark -> remarkLength Bytes (max 30 bytes)
   *
   * TX_SPECIFIC (handled here)
   * - txHash
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

    case _6_LEAVE_CONS_TXHASH:
      txContext.tx_parsing_state = _6_LEAVE_CONS_TXHASH;
      PRINTF("-- _6_LEAVE_CONS_TXHASH\n");
      is_available_to_parse(HASH_LENGTH);
      os_memmove(txContext.tx_fields.leave_consensus.txHash, txContext.bufferPointer, HASH_LENGTH);
      transaction_offset_increase(HASH_LENGTH);
      PRINTF("txHash: %.*H\n", HASH_LENGTH, txContext.tx_fields.leave_consensus.txHash);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_6_leave_consensus() {
  PRINTF("tx_finalize_6_leave_consensus\n");

  //Throw if:

  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0 || (reqContext.accountChange.pathLength > 0 && !txContext.changeFound)) {
    // PRINTF(("Change not provided!\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_6_leave_consensus - A\n");

  /*
  PRINTF("tx_finalize_6_leave_consensus - nOut %d\n", txContext.nOut);
  PRINTF("tx_finalize_6_leave_consensus - txContext.outputAddress %.*H\n", ADDRESS_LENGTH, txContext.outputAddress[0]);
  PRINTF("tx_finalize_6_leave_consensus - BLACK_HOLE_ADDRESS %.*H\n", ADDRESS_LENGTH, BLACK_HOLE_ADDRESS);
  PRINTF("tx_finalize_6_leave_consensus - txContext.outputAmount %.*H\n", AMOUNT_LENGTH, txContext.outputAmount[0]);
  PRINTF("tx_finalize_6_leave_consensus - BLACK_HOLE_ALIAS_AMOUNT %.*H\n", AMOUNT_LENGTH, BLACK_HOLE_ALIAS_AMOUNT);
  PRINTF("tx_finalize_6_leave_consensus - nuls_secure_memcmp address %d\n", nuls_secure_memcmp(txContext.outputAddress[0], BLACK_HOLE_ADDRESS, ADDRESS_LENGTH));
  PRINTF("tx_finalize_6_leave_consensus - nuls_secure_memcmp amount %d\n", nuls_secure_memcmp(txContext.outputAmount[0], BLACK_HOLE_ALIAS_AMOUNT, AMOUNT_LENGTH));
  */

  PRINTF("tx_finalize_6_leave_consensus - B\n");

  //Calculate fees (input - output)
  if (transaction_amount_sub_be(txContext.fees, txContext.totalInputAmount, txContext.totalOutputAmount)) {
    // L_DEBUG_APP(("Fee amount not consistent\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_6_leave_consensus - C\n");

  PRINTF("finalize. Fees: %.*H\n", AMOUNT_LENGTH, txContext.fees);

  ux.elements = ui_6_leave_consensus_nano;
  ux.elements_count = 9;
  totalSteps = 3;
  step_processor = stepProcessor_6_leave_consensus;
  ui_processor = uiProcessor_6_leave_consensus;
}
