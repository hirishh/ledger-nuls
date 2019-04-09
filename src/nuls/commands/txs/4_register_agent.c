#include "common_parser.h"
#include "4_register_agent.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_4_register_agent_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Register Agent from", 0x01),
  TITLE_ITEM("Deposit", 0x02),
  TITLE_ITEM("Agent Address", 0x03),
  TITLE_ITEM("Packaging Address", 0x04),
  TITLE_ITEM("Reward Address", 0x05),
  TITLE_ITEM("Commission Rate", 0x06),
  TITLE_ITEM("Fees", 0x07),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_ARROW_RIGHT(0x04),
  ICON_ARROW_RIGHT(0x05),
  ICON_ARROW_RIGHT(0x06),
  ICON_CHECK(0x07),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_4_register_agent(uint8_t step) {
  return step + 1;
}

static void uiProcessor_4_register_agent(uint8_t step) {
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
      amountTextSize = nuls_hex_amount_to_displayable(txContext.tx_fields.register_agent.deposit, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 3:
      //Agent Address
      nuls_address_to_encoded_base58(txContext.tx_fields.register_agent.agentAddress, lineBuffer);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 4:
      //Packaging Address
      nuls_address_to_encoded_base58(txContext.tx_fields.register_agent.packagingAddress, lineBuffer);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 5:
      //Reward Address
      nuls_address_to_encoded_base58(txContext.tx_fields.register_agent.rewardAddress, lineBuffer);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 6:
      //Commission Rate
      //nuls_double_to_displayable(txContext.tx_fields.register_agent.commissionRate, 40, lineBuffer);
      //snprintf(lineBuffer, 50, "%.5g percentage", txContext.tx_fields.register_agent.commissionRate);
      nuls_int_to_string((int) txContext.tx_fields.register_agent.commissionRate, lineBuffer); //TODO Fix double on monitor
      break;
    case 7:
      //Fees
      amountTextSize = nuls_hex_amount_to_displayable(txContext.fees, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    default:
      THROW(INVALID_STATE);
  }
}

void tx_parse_specific_4_register_agent() {

  /* TX Structure:
   *
   * TX_SPECIFIC
   * - depositAmount -> AMOUNT_LENGTH
   * - agentAddress -> ADDRESS_LENGTH
   * - packingAddress -> ADDRESS_LENGTH
   * - rewardAddress -> ADDRESS_LENGTH
   * - commissionRate -> Double LE (8 bytes)
   *
   * */

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      PRINTF("-- BEGINNING\n");


    case _4_REGISTER_AGENT_DEPOSIT:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_DEPOSIT;
      PRINTF("-- _4_REGISTER_AGENT_DEPOSIT\n");
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(txContext.tx_fields.register_agent.deposit, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("deposit: %.*H\n", AMOUNT_LENGTH, txContext.tx_fields.register_agent.deposit);

      // - Check here that deposit is more than Min Deposit
      if(nuls_secure_memcmp(txContext.tx_fields.join_consensus.deposit, MIN_DEPOSIT_REGISTER_AGENT, AMOUNT_LENGTH) < 0) {
        PRINTF(("Invalid deposit, should be equal or greater than 20000 nuls)\n"));
        THROW(INVALID_PARAMETER);
      }

    case _4_REGISTER_AGENT_AGENT_ADDR:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_AGENT_ADDR;
      PRINTF("-- _4_REGISTER_AGENT_AGENT_ADDR\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(txContext.tx_fields.register_agent.agentAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("agentAddress: %.*H\n", ADDRESS_LENGTH, txContext.tx_fields.register_agent.agentAddress);

    case _4_REGISTER_AGENT_PACKING_ADDR:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_PACKING_ADDR;
      PRINTF("-- _4_REGISTER_AGENT_PACKING_ADDR\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(txContext.tx_fields.register_agent.packagingAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("packagingAddress: %.*H\n", ADDRESS_LENGTH, txContext.tx_fields.register_agent.packagingAddress);

      // agentAddress can not be the same as packingAddress
      if(nuls_secure_memcmp(txContext.tx_fields.register_agent.agentAddress, txContext.tx_fields.register_agent.packagingAddress, ADDRESS_LENGTH) == 0) {
        PRINTF(("agentAddress can not be the same as packingAddress!\n"));
        THROW(INVALID_PARAMETER);
      }

    case _4_REGISTER_AGENT_REWARD_ADDR:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_REWARD_ADDR;
      PRINTF("-- _4_REGISTER_AGENT_REWARD_ADDR\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(txContext.tx_fields.register_agent.rewardAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("rewardAddress: %.*H\n", ADDRESS_LENGTH, txContext.tx_fields.register_agent.rewardAddress);

    case _4_REGISTER_AGENT_COMMISSION_RATE:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_COMMISSION_RATE;
      PRINTF("-- _4_REGISTER_AGENT_COMMISSION_RATE\n");
      is_available_to_parse(AMOUNT_LENGTH);
      //It's a Double LE...
      os_memmove(&(txContext.tx_fields.register_agent.commissionRate), txContext.bufferPointer, sizeof(double));
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("commissionRate: %.*H\n", AMOUNT_LENGTH, &txContext.tx_fields.register_agent.commissionRate);
      PRINTF("commissionRate: %.5g\n", txContext.tx_fields.register_agent.commissionRate);
      PRINTF("commissionRate: %d\n", (int) txContext.tx_fields.register_agent.commissionRate);

      if(txContext.tx_fields.register_agent.commissionRate < 10 || txContext.tx_fields.register_agent.commissionRate > 100) {
        PRINTF(("Invalid commission rate, should be between [10, 100]\n"));
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

void tx_finalize_4_register_agent() {
  PRINTF("tx_finalize_4_register_agent\n");

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

  ux.elements = ui_4_register_agent_nano;
  ux.elements_count = 17;
  totalSteps = 7;
  step_processor = stepProcessor_4_register_agent;
  ui_processor = uiProcessor_4_register_agent;
}
