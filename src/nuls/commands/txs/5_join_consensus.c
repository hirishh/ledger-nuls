#include "common_parser.h"
#include "2_transfer.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_5_join_consensus_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Join Consensus for", 0x01),
  TITLE_ITEM("Deposit", 0x02),
  TITLE_ITEM("Agent Hash", 0x03),
  TITLE_ITEM("Fees", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_5_join_consensus(uint8_t step) {
  return step + 1;
}

static void uiProcessor_5_join_consensus(uint8_t step) {
  unsigned short amountTextSize;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      //Join Consensus for address
      os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, BASE58_ADDRESS_LENGTH);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 2:
      //Deposit
      amountTextSize = nuls_hex_amount_to_displayable(txContext.tx_specific_fields.join_consensus.deposit, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 3:
      //Agent Hash
      //snprintf(lineBuffer, 50, "%.*X", txContext.tx_specific_fields.join_consensus.agentHash);
      //os_memmove(lineBuffer + 46, "...\0", 4);
      snprintf(lineBuffer, 50, "%.*H...%.*H",
              8, txContext.tx_specific_fields.join_consensus.agentHash,
              8, txContext.tx_specific_fields.join_consensus.agentHash + HASH_LENGTH - 8);
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

void tx_parse_specific_5_join_consensus() {

  /* TX Structure:
   *
   * COMMON
   * - type -> 2 Bytes
   * - time -> 6 Bytes
   * - remarkLength -> 1 Byte
   * - remark -> remarkLength Bytes (max 30 bytes)
   *
   * TX_SPECIFIC (handled here)
   * - amount
   * - address
   * - agentHash
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

    case _5_JOIN_CONS_DEPOSIT:
      txContext.tx_parsing_state = _5_JOIN_CONS_DEPOSIT;
      PRINTF("-- _5_JOIN_CONS_DEPOSIT\n");
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(txContext.tx_specific_fields.join_consensus.deposit, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("deposit: %.*H\n", AMOUNT_LENGTH, txContext.tx_specific_fields.join_consensus.deposit);

    case _5_JOIN_CONS_ADDRESS:
      txContext.tx_parsing_state = _5_JOIN_CONS_ADDRESS;
      PRINTF("-- _5_JOIN_CONS_ADDRESS\n");
      is_available_to_parse(ADDRESS_LENGTH);
      //Save the address
      os_memmove(txContext.tx_specific_fields.join_consensus.address, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("address: %.*H\n", ADDRESS_LENGTH, txContext.tx_specific_fields.join_consensus.address);

    case _5_JOIN_CONS_AGENTHASH:
      txContext.tx_parsing_state = _5_JOIN_CONS_AGENTHASH;
      PRINTF("-- _5_JOIN_CONS_AGENTHASH\n");
      is_available_to_parse(HASH_LENGTH);
      os_memmove(txContext.tx_specific_fields.join_consensus.agentHash, txContext.bufferPointer, HASH_LENGTH);
      transaction_offset_increase(HASH_LENGTH);
      PRINTF("agentHash: %.*H\n", HASH_LENGTH, txContext.tx_specific_fields.join_consensus.agentHash);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_5_join_consensus() {
  PRINTF("tx_finalize_5_join_consensus\n");

  //Throw if:

  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0 || (reqContext.accountChange.pathLength > 0 && !txContext.changeFound)) {
    // PRINTF(("Change not provided!\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_5_join_consensus - A\n");

  // - addresFrom is different from join_consensus.address
  if(nuls_secure_memcmp(reqContext.accountFrom.address, txContext.tx_specific_fields.join_consensus.address, ADDRESS_LENGTH) != 0) {
    // PRINTF(("Deposit address is different from account provided in input!\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_5_join_consensus - B\n");

  /* Not Really necessary
  // - changeFound is parsed correctly but it's not equal to alias.address
  if(reqContext.accountChange.pathLength > 0 && txContext.changeFound &&
     nuls_secure_memcmp(reqContext.accountChange.address, txContext.tx_specific_fields.alias.address, ADDRESS_LENGTH) != 0) {
    // PRINTF(("Change Address provided but it's different from tx specific alias address\n"));
    THROW(INVALID_PARAMETER);
  }
  */

  PRINTF("tx_finalize_5_join_consensus - nuls_secure_memcmp min deposit %d\n",
          nuls_secure_memcmp(txContext.tx_specific_fields.join_consensus.deposit, MIN_DEPOSIT_JOIN_CONSENSUS, AMOUNT_LENGTH));

  if(nuls_secure_memcmp(txContext.tx_specific_fields.join_consensus.deposit, MIN_DEPOSIT_JOIN_CONSENSUS, AMOUNT_LENGTH) < 0) {
    // PRINTF(("Number of output is wrong. Only 1 normal output and must be a black hole)\n"));
    THROW(INVALID_PARAMETER);
  }

  /*
  PRINTF("tx_finalize_5_join_consensus - nOut %d\n", txContext.nOut);
  PRINTF("tx_finalize_5_join_consensus - txContext.outputAddress %.*H\n", ADDRESS_LENGTH, txContext.outputAddress[0]);
  PRINTF("tx_finalize_5_join_consensus - BLACK_HOLE_ADDRESS %.*H\n", ADDRESS_LENGTH, BLACK_HOLE_ADDRESS);
  PRINTF("tx_finalize_5_join_consensus - txContext.outputAmount %.*H\n", AMOUNT_LENGTH, txContext.outputAmount[0]);
  PRINTF("tx_finalize_5_join_consensus - BLACK_HOLE_ALIAS_AMOUNT %.*H\n", AMOUNT_LENGTH, BLACK_HOLE_ALIAS_AMOUNT);
  PRINTF("tx_finalize_5_join_consensus - nuls_secure_memcmp address %d\n", nuls_secure_memcmp(txContext.outputAddress[0], BLACK_HOLE_ADDRESS, ADDRESS_LENGTH));
  PRINTF("tx_finalize_5_join_consensus - nuls_secure_memcmp amount %d\n", nuls_secure_memcmp(txContext.outputAmount[0], BLACK_HOLE_ALIAS_AMOUNT, AMOUNT_LENGTH));

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

   */

  PRINTF("tx_finalize_5_join_consensus - C\n");

  //Calculate fees (input - output)
  if (transaction_amount_sub_be(txContext.fees, txContext.totalInputAmount, txContext.totalOutputAmount)) {
    // L_DEBUG_APP(("Fee amount not consistent\n"));
    THROW(INVALID_PARAMETER);
  }

  //Stake (We reuse amount Spent
  if (transaction_amount_add_be(txContext.amountSpent, txContext.amountSpent, txContext.tx_specific_fields.join_consensus.deposit)) {
    // L_DEBUG_APP(("Fee amount not consistent\n"));
    THROW(INVALID_PARAMETER);
  }

  PRINTF("tx_finalize_5_join_consensus - D\n");

  PRINTF("finalize. Fees: %.*H\n", AMOUNT_LENGTH, txContext.fees);

  ux.elements = ui_5_join_consensus_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_5_join_consensus;
  ui_processor = uiProcessor_5_join_consensus;
}
