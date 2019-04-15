#include "common_parser.h"
#include "101_call_contract.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

/**
 * Sign with address
 */
static const bagl_element_t ui_101_call_contract_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Call contract from", 0x01),
  TITLE_ITEM("Contract address", 0x02),
  TITLE_ITEM("Value", 0x03),
  TITLE_ITEM("Gas Limit", 0x04),
  TITLE_ITEM("Price", 0x05),
  TITLE_ITEM("MethodName", 0x06),
  TITLE_ITEM("Arguments", 0x07),
  TITLE_ITEM("Remark", 0x08),
  TITLE_ITEM("Fees", 0x09),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_ARROW_RIGHT(0x04),
  ICON_ARROW_RIGHT(0x05),
  ICON_ARROW_RIGHT(0x06),
  ICON_ARROW_RIGHT(0x07),
  ICON_ARROW_RIGHT(0x08),
  ICON_CHECK(0x09),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_101_call_contract(uint8_t step) {
  uint8_t nextStep = step + 1;
  if(step == 7 && txContext.remarkSize == 0) {
    nextStep++; // no remark
  }
  return nextStep;
}

static tx_type_specific_101_call_contract_t *cc = &(txContext.tx_fields.call_contract);

static void uiProcessor_101_call_contract(uint8_t step) {
  unsigned short amountTextSize;
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
    case 3:
      //Value
      amountTextSize = nuls_hex_amount_to_displayable(cc->value, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 4:
      //Gas Limit
      amountTextSize = nuls_hex_amount_to_displayable(cc->gasLimit, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 5:
      //Price
      amountTextSize = nuls_hex_amount_to_displayable(cc->price, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 6:
      //Method Name
      amountTextSize = MIN(50, cc->methodNameSize);
      os_memmove(lineBuffer, cc->methodName, amountTextSize);
      lineBuffer[amountTextSize] = '\0';
      break;
    case 7:
      //Args
      amountTextSize = MIN(50, cc->argsSize);
      os_memmove(lineBuffer, cc->args, amountTextSize);
      if(amountTextSize < 46) { //Remove tailing ", "
        lineBuffer[amountTextSize - 3] = '\0';
      } else {
        os_memmove(lineBuffer + 46, "...\0", 4);
      }
      break;
    case 8:
      //Remark
      os_memmove(lineBuffer, &txContext.remark, txContext.remarkSize);
      lineBuffer[txContext.remarkSize] = '\0';
      break;
    case 9:
      //Fees
      amountTextSize = nuls_hex_amount_to_displayable(txContext.fees, lineBuffer);
      lineBuffer[amountTextSize] = '\0';
      break;
    default:
      THROW(INVALID_STATE);
  }
}

void tx_parse_specific_101_call_contract() {

  /* TX Structure:
   *
   * TX_SPECIFIC (handled here)
   * - sender - ADDRESS_LENGTH
   * - contractAddress - ADDRESS_LENGTH
   * - value - AMOUNT_LENGTH
   * - gasLimit - AMOUNT_LENGTH
   * - price - AMOUNT_LENGTH
   * - len:methodName
   * - len:methodDesc
   * - argsn - 1 byte -> number of args (loop)
   *   -argn - 1 byte -> number of arg items
   *      - len:argitem
   *
   * */

  uint64_t tmpVarInt;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      PRINTF("-- BEGINNING\n");

    case _101_CALL_CONTRACT_SENDER:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_SENDER;
      PRINTF("-- _101_CALL_CONTRACT_SENDER\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->sender, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("sender: %.*H\n", ADDRESS_LENGTH, cc->sender);

      //Check here that sender is the same as accountFrom
      if(nuls_secure_memcmp(reqContext.accountFrom.address, cc->sender, ADDRESS_LENGTH) != 0) {
        // PRINTF(("Deposit address is different from account provided in input!\n"));
        THROW(INVALID_PARAMETER);
      }

    case _101_CALL_CONTRACT_CADDRESS:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_CADDRESS;
      PRINTF("-- _101_CALL_CONTRACT_CADDRESS\n");
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->contractAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);
      PRINTF("contractAddress: %.*H\n", ADDRESS_LENGTH, cc->contractAddress);

    case _101_CALL_CONTRACT_VALUE:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_VALUE;
      PRINTF("-- _101_CALL_CONTRACT_VALUE\n");
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->value, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("value: %.*H\n", AMOUNT_LENGTH, cc->value);

    case _101_CALL_CONTRACT_GASLIMIT:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_GASLIMIT;
      PRINTF("-- _101_CALL_CONTRACT_GASLIMIT\n");
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->gasLimit, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("gasLimit: %.*H\n", AMOUNT_LENGTH, cc->gasLimit);

    case _101_CALL_CONTRACT_PRICE:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_PRICE;
      PRINTF("-- _101_CALL_CONTRACT_PRICE\n");
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->price, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);
      PRINTF("price: %.*H\n", AMOUNT_LENGTH, cc->price);

    case _101_CALL_CONTRACT_METHODNAME_LENGTH:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_METHODNAME_LENGTH;
      PRINTF("-- _101_CALL_CONTRACT_METHODNAME_LENGTH\n");
      cc->methodNameSize = transaction_get_varint();

      if(cc->methodNameSize > MAX_METHODNAME_LENGTH) {
        THROW(INVALID_PARAMETER);
      }

    case _101_CALL_CONTRACT_METHODNAME:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_METHODNAME;
      PRINTF("-- _101_CALL_CONTRACT_METHODNAME\n");
      is_available_to_parse(cc->methodNameSize);
      os_memmove(cc->methodName, txContext.bufferPointer, cc->methodNameSize);
      cc->methodName[cc->methodNameSize] = '\0';
      transaction_offset_increase(cc->methodNameSize);
      PRINTF("methodName: %s\n", cc->methodName);

    case _101_CALL_CONTRACT_METHODDESC_LENGTH:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_METHODDESC_LENGTH;
      PRINTF("-- _101_CALL_CONTRACT_METHODDESC_LENGTH\n");
      tmpVarInt = transaction_get_varint();

    case _101_CALL_CONTRACT_METHODDESC:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_METHODDESC;
      PRINTF("-- _101_CALL_CONTRACT_METHODDESC\n");
      //is_available_to_parse(tmpVarInt);
      transaction_offset_increase(tmpVarInt);

    case _101_CALL_CONTRACT_ARGS_I:
      txContext.tx_parsing_state = _101_CALL_CONTRACT_ARGS_I; // Will be done in the next switch
    case _101_CALL_CONTRACT_ARGS_J:
    case _101_CALL_CONTRACT_ARG_LENGTH:
    case _101_CALL_CONTRACT_ARG:
      break; // Go inside do-while

    default:
      THROW(INVALID_STATE);
  }

  do {

    switch(txContext.tx_parsing_state) {

      case _101_CALL_CONTRACT_ARGS_I:
        txContext.tx_parsing_state = _101_CALL_CONTRACT_ARGS_I;
        PRINTF("-- _101_CALL_CONTRACT_ARGS_I\n");
        is_available_to_parse(1);
        cc->arg_i = txContext.bufferPointer[0];
        cc->curr_i = 0;
        transaction_offset_increase(1);
        PRINTF("-- args i: %d\n", cc->arg_i);

      case _101_CALL_CONTRACT_ARGS_J:
        txContext.tx_parsing_state = _101_CALL_CONTRACT_ARGS_J;
        PRINTF("-- _101_CALL_CONTRACT_ARGS_J\n");
        is_available_to_parse(1);
        cc->arg_j = txContext.bufferPointer[0];
        cc->curr_j = 0;
        transaction_offset_increase(1);
        PRINTF("-- args j: %d\n", cc->arg_j);

      case _101_CALL_CONTRACT_ARG_LENGTH:
        txContext.tx_parsing_state = _101_CALL_CONTRACT_ARG_LENGTH;
        PRINTF("-- _101_CALL_CONTRACT_ARG_LENGTH\n");
        cc->argLength = transaction_get_varint();
        PRINTF("-- args [%d][%d] length: %d\n", cc->curr_i, cc->curr_j, (unsigned char) cc->argLength);

      case _101_CALL_CONTRACT_ARG:
        txContext.tx_parsing_state = _101_CALL_CONTRACT_ARG;
        PRINTF("-- _101_CALL_CONTRACT_ARG\n");
        is_available_to_parse(cc->argLength);

        char charToVideo = MIN(50 - cc->argsSize - 3, cc->argLength); // -3 because of ", \0" at the end
        PRINTF("-- charToVideo: %d\n", charToVideo);
        PRINTF("-- current argSize: %d\n", cc->argsSize);
        if(charToVideo > 0 && cc->argsSize < ( 50 - charToVideo )) {
          //if cc->argsSize is not 0, remove \0
          if(cc->argsSize > 0) cc->argsSize--;
          os_memmove(cc->args + cc->argsSize, txContext.bufferPointer, charToVideo);
          cc->argsSize += charToVideo;
          cc->args[cc->argsSize] = ',';
          cc->args[cc->argsSize + 1] = ' ';
          cc->args[cc->argsSize + 2] = '\0';
          cc->argsSize += 3;
        }

        transaction_offset_increase(cc->argLength);
        PRINTF("-- temp arg display: %s\n", cc->args);


        //Calculate next state
        txContext.tx_parsing_state = _101_CALL_CONTRACT_ARG_LENGTH;
        cc->curr_j++;
        if(cc->curr_j == cc->arg_j) {
          txContext.tx_parsing_state = _101_CALL_CONTRACT_ARGS_J;
          cc->curr_i++;
        }
        PRINTF("-- arg_i: %d\n", cc->arg_i);
        PRINTF("-- curr_i: %d\n", cc->curr_i);
        PRINTF("-- arg_j: %d\n", cc->arg_j);
        PRINTF("-- curr_j: %d\n", cc->curr_j);
        break;

      default:
        THROW(INVALID_STATE);

    }

  }
  while (cc->curr_i < cc->arg_i);

  PRINTF("-- OUT FROM tx_parse_specific_101_call_contract\n");

  //It's time for CoinData
  txContext.tx_parsing_group = COIN_INPUT;
  txContext.tx_parsing_state = BEGINNING;
}

void tx_finalize_101_call_contract() {
  PRINTF("tx_finalize_101_call_contract\n");

  //Throw if:

  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    PRINTF(("Change not provided!\n"));
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_101_call_contract_nano;
  ux.elements_count = 21;
  totalSteps = 9;
  step_processor = stepProcessor_101_call_contract;
  ui_processor = uiProcessor_101_call_contract;
}
