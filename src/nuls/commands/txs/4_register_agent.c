#include "common_parser.h"
#include "4_register_agent.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

static const bagl_element_t ui_4_register_agent_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Register Agent from", 0x01),
  TITLE_ITEM("Deposit", 0x02),
  TITLE_ITEM("Agent Address", 0x03),
  TITLE_ITEM("Packaging Address", 0x04),
  TITLE_ITEM("Reward Address", 0x05),
  TITLE_ITEM("Commission Rate", 0x06),
  TITLE_ITEM("Remark", 0x07),
  TITLE_ITEM("Fees", 0x08),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_ARROW_RIGHT(0x04),
  ICON_ARROW_RIGHT(0x05),
  ICON_ARROW_RIGHT(0x06),
  ICON_ARROW_RIGHT(0x07),
  ICON_CHECK(0x08),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_4_register_agent(uint8_t step) {
  uint8_t nextStep = step + 1;

  if(step == 6 && txContext.remarkSize == 0) {
    nextStep++; // no remark
  }

  return nextStep;
}

static tx_type_specific_4_register_agent_t *cc = &(txContext.tx_fields.register_agent);

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
      amountTextSize = nuls_hex_amount_to_displayable(cc->deposit, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 3:
      //Agent Address
      nuls_address_to_encoded_base58(cc->agentAddress, lineBuffer);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 4:
      //Packaging Address
      nuls_address_to_encoded_base58(cc->packagingAddress, lineBuffer);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 5:
      //Reward Address
      nuls_address_to_encoded_base58(cc->rewardAddress, lineBuffer);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 6:
      //Commission Rate
      //nuls_double_to_displayable(cc->commissionRate, 40, lineBuffer);
      //snprintf(lineBuffer, 50, "%.5g percentage", cc->commissionRate);
      nuls_int_to_string((int) cc->commissionRate, lineBuffer); //TODO Fix double on monitor
      break;
    case 7:
      //Remark
      os_memmove(lineBuffer, &txContext.remark, txContext.remarkSize);
      lineBuffer[txContext.remarkSize] = '\0';
      break;
    case 8:
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

    case _4_REGISTER_AGENT_DEPOSIT:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_DEPOSIT;
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->deposit, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);

      // - Check here that deposit is more than Min Deposit
      if(nuls_secure_memcmp(txContext.tx_fields.join_consensus.deposit, MIN_DEPOSIT_REGISTER_AGENT, AMOUNT_LENGTH) < 0) {
        THROW(INVALID_PARAMETER);
      }

    case _4_REGISTER_AGENT_AGENT_ADDR:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_AGENT_ADDR;
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->agentAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);

    case _4_REGISTER_AGENT_PACKING_ADDR:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_PACKING_ADDR;
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->packagingAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);

      // agentAddress can not be the same as packingAddress
      if(nuls_secure_memcmp(cc->agentAddress, cc->packagingAddress, ADDRESS_LENGTH) == 0) {
        THROW(INVALID_PARAMETER);
      }

    case _4_REGISTER_AGENT_REWARD_ADDR:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_REWARD_ADDR;
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->rewardAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);

    case _4_REGISTER_AGENT_COMMISSION_RATE:
      txContext.tx_parsing_state = _4_REGISTER_AGENT_COMMISSION_RATE;
      is_available_to_parse(AMOUNT_LENGTH);
      //It's a Double LE...
      os_memmove(&(cc->commissionRate), txContext.bufferPointer, sizeof(double));
      transaction_offset_increase(AMOUNT_LENGTH);

      if(cc->commissionRate < 10 || cc->commissionRate > 100) {
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
  //Throw if:
  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    THROW(INVALID_PARAMETER);
  }

  //Stake (We reuse amount Spent)
  if (transaction_amount_add_be(txContext.amountSpent, txContext.amountSpent, cc->deposit)) {
    THROW(EXCEPTION_OVERFLOW);
  }

  ux.elements = ui_4_register_agent_nano;
  ux.elements_count = 19;
  totalSteps = 8;
  step_processor = stepProcessor_4_register_agent;
  ui_processor = uiProcessor_4_register_agent;
}
