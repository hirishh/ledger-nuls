#include "common_parser.h"
#include "100_create_contract.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

static const bagl_element_t ui_100_create_contract_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Create contract from", 0x01),
  TITLE_ITEM("Contract address", 0x02),
  TITLE_ITEM("Value", 0x03),
  TITLE_ITEM("Gas Limit", 0x04),
  TITLE_ITEM("Price", 0x05),
  TITLE_ITEM("Code Hash", 0x06),
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

static uint8_t stepProcessor_100_create_contract(uint8_t step) {
  uint8_t nextStep = step + 1;
  if(step == 7 && txContext.remarkSize == 0) {
    nextStep++; // no remark
  }
  return nextStep;
}

static tx_type_specific_100_create_contract_t *cc = &(txContext.tx_fields.create_contract);

static void uiProcessor_100_create_contract(uint8_t step) {
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
      //Code Hash
      snprintf(lineBuffer, sizeof(lineBuffer), "%.*H...%.*H",
              4, cc->codeDigest,
              4, cc->codeDigest + DIGEST_LENGTH - 4);
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

void tx_parse_specific_100_create_contract() {

  /* TX Structure:
   *
   * TX_SPECIFIC (handled here)
   * - sender - ADDRESS_LENGTH
   * - contractAddress - ADDRESS_LENGTH
   * - value - AMOUNT_LENGTH
   * - codelen - 32bytes
   * - code
   * - gasLimit - AMOUNT_LENGTH
   * - price - AMOUNT_LENGTH
   * - argsn - 1 byte -> number of args (loop)
   *   -argn - 1 byte -> number of arg items
   *      - len:argitem
   *
   * */

  uint64_t tmpVarInt;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      //init sha256
      cx_sha256_init(&cc->codeHash);

    case _100_CREATE_CONTRACT_SENDER:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_SENDER;
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->sender, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);

      //Check here that sender is the same as accountFrom
      if(nuls_secure_memcmp(reqContext.accountFrom.address, cc->sender, ADDRESS_LENGTH) != 0) {
        THROW(INVALID_PARAMETER);
      }

    case _100_CREATE_CONTRACT_CADDRESS:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_CADDRESS;
      is_available_to_parse(ADDRESS_LENGTH);
      os_memmove(cc->contractAddress, txContext.bufferPointer, ADDRESS_LENGTH);
      transaction_offset_increase(ADDRESS_LENGTH);

    case _100_CREATE_CONTRACT_VALUE:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_VALUE;
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->value, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);

    case _100_CREATE_CONTRACT_CODELEN:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_VALUE;
      is_available_to_parse(4);
      cc->codeLen = nuls_read_u32(txContext.bufferPointer, 1, 0);
      cc->codeLenMissing = cc->codeLen;
      transaction_offset_increase(4);

    case _100_CREATE_CONTRACT_CODE_LENGTH:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_CODE_LENGTH;
      tmpVarInt = transaction_get_varint();

      if(tmpVarInt != cc->codeLen) {
        THROW(INVALID_PARAMETER);
      }

    case _100_CREATE_CONTRACT_CODE:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_CODE;
      tmpVarInt = MIN(cc->codeLenMissing, txContext.bytesChunkRemaining);
      cx_hash(&cc->codeHash, 0, txContext.bufferPointer, tmpVarInt, NULL, 0);
      cc->codeLenMissing -= tmpVarInt;
      transaction_offset_increase(tmpVarInt);

      //Check if we need next chunk
      if(txContext.bytesChunkRemaining == 0 && cc->codeLenMissing != 0) {
        THROW(NEED_NEXT_CHUNK);
      }

      if(cc->codeLenMissing == 0) {
        //let's finalize the hash
        unsigned char fake[1];
        cx_hash(&cc->codeHash.header, CX_LAST, fake, 0, cc->codeDigest, DIGEST_LENGTH);
      }

    case _100_CREATE_CONTRACT_GASLIMIT:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_GASLIMIT;
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->gasLimit, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);

    case _100_CREATE_CONTRACT_PRICE:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_PRICE;
      is_available_to_parse(AMOUNT_LENGTH);
      nuls_swap_bytes(cc->price, txContext.bufferPointer, AMOUNT_LENGTH);
      transaction_offset_increase(AMOUNT_LENGTH);

    case _100_CREATE_CONTRACT_ARGS_I:
      txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARGS_I; // Will be done in the next switch
    case _100_CREATE_CONTRACT_ARGS_J:
    case _100_CREATE_CONTRACT_ARG_LENGTH:
    case _100_CREATE_CONTRACT_ARG:
      break; // Go inside do-while

    default:
      THROW(INVALID_STATE);
  }

  do {

    switch(txContext.tx_parsing_state) {

      case _100_CREATE_CONTRACT_ARGS_I:
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARGS_I;
        is_available_to_parse(1);
        cc->arg_i = txContext.bufferPointer[0];
        cc->curr_i = 0;
        transaction_offset_increase(1);

      case _100_CREATE_CONTRACT_ARGS_J:
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARGS_J;
        is_available_to_parse(1);
        cc->arg_j = txContext.bufferPointer[0];
        cc->curr_j = 0;
        transaction_offset_increase(1);

      case _100_CREATE_CONTRACT_ARG_LENGTH:
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARG_LENGTH;
        cc->argLength = transaction_get_varint();

      case _100_CREATE_CONTRACT_ARG:
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARG;
        is_available_to_parse(cc->argLength);

        char charToVideo = MIN(50 - cc->argsSize - 3, cc->argLength); // -3 because of ", \0" at the end
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

        //Calculate next state
        txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARG_LENGTH;
        cc->curr_j++;
        if(cc->curr_j == cc->arg_j) {
          txContext.tx_parsing_state = _100_CREATE_CONTRACT_ARGS_J;
          cc->curr_i++;
        }
        break;

      default:
        THROW(INVALID_STATE);

    }

  }
  while (cc->curr_i < cc->arg_i);

  //It's time for CoinData
  txContext.tx_parsing_group = COIN_INPUT;
  txContext.tx_parsing_state = BEGINNING;
}

void tx_finalize_100_create_contract() {
  //Throw if:
  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_100_create_contract_nano;
  ux.elements_count = 21;
  totalSteps = 9;
  step_processor = stepProcessor_100_create_contract;
  ui_processor = uiProcessor_100_create_contract;
}
