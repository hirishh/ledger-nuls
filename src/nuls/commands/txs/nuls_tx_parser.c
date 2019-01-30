#include "../../nuls_utils.h"
#include "../../nuls_helpers.h"
#include "../signTx.h"

#define HASH_LENGTH 34
#define ADDRESS_LENGTH 23
#define AMOUNT_LENGTH 8
#define LOCKTIME_LENGTH 6

void parse_group_common() {

  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      // Reset transaction state
      txContext.transactionRemainingInputsOutputs = 0;
      txContext.transactionCurrentInputOutput = 0;
      os_memset(
              txContext.totalInputAmount,
              0, sizeof(txContext.totalInputAmount));
      os_memset(
              txContext.totalOutputAmount,
              0, sizeof(txContext.totalOutputAmount));
      //Done, start with the next field
      txContext.tx_parsing_state = FIELD_TYPE;
      //no break is intentional

    case FIELD_TYPE:
      //already parsed..
      is_available_to_parse(2);
      uint16_t
      tmpType = nuls_re
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
      txContext.remarkSize = transaction_get_varint();
      txContext.tx_parsing_state = FIELD_REMARK;
      //no break is intentional

    case FIELD_REMARK:
      if (txContext.remarkSize != 0) {
        is_available_to_parse(txContext.remarkSize)
        os_memmove(txContext.remark, txContext.buffer, txContext.remarkSize)
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

  do {
    switch(txContext.tx_parsing_state) {

      case BEGINNING:
        // Read how many inputs
        txContext.remainingInputsOutputs = transaction_get_varint(); //throw if it can't.
        txContext.currentInputOutput = 0;
        if(txContext.remainingInputsOutputs > 0) {
          txContext.tx_parsing_state = INPUT_OWNER_DATA_LENGTH; //Read input data
        } else {
          //No input (???), let's check outputs
          txContext.tx_parsing_group = COIN_OUTPUT;
          txContext.tx_parsing_state = BEGINNING;
          break; //exit from this switch
        }

      case COIN_INPUT_OWNER_DATA_LENGTH:
        txContext.currentInputOutputOwnerLength = transaction_get_varint();
        txContext.tx_parsing_state = COIN_INPUT_DATA;

      case COIN_INPUT_DATA:
        //Check if we can parse whole input (owner + amount + locktime)
        is_available_to_parse(txContext.currentInputOutputOwnerLength + AMOUNT_LENGTH + LOCKTIME_LENGTH)
        //now we have whole input
        transaction_offset_increase(txContext.currentInputOutputOwnerLength);
        //save amount
        unsigned char amount[AMOUNT_LENGTH];
        nuls_swap_bytes(amount, txContext.buffer, AMOUNT_LENGTH);
        if (transaction_amount_add_be(txContext.totalInputAmount, txContext.totalInputAmount, amount)) {
          L_DEBUG_APP(("Input amount Overflow\n"));
          THROW(INVALID_PARAMETER);
        }
        transaction_offset_increase(AMOUNT_LENGTH);
        //locktime
        //TODO Do checks on locktime? maybe not.
        transaction_offset_increase(LOCKTIME_LENGTH)

        //update indexes
        txContext.remainingInputsOutputs--;
        txContext.currentInputOutput++;
        if(txContext.remainingInputsOutputs == 0) {
          txContext.tx_parsing_group = COIN_OUTPUT;
          txContext.tx_parsing_state = BEGINNING;
        } else {
          //Read another input
          txContext.tx_parsing_state = COIN_INPUT_OWNER_DATA_LENGTH;
        }
        break;

      default:
        THROW(INVALID_STATE)
    }
  }
  while(txContext.remainingInputsOutputs == 0);
}

void parse_group_coin_output() {

}

void check_sanity_before_sign() {

  //TODO sanity checks about buffer status
  txContext.tx_parsing_state = READY_TO_SIGN;
}


// Parser Utils

void transaction_offset_increase(unsigned char value) {
  txContext.buffer += value;
  txContext.bytesChunkRemaining -= value;
}

void is_available_to_parse(unsigned char x) {
  if(txContext.bytesChunkRemaining >= x)
    return true;
  else
    THROW(NEED_NEXT_CHUNK);
}

unsigned long int transaction_get_varint(void) {
  unsigned char firstByte;
  is_available_to_parse(1);
  firstByte = *txContext.buffer;
  if (firstByte < 0xFD) {
    transaction_offset_increase(1);
    return firstByte;
  } else if (firstByte == 0xFD) {
    unsigned long int result;
    transaction_offset_increase(1);
    is_available_to_parse(2);
    result = (unsigned long int)(*txContext.buffer) |
             ((unsigned long int)(*(txContext.buffer + 1)) << 8);
    transaction_offset_increase(2);
    return result;
  } else if (firstByte == 0xFE) {
    unsigned long int result;
    transaction_offset_increase(1);
    is_available_to_parse(4);
    result = nuls_read_u32(txContext.buffer, 0, 0);
    transaction_offset_increase(4);
    return result;
  } else {
    L_DEBUG_APP(("Varint parsing failed\n"));
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
