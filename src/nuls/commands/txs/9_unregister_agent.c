#include "common_parser.h"
#include "9_unregister_agent.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_9_unregister_agent_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Unregister Agent for", 0x01),
  TITLE_ITEM("TX Hash", 0x02),
  TITLE_ITEM("Fees", 0x03),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_CHECK(0x03),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_9_unregister_agent(uint8_t step) {
  return step + 1;
}

static void uiProcessor_9_unregister_agent(uint8_t step) {
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
              8, txContext.tx_fields.unregister_agent.txHash,
              8, txContext.tx_fields.unregister_agent.txHash + HASH_LENGTH - 8);
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

void tx_parse_specific_9_unregister_agent() {

  /* TX Structure:
   *
   * TX_SPECIFIC
   * - createTxHash -> HASH_LENGTH
   *
   * */

  uint64_t tmpVarInt = 0;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      PRINTF("-- BEGINNING\n");

    case _9_UNREGISTER_AGENT_TXHASH:
      txContext.tx_parsing_state = _9_UNREGISTER_AGENT_TXHASH;
      PRINTF("-- _9_UNREGISTER_AGENT_TXHASH\n");
      is_available_to_parse(HASH_LENGTH);
      os_memmove(txContext.tx_fields.unregister_agent.txHash, txContext.bufferPointer, HASH_LENGTH);
      transaction_offset_increase(HASH_LENGTH);
      PRINTF("txHash: %.*H\n", HASH_LENGTH, txContext.tx_fields.unregister_agent.txHash);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_9_unregister_agent() {
  PRINTF("tx_finalize_9_unregister_agent\n");

  //Throw if:

  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    PRINTF(("Change not provided!\n"));
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_9_unregister_agent_nano;
  ux.elements_count = 9;
  totalSteps = 3;
  step_processor = stepProcessor_9_unregister_agent;
  ui_processor = uiProcessor_9_unregister_agent;
}
