#include "common_parser.h"
#include "102_delete_contract.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_102_delete_contract_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Delete contract from", 0x01),
  TITLE_ITEM("Contract address", 0x02),
  TITLE_ITEM("Remark", 0x03),
  TITLE_ITEM("Fees", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_102_delete_contract(uint8_t step) {
  uint8_t nextStep = step + 1;
  if(step == 2 && txContext.remarkSize == 0) {
    nextStep++; // no remark
  }
  return nextStep;
}

static tx_type_specific_102_delete_contract_t *cc = &(txContext.tx_fields.delete_contract);

static void uiProcessor_102_delete_contract(uint8_t step) {
  unsigned short amountTextSize;
  os_memset(lineBuffer, 0, sizeof(lineBuffer));
  switch (step) {
    case 1:
      //Call contract from
      os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, BASE58_ADDRESS_LENGTH);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 2:
      //Contract Address
      nuls_address_to_encoded_base58(cc->contractAddress, lineBuffer);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
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

void tx_parse_specific_102_delete_contract() {

  /* TX Structure:
   *
   * TX_SPECIFIC
   * - sender - ADDRESS_LENGTH
   * - contractAddress - ADDRESS_LENGTH
   *
   * */

  uint64_t tmpVarInt;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:

    case _102_DELETE_CONTRACT_SENDER:
      txContext.tx_parsing_state = _102_DELETE_CONTRACT_SENDER;
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->sender, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);

      //Check here that sender is the same as accountFrom
      if(nuls_secure_memcmp(reqContext.accountFrom.address, cc->sender, ADDRESS_LENGTH) != 0) {
        THROW(INVALID_PARAMETER);
      }

    case _102_DELETE_CONTRACT_CADDRESS:
      txContext.tx_parsing_state = _102_DELETE_CONTRACT_CADDRESS;
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->contractAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;

      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_102_delete_contract() {
  //Throw if:
  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_102_delete_contract_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_102_delete_contract;
  ui_processor = uiProcessor_102_delete_contract;
}
