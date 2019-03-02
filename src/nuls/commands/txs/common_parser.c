#include "common_parser.h"
#include "../../nuls_internals.h"

void parse_group_common() {

  if(txContext.tx_parsing_group != COMMON) {
    THROW(INVALID_STATE);
  }

  uint64_t remarkVarInt;

  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      PRINTF("-- BEGINNING\n");
      // Reset transaction state
      txContext.remainingInputsOutputs = 0;
      txContext.currentInputOutput = 0;
      os_memset(txContext.totalInputAmount, 0, sizeof(txContext.totalInputAmount));
      os_memset(txContext.totalOutputAmount, 0, sizeof(txContext.totalOutputAmount));
      os_memset(txContext.outputAmount, 0, sizeof(txContext.totalOutputAmount));
      os_memset(txContext.changeAmount, 0, sizeof(txContext.totalOutputAmount));
      os_memset(txContext.fees, 0, sizeof(txContext.totalOutputAmount));
      //Done, start with the next field
      txContext.tx_parsing_state = FIELD_TYPE;
      //no break is intentional

    case FIELD_TYPE:
      PRINTF("-- FIELD_TYPE\n");
      //already parsed..
      is_available_to_parse(2);
      transaction_offset_increase(2);
      txContext.tx_parsing_state = FIELD_TIME;
      //no break is intentional

    case FIELD_TIME:
      PRINTF("-- FIELD_TIME\n");
      is_available_to_parse(6);
      transaction_offset_increase(6);
      txContext.tx_parsing_state = FIELD_REMARK_LENGTH;
      //no break is intentional

    case FIELD_REMARK_LENGTH:
      PRINTF("-- FIELD_REMARK_LENGTH\n");
      remarkVarInt = transaction_get_varint();
      if(remarkVarInt > REMARK_LENGTH) {
        THROW(INVALID_PARAMETER);
      }
      txContext.remarkSize = (unsigned char) remarkVarInt;
      txContext.tx_parsing_state = FIELD_REMARK;
      //no break is intentional

    case FIELD_REMARK:
      PRINTF("-- FIELD_REMARK\n");
      if (txContext.remarkSize != 0) {
        is_available_to_parse(txContext.remarkSize);
        os_memmove(txContext.remark, txContext.bufferPointer, txContext.remarkSize);
        txContext.remark[txContext.remarkSize] = '\0';
        PRINTF("Remark:  %s\n", txContext.remark);
        transaction_offset_increase(txContext.remarkSize);
      }
      txContext.tx_parsing_state = BEGINNING;
      txContext.tx_parsing_group = TX_SPECIFIC;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void parse_group_coin_input() {

  if(txContext.tx_parsing_group != COIN_INPUT) {
    THROW(INVALID_STATE);
  }

  unsigned char amount[AMOUNT_LENGTH];

  do {
    switch(txContext.tx_parsing_state) {

      case BEGINNING:
        PRINTF("-- BEGINNING\n");
        // Read how many inputs
        txContext.remainingInputsOutputs = transaction_get_varint(); //throw if it can't.
        txContext.currentInputOutput = 0;
        if(txContext.remainingInputsOutputs > 0) {
          txContext.tx_parsing_state = COIN_OWNER_DATA_LENGTH; //Read input data
        } else {
          //No input (???), let's check outputs
          txContext.tx_parsing_group = COIN_OUTPUT;
          txContext.tx_parsing_state = BEGINNING;
          break; //exit from this switch
        }

      case COIN_OWNER_DATA_LENGTH:
        PRINTF("-- COIN_OWNER_DATA_LENGTH\n");
        PRINTF("remainingInputsOutputs: %d\n", txContext.remainingInputsOutputs);
        PRINTF("currentInputOutput: %d\n", txContext.currentInputOutput);
        txContext.currentInputOutputOwnerLength = transaction_get_varint();
        PRINTF("currentInputOutputOwnerLength: %d\n", txContext.currentInputOutputOwnerLength);
        txContext.tx_parsing_state = COIN_DATA;

      case COIN_DATA:
        PRINTF("-- COIN_DATA\n");
        //Check if we can parse whole input (owner + amount + locktime)
        is_available_to_parse(txContext.currentInputOutputOwnerLength + AMOUNT_LENGTH + LOCKTIME_LENGTH);
        //now we have whole input
        PRINTF("owner: %.*H\n", txContext.currentInputOutputOwnerLength, txContext.bufferPointer);
        transaction_offset_increase(txContext.currentInputOutputOwnerLength);
        //save amount
        nuls_swap_bytes(amount, txContext.bufferPointer, AMOUNT_LENGTH);
        PRINTF("amount: %.*H\n", 8, amount);

        if (transaction_amount_add_be(txContext.totalInputAmount, txContext.totalInputAmount, amount)) {
          // L_DEBUG_APP(("Input amount Overflow\n"));
          THROW(INVALID_PARAMETER);
        }
        transaction_offset_increase(AMOUNT_LENGTH);
        //locktime
        PRINTF("locktime\n");
        transaction_offset_increase(LOCKTIME_LENGTH);

        //update indexes
        txContext.remainingInputsOutputs--;
        txContext.currentInputOutput++;
        PRINTF("before if remainingInputsOutputs: %d\n", txContext.remainingInputsOutputs);
        PRINTF("before if currentInputOutput: %d\n", txContext.currentInputOutput);
        if(txContext.remainingInputsOutputs == 0) {
          txContext.tx_parsing_group = COIN_OUTPUT;
          txContext.tx_parsing_state = BEGINNING;
        } else {
          //Read another input
          txContext.tx_parsing_state = COIN_OWNER_DATA_LENGTH;
        }
        break;

      default:
        THROW(INVALID_STATE);
    }

    PRINTF("WHILE remainingInputsOutputs: %d\n", txContext.remainingInputsOutputs);
    PRINTF("WHILE currentInputOutput: %d\n", txContext.currentInputOutput);
  }
  while(txContext.remainingInputsOutputs != 0);

  PRINTF("-- OUT FROM COIN_INPUT\n");
}

void parse_group_coin_output() {

  if(txContext.tx_parsing_group != COIN_OUTPUT) {
    THROW(INVALID_STATE);
  }

  unsigned char address[ADDRESS_LENGTH];
  unsigned char amount[AMOUNT_LENGTH];

  do {
    switch(txContext.tx_parsing_state) {

      case BEGINNING:
        PRINTF("-- BEGINNING\n");
        // Read how many outputs
        txContext.remainingInputsOutputs = transaction_get_varint(); //throw if it can't.
        txContext.currentInputOutput = 0;

        if(txContext.remainingInputsOutputs > 0) {
          txContext.tx_parsing_state = COIN_OWNER_DATA_LENGTH; //Read input data
        } else {
          //No input (???), let's check outputs
          txContext.tx_parsing_group = COIN_OUTPUT;
          txContext.tx_parsing_state = BEGINNING;
          break; //exit from this switch
        }

      case COIN_OWNER_DATA_LENGTH:
        PRINTF("-- COIN_OWNER_DATA_LENGTH\n");
        PRINTF("-- remainingInputsOutputs: %d\n", txContext.remainingInputsOutputs);
        PRINTF("-- currentInputOutput: %d\n", txContext.currentInputOutput);
        txContext.currentInputOutputOwnerLength = transaction_get_varint();
        PRINTF("-- currentInputOutputOwnerLength: %d\n", txContext.currentInputOutputOwnerLength);
        PRINTF("-- ADDRESS_LENGTH: %d\n", ADDRESS_LENGTH);
        if(txContext.currentInputOutputOwnerLength != ADDRESS_LENGTH) {
          //TODO At the moment we support only transfer to address. rawScript is not implemented
          THROW(NOT_SUPPORTED);
        }
        txContext.tx_parsing_state = COIN_DATA;

      case COIN_DATA:
        PRINTF("-- COIN_DATA\n");
        //Check if we can parse whole input (owner + amount + locktime)
        is_available_to_parse(txContext.currentInputOutputOwnerLength + AMOUNT_LENGTH + LOCKTIME_LENGTH);
        //now we have whole output
        PRINTF("owner: %.*H\n", txContext.currentInputOutputOwnerLength, txContext.bufferPointer);
        //TODO we need to change it in a second moment
        //if currentInputOutput == 0 -> addressTo (temp solution)
        //if currentInputOutput == 1 -> change address (temp solution)
        os_memmove(address, txContext.bufferPointer, ADDRESS_LENGTH);
        PRINTF("Address:  %s\n", address);
        transaction_offset_increase(ADDRESS_LENGTH);
        nuls_swap_bytes(amount, txContext.bufferPointer, AMOUNT_LENGTH);
        PRINTF("amount: %.*H\n", 8, amount);
        transaction_offset_increase(AMOUNT_LENGTH);

        //If address is a P2SH, throw an error since it's not yet supported
        //TODO support P2SH
        if(address[2] != 0x01) { //P2PKH
          THROW(INVALID_PARAMETER);
        }

        if (transaction_amount_add_be(txContext.totalOutputAmount, txContext.totalOutputAmount, amount)) {
          // L_DEBUG_APP(("Output amount Overflow\n"));
          THROW(INVALID_PARAMETER);
        }

        if(txContext.currentInputOutput == 0) {
          //AddressTO
          os_memmove(txContext.outputAddress, address, ADDRESS_LENGTH);
          os_memmove(txContext.outputAmount, amount, AMOUNT_LENGTH);
        } else if (txContext.currentInputOutput == 1) {
          os_memmove(txContext.changeAddress, address, ADDRESS_LENGTH);
          os_memmove(txContext.changeAmount, amount, AMOUNT_LENGTH);
        } else {
          THROW(NOT_SUPPORTED);
        }

        //locktime
        PRINTF("locktime\n");
        transaction_offset_increase(LOCKTIME_LENGTH);

        //update indexes
        txContext.remainingInputsOutputs--;
        txContext.currentInputOutput++;
        PRINTF("before if remainingInputsOutputs: %d\n", txContext.remainingInputsOutputs);
        PRINTF("before if currentInputOutput: %d\n", txContext.currentInputOutput);
        if(txContext.remainingInputsOutputs == 0) {
          txContext.tx_parsing_group = CHECK_SANITY_BEFORE_SIGN;
          txContext.tx_parsing_state = BEGINNING;
        } else {
          //Read another output
          txContext.tx_parsing_state = COIN_OWNER_DATA_LENGTH;
        }
        break;

      default:
        THROW(INVALID_STATE);
    }

    PRINTF("WHILE remainingInputsOutputs: %d\n", txContext.remainingInputsOutputs);
    PRINTF("WHILE currentInputOutput: %d\n", txContext.currentInputOutput);
  }
  while(txContext.remainingInputsOutputs != 0);

  PRINTF("-- OUT FROM COIN_OUTPUT\n");
}

void check_sanity_before_sign() {
  PRINTF("check_sanity_before_sign\n");
  if(txContext.tx_parsing_group != CHECK_SANITY_BEFORE_SIGN) {
    THROW(INVALID_STATE);
  }

  PRINTF("txContext.bytesChunkRemaining: %d\n", txContext.bytesChunkRemaining);
  PRINTF("txContext.bytesRead: %d\n", txContext.bytesRead);
  PRINTF("txContext.totalTxBytes: %d\n", txContext.totalTxBytes);

  //Sanity checks about final parsing state
  if(txContext.bytesChunkRemaining != 0 || txContext.bytesRead != txContext.totalTxBytes) {
    THROW(INVALID_STATE);
  }

  txContext.tx_parsing_group = TX_PARSED;
  txContext.tx_parsing_state = READY_TO_SIGN;
}


// Parser Utils
void cx_hash_finalize(unsigned char *dest, unsigned char size) {
  unsigned char fake[1];
  unsigned char tmpHash[DIGEST_LENGTH];
  PRINTF("cx_hash_finalize\n");
  cx_sha256_t localHash;

  cx_hash(&txContext.txHash.header, CX_LAST, fake, 0, tmpHash, DIGEST_LENGTH);
  PRINTF("CX First Hash  %.*H\n", DIGEST_LENGTH, tmpHash);
  // Rehash
  cx_sha256_init(&localHash);
  cx_hash(&localHash.header, CX_LAST, tmpHash, DIGEST_LENGTH, dest, size);
  PRINTF("CX Second Hash  %.*H\n", size, dest);
}

void cx_hash_increase(unsigned char value) {
  cx_hash(&txContext.txHash.header, 0, txContext.bufferPointer, value, NULL, 0);
}

void transaction_offset_increase(unsigned char value) {
  cx_hash_increase(value);
  PRINTF("buffer processed: %.*H\n", value, txContext.bufferPointer);
  txContext.bytesRead += value;
  PRINTF("buffer bytesRead: %d\n", txContext.bytesRead);
  txContext.bufferPointer += value;
  txContext.bytesChunkRemaining -= value;
  PRINTF("buffer bytesChunkRemaining: %d\n", txContext.bytesChunkRemaining);
}

void is_available_to_parse(unsigned char x) {
  if(txContext.bytesChunkRemaining < x)
    THROW(NEED_NEXT_CHUNK);
}

unsigned long int transaction_get_varint(void) {
  unsigned char firstByte;
  is_available_to_parse(1);
  firstByte = *txContext.bufferPointer;
  if (firstByte < 0xFD) {
    transaction_offset_increase(1);
    return firstByte;
  } else if (firstByte == 0xFD) {
    unsigned long int result;
    transaction_offset_increase(1);
    is_available_to_parse(2);
    result = (unsigned long int)(*txContext.bufferPointer) |
             ((unsigned long int)(*(txContext.bufferPointer + 1)) << 8);
    transaction_offset_increase(2);
    return result;
  } else if (firstByte == 0xFE) {
    unsigned long int result;
    transaction_offset_increase(1);
    is_available_to_parse(4);
    result = nuls_read_u32(txContext.bufferPointer, 0, 0);
    transaction_offset_increase(4);
    return result;
  } else {
    // L_DEBUG_APP(("Varint parsing failed\n"));
    THROW(INVALID_PARAMETER);
  }
}

unsigned char transaction_amount_add_be(unsigned char *target, unsigned char *a, unsigned char *b) {
  unsigned char carry = 0;
  unsigned char i;
  for (i = 0; i < 8; i++) {
    unsigned short val = a[8 - 1 - i] + b[8 - 1 - i] + (carry ? 1 : 0);
    carry = (val > 255);
    target[8 - 1 - i] = (val & 255);
  }
  return carry;
}

unsigned char transaction_amount_sub_be(unsigned char *target, unsigned char *a, unsigned char *b) {
  unsigned char borrow = 0;
  unsigned char i;
  for (i = 0; i < 8; i++) {
    unsigned short tmpA = a[8 - 1 - i];
    unsigned short tmpB = b[8 - 1 - i];
    if (borrow) {
      if (tmpA <= tmpB) {
        tmpA += (255 + 1) - 1;
      } else {
        borrow = 0;
        tmpA--;
      }
    }
    if (tmpA < tmpB) {
      borrow = 1;
      tmpA += 255 + 1;
    }
    target[8 - 1 - i] = (unsigned char)(tmpA - tmpB);
  }
  return borrow;
}
