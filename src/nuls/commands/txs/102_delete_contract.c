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
  ICON_ARROW_RIGHT(0x01),
  ICON_CHECK(0x02),
  ICON_CROSS(0x00),
  LINEBUFFER,
};


static uint8_t stepProcessor_102_delete_contract(uint8_t step) {
  return step + 1;
}

static void uiProcessor_102_delete_contract(uint8_t step) {
  unsigned short amountTextSize;
  tx_type_specific_102_delete_contract_t *cc = &(txContext.tx_fields.delete_contract);
  os_memset(lineBuffer, 0, 50);
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

  tx_type_specific_102_delete_contract_t *cc = &(txContext.tx_fields.delete_contract);
  uint64_t tmpVarInt;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      PRINTF("-- BEGINNING\n");

    case _102_DELETE_CONTRACT_SENDER:
      txContext.tx_parsing_state = _102_DELETE_CONTRACT_SENDER;
      PRINTF("-- _102_DELETE_CONTRACT_SENDER\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->sender, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("sender: %.*H\n", ADDRESS_LENGTH, cc->sender);

      //Check here that sender is the same as accountFrom
      if(nuls_secure_memcmp(reqContext.accountFrom.address, cc->sender, ADDRESS_LENGTH) != 0) {
        // PRINTF(("Deposit address is different from account provided in input!\n"));
        THROW(INVALID_PARAMETER);
      }

    case _102_DELETE_CONTRACT_CADDRESS:
      txContext.tx_parsing_state = _102_DELETE_CONTRACT_CADDRESS;
      PRINTF("-- _102_DELETE_CONTRACT_CADDRESS\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->contractAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("contractAddress: %.*H\n", ADDRESS_LENGTH, cc->contractAddress);

      //It's time for CoinData
      txContext.tx_parsing_group = COIN_INPUT;
      txContext.tx_parsing_state = BEGINNING;

      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_102_delete_contract() {
  PRINTF("tx_finalize_102_delete_contract\n");

  //Throw if:

  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    // PRINTF(("Change not provided!\n"));
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_102_delete_contract_nano;
  ux.elements_count = 7;
  totalSteps = 2;
  step_processor = stepProcessor_102_delete_contract;
  ui_processor = uiProcessor_102_delete_contract;
}
