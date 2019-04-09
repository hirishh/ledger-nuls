#include "common_parser.h"
#include "5_join_consensus.h"
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
  TITLE_ITEM("Remark", 0x04),
  TITLE_ITEM("Fees", 0x05),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_ARROW_RIGHT(0x04),
  ICON_CHECK(0x05),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_5_join_consensus(uint8_t step) {
  uint8_t nextStep = step + 1;

  if(step == 3 && txContext.remarkSize == 0) {
    nextStep++; // no remark
  }

  return nextStep;
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
      amountTextSize = nuls_hex_amount_to_displayable(txContext.tx_fields.join_consensus.deposit, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 3:
      //Agent Hash
      //snprintf(lineBuffer, 50, "%.*X", txContext.tx_fields.join_consensus.agentHash);
      //os_memmove(lineBuffer + 46, "...\0", 4);
      snprintf(lineBuffer, 50, "%.*H...%.*H",
              8, txContext.tx_fields.join_consensus.agentHash,
              8, txContext.tx_fields.join_consensus.agentHash + HASH_LENGTH - 8);
      break;
    case 4:
      //Remark
      os_memmove(lineBuffer, &txContext.remark, txContext.remarkSize);
      lineBuffer[txContext.remarkSize] = '\0';
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

void tx_parse_specific_5_join_consensus() {

  /* TX Structure:
   *
   * TX_SPECIFIC
   * - amount
   * - address
   * - agentHash
   *
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
      nuls_swap_bytes(txContext.tx_fields.join_consensus.deposit, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("deposit: %.*H\n", AMOUNT_LENGTH, txContext.tx_fields.join_consensus.deposit);

      // - Check here that deposit is more than Min Deposit
      if(nuls_secure_memcmp(txContext.tx_fields.join_consensus.deposit, MIN_DEPOSIT_JOIN_CONSENSUS, AMOUNT_LENGTH) < 0) {
        // PRINTF(("Number of output is wrong. Only 1 normal output and must be a black hole)\n"));
        THROW(INVALID_PARAMETER);
      }

    case _5_JOIN_CONS_ADDRESS:
      txContext.tx_parsing_state = _5_JOIN_CONS_ADDRESS;
      PRINTF("-- _5_JOIN_CONS_ADDRESS\n");
      is_available_to_parse(ADDRESS_LENGTH);
      //Save the address
      os_memmove(txContext.tx_fields.join_consensus.address, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("address: %.*H\n", ADDRESS_LENGTH, txContext.tx_fields.join_consensus.address);

      // - Check here that address is the same as accountFrom
      if(nuls_secure_memcmp(reqContext.accountFrom.address, txContext.tx_fields.join_consensus.address, ADDRESS_LENGTH) != 0) {
        // PRINTF(("Deposit address is different from account provided in input!\n"));
        THROW(INVALID_PARAMETER);
      }

    case _5_JOIN_CONS_AGENTHASH:
      txContext.tx_parsing_state = _5_JOIN_CONS_AGENTHASH;
      PRINTF("-- _5_JOIN_CONS_AGENTHASH\n");
      is_available_to_parse(HASH_LENGTH);
      os_memmove(txContext.tx_fields.join_consensus.agentHash, txContext.bufferPointer, HASH_LENGTH);
      transaction_offset_increase(HASH_LENGTH);
      PRINTF("agentHash: %.*H\n", HASH_LENGTH, txContext.tx_fields.join_consensus.agentHash);

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
  if(reqContext.accountChange.pathLength == 0) {
    PRINTF(("Change not provided!\n"));
    THROW(INVALID_PARAMETER);
  }

  //Stake (We reuse amount Spent)
  if (transaction_amount_add_be(txContext.amountSpent, txContext.amountSpent, txContext.tx_fields.join_consensus.deposit)) {
    PRINTF(("AmountSpent not consistent\n"));
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_5_join_consensus_nano;
  ux.elements_count = 13;
  totalSteps = 5;
  step_processor = stepProcessor_5_join_consensus;
  ui_processor = uiProcessor_5_join_consensus;
}
