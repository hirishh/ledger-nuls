#include "nuls_tx_parser.h"
#include "../../nuls_utils.h"
#include "../../nuls_helpers.h"
#include "../signTx.h"

void parse_group_common() {

  if(txContext.tx_parsing_group != COMMON) {
    THROW(INVALID_STATE);
  }

  uint64_t remarkVarInt;

  switch(txContext.tx_parsing_state) {

    case BEGINNING:
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
      //already parsed..
      is_available_to_parse(2);
      transaction_offset_increase(2);
      txContext.tx_parsing_state = FIELD_TIME;
      //no break is intentional

    case FIELD_TIME:
      is_available_to_parse(6);
      transaction_offset_increase(6);
      txContext.tx_parsing_state = FIELD_REMARK_LENGTH;
      //no break is intentional

    case FIELD_REMARK_LENGTH:
      //Size
      remarkVarInt = transaction_get_varint();
      if(remarkVarInt > REMARK_LENGTH) {
        THROW(INVALID_PARAMETER);
      }
      txContext.remarkSize = (unsigned char) remarkVarInt;
      txContext.tx_parsing_state = FIELD_REMARK;
      //no break is intentional

    case FIELD_REMARK:
      if (txContext.remarkSize != 0) {
        is_available_to_parse(txContext.remarkSize);
        nuls_swap_bytes(txContext.remark, txContext.bufferPointer, txContext.remarkSize);
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

  do {
    switch(txContext.tx_parsing_state) {

      case BEGINNING:
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
        txContext.currentInputOutputOwnerLength = transaction_get_varint();
        txContext.tx_parsing_state = COIN_DATA;

      case COIN_DATA:
        //Check if we can parse whole input (owner + amount + locktime)
        is_available_to_parse(txContext.currentInputOutputOwnerLength + AMOUNT_LENGTH + LOCKTIME_LENGTH);
        //now we have whole input
        transaction_offset_increase(txContext.currentInputOutputOwnerLength);
        //save amount
        unsigned char amount[AMOUNT_LENGTH];
        nuls_swap_bytes(amount, txContext.bufferPointer, AMOUNT_LENGTH);
        if (transaction_amount_add_be(txContext.totalInputAmount, txContext.totalInputAmount, amount)) {
          // L_DEBUG_APP(("Input amount Overflow\n"));
          THROW(INVALID_PARAMETER);
        }
        transaction_offset_increase(AMOUNT_LENGTH);
        //locktime
        transaction_offset_increase(LOCKTIME_LENGTH);

        //update indexes
        txContext.remainingInputsOutputs--;
        txContext.currentInputOutput++;
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
  }
  while(txContext.remainingInputsOutputs == 0);
}

void parse_group_coin_output() {

  if(txContext.tx_parsing_group != COIN_OUTPUT) {
    THROW(INVALID_STATE);
  }

  do {
    switch(txContext.tx_parsing_state) {

      case BEGINNING:
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
        txContext.currentInputOutputOwnerLength = transaction_get_varint();
        if(txContext.currentInputOutputOwnerLength != ADDRESS_LENGTH) {
          //TODO At the moment we support only transfer to address. rawScript is not implemented
          THROW(NOT_SUPPORTED);
        }
        txContext.tx_parsing_state = COIN_DATA;

      case COIN_DATA:
        //Check if we can parse whole input (owner + amount + locktime)
        is_available_to_parse(txContext.currentInputOutputOwnerLength + AMOUNT_LENGTH + LOCKTIME_LENGTH);
        //now we have whole output
        //TODO we need to change it in a second moment
        //if currentInputOutput == 0 -> addressTo (temp solution)
        //if currentInputOutput == 1 -> change address (temp solution)
        unsigned char address[ADDRESS_LENGTH];
        unsigned char amount[AMOUNT_LENGTH];
        nuls_swap_bytes(address, txContext.bufferPointer, ADDRESS_LENGTH);
        transaction_offset_increase(txContext.currentInputOutputOwnerLength);
        nuls_swap_bytes(amount, txContext.bufferPointer, AMOUNT_LENGTH);
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
        transaction_offset_increase(LOCKTIME_LENGTH);

        //update indexes
        txContext.remainingInputsOutputs--;
        txContext.currentInputOutput++;
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
  }
  while(txContext.remainingInputsOutputs == 0);
}

void check_sanity_before_sign() {

  if(txContext.tx_parsing_group != CHECK_SANITY_BEFORE_SIGN) {
    THROW(INVALID_STATE);
  }

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
  cx_sha256_t localHash;

  //L_DEBUG_BUF(("Finalize hash with\n", dataBuffer, sizeof(dataBuffer)));
  cx_hash(&txContext.txHash.header, CX_LAST, fake, 0, tmpHash, DIGEST_LENGTH);
  //L_DEBUG_BUF(("Hash1\n", hash1, sizeof(hash1)));

  // Rehash
  cx_sha256_init(&localHash);
  cx_hash(&localHash.header, CX_LAST, tmpHash, DIGEST_LENGTH, dest, size);
  //L_DEBUG_BUF(("Hash2\n", hash2, sizeof(hash2)));
}

void cx_hash_increase(unsigned char value) {
  cx_hash(&txContext.txHash.header, 0, txContext.bufferPointer, value, NULL, 0);
}

void transaction_offset_increase(unsigned char value) {
  cx_hash_increase(value);
  txContext.bufferPointer += value;
  txContext.bytesChunkRemaining -= value;
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
