#include "common_parser.h"
#include "6_leave_consensus.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

static const bagl_element_t ui_6_leave_consensus_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Leave Consensus for", 0x01),
  TITLE_ITEM("TX Hash", 0x02),
  TITLE_ITEM("Remark", 0x03),
  TITLE_ITEM("Fees", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_6_leave_consensus(uint8_t step) {
  uint8_t nextStep = step + 1;
  if(step == 2 && txContext.remarkSize == 0) {
    nextStep++; // no remark
  }
  return nextStep;
}

static tx_type_specific_6_leave_consensus_t *cc = &(txContext.tx_fields.leave_consensus);

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
              4, cc->txHash,
              4, cc->txHash + HASH_LENGTH - 4);
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

void tx_parse_specific_6_leave_consensus() {

  /* TX Structure:
   *
   * TX_SPECIFIC (handled here)
   * - txHash -> HASH_LENGTH
   *
   * */

  uint64_t tmpVarInt = 0;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:

    case _6_LEAVE_CONS_TXHASH:
      txContext.tx_parsing_state = _6_LEAVE_CONS_TXHASH;
      is_available_to_parse(HASH_LENGTH);
      os_memmove(cc->txHash, txContext.bufferPointer, HASH_LENGTH);
      transaction_offset_increase(HASH_LENGTH);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_6_leave_consensus() {
  //Throw if:
  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_6_leave_consensus_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_6_leave_consensus;
  ui_processor = uiProcessor_6_leave_consensus;
}
