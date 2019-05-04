#include "common_parser.h"
#include "5_join_consensus.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

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

static tx_type_specific_5_join_consensus_t *cc = &(txContext.tx_fields.join_consensus);

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
      amountTextSize = nuls_hex_amount_to_displayable(cc->deposit, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 3:
      //Agent Hash
      //snprintf(lineBuffer, 50, "%.*X", cc->agentHash);
      //os_memmove(lineBuffer + 46, "...\0", 4);
      snprintf(lineBuffer, 50, "%.*H...%.*H",
              4, cc->agentHash,
              4, cc->agentHash + HASH_LENGTH - 4);
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

    case _5_JOIN_CONS_DEPOSIT:
      txContext.tx_parsing_state = _5_JOIN_CONS_DEPOSIT;
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->deposit, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);

      // - Check here that deposit is more than Min Deposit
      if(nuls_secure_memcmp(cc->deposit, MIN_DEPOSIT_JOIN_CONSENSUS, AMOUNT_LENGTH) < 0) {
        THROW(INVALID_PARAMETER);
      }

    case _5_JOIN_CONS_ADDRESS:
      txContext.tx_parsing_state = _5_JOIN_CONS_ADDRESS;
      is_available_to_parse(ADDRESS_LENGTH);
      //Save the address
      os_memmove(cc->address, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);

      // - Check here that address is the same as accountFrom
      if(nuls_secure_memcmp(reqContext.accountFrom.address, cc->address, ADDRESS_LENGTH) != 0) {
        THROW(INVALID_PARAMETER);
      }

    case _5_JOIN_CONS_AGENTHASH:
      txContext.tx_parsing_state = _5_JOIN_CONS_AGENTHASH;
      is_available_to_parse(HASH_LENGTH);
      os_memmove(cc->agentHash, txContext.bufferPointer, HASH_LENGTH);
      transaction_offset_increase(HASH_LENGTH);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_5_join_consensus() {
  //Throw if:
  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    THROW(INVALID_PARAMETER);
  }

  //Stake (We reuse amount Spent)
  if (transaction_amount_add_be(txContext.amountSpent, txContext.amountSpent, cc->deposit)) {
    THROW(EXCEPTION_OVERFLOW);
  }

  ux.elements = ui_5_join_consensus_nano;
  ux.elements_count = 13;
  totalSteps = 5;
  step_processor = stepProcessor_5_join_consensus;
  ui_processor = uiProcessor_5_join_consensus;
}
