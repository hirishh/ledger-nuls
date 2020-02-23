#include "common_parser.h"
#include "../../nuls_internals.h"

/* TX Structure:
   *
   * COMMON
   * - type -> 2 Bytes
   * - time -> 4 Bytes
   *
   * */
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
      //no break is intentional
    case FIELD_TYPE:
      txContext.tx_parsing_state = FIELD_TYPE;
      //already parsed..
      is_available_to_parse(2);
      transaction_offset_increase(2);
      //no break is intentional
    case FIELD_TIME:
      txContext.tx_parsing_state = FIELD_TIME;
      is_available_to_parse(4);
      transaction_offset_increase(4);
      //no break is intentional
    case FIELD_REMARK_LENGTH:
      txContext.tx_parsing_state = FIELD_REMARK_LENGTH;
      remarkVarInt = transaction_get_varint();
      if(remarkVarInt > MAX_REMARK_LENGTH) {
        THROW(INVALID_PARAMETER);
      }
      txContext.remarkSize = (unsigned char) remarkVarInt;
      //no break is intentional
    case FIELD_REMARK:
      txContext.tx_parsing_state = FIELD_REMARK;
      if (txContext.remarkSize != 0) {
        is_available_to_parse(txContext.remarkSize);
        os_memmove(txContext.remark, txContext.bufferPointer, txContext.remarkSize);
        txContext.remark[txContext.remarkSize] = '\0';
        transaction_offset_increase(txContext.remarkSize);
      }
      txContext.tx_parsing_state = BEGINNING;
      txContext.tx_parsing_group = TX_SPECIFIC;
      break;

    default:
      THROW(INVALID_STATE);
  }
}

/* TX Structure:
   *
   * COIN_INPUT
   * - owner (hash + index)
   * - amount
   * - locktime
   * COIN_OUTPUT
   * - owner (address or script)
   * - amount
   * - locktime
   *
   * */

void parse_group_coin_input() {

  if(txContext.tx_parsing_group != COIN_INPUT) {
    THROW(INVALID_STATE);
  }

  unsigned char amount[AMOUNT_LENGTH];
  switch(txContext.tx_parsing_state) {

      case BEGINNING:
        // Read inputs and outputs length
        // TODO: to verify data length
        txContext.txCoinInputsOutputsLength = transaction_get_varint();

        // Read how many inputs
        txContext.remainingInputsOutputs = transaction_get_varint(); //throw if it can't.
        txContext.currentInputOutput = 0;
        // should be one coinfrom
        if(txContext.remainingInputsOutputs != 1) {
          THROW(INVALID_STATE);
        }

      case COIN_DATA:
        txContext.tx_parsing_state = COIN_DATA;

        // coinfrom address
        uint32_t addrLen = transaction_get_varint();
        if(addrLen > ADDRESS_LENGTH) {
          THROW(INVALID_STATE);
        }
        is_available_to_parse(addrLen);
        os_memmove(txContext.inputAddress, txContext.bufferPointer, addrLen);
        transaction_offset_increase(addrLen);

        // coinfrom chainid
        is_available_to_parse(2);
        txContext.inputChainId = nuls_read_u16(txContext.bufferPointer, 0, 0);
        transaction_offset_increase(2);

        //coinfrom assetid
        is_available_to_parse(2);
        txContext.inputAssetId = nuls_read_u16(txContext.bufferPointer, 0, 0);
        transaction_offset_increase(2);

        //coinfrom amount
        is_available_to_parse(AMOUNT_LENGTH);
        nuls_swap_bytes(txContext.totalInputAmount, txContext.bufferPointer, AMOUNT_LENGTH);
        transaction_offset_increase(AMOUNT_LENGTH);

        //coinfrom nonce
        uint32_t nonceLen = transaction_get_varint();
        if(nonceLen > MAX_NONCE_LENGTH) {
          THROW(INVALID_STATE);
        }  
        is_available_to_parse(MAX_NONCE_LENGTH);
        txContext.inputNonceSize = nonceLen;
        os_memmove(txContext.inputNonce, txContext.bufferPointer, txContext.inputNonceSize);
        transaction_offset_increase(nonceLen);

        //coinfrom locked
        is_available_to_parse(1);
        txContext.inputLocked = *txContext.bufferPointer;
        transaction_offset_increase(1);

        //update indexes
        txContext.remainingInputsOutputs--;
        txContext.currentInputOutput++;
        txContext.tx_parsing_group = COIN_OUTPUT;
        txContext.tx_parsing_state = BEGINNING;
        break;

      default:
        THROW(INVALID_STATE);
    }
#if 0
  do {
    switch(txContext.tx_parsing_state) {

      case BEGINNING:
        // Read inputs and outputs length
        txContext.txCoinInputsOutputsLength = transaction_get_varint();


        // Read how many inputs
        txContext.remainingInputsOutputs = transaction_get_varint(); //throw if it can't.
        txContext.currentInputOutput = 0;
        // only one coin from 
        if(txContext.remainingInputsOutputs != 1) {
          THROW(INVALID_STATE);
        }

        if(txContext.remainingInputsOutputs == 0) {
          //No input (???), let's check outputs
          txContext.tx_parsing_group = COIN_OUTPUT;
          txContext.tx_parsing_state = BEGINNING;
          break; //exit from this switch
        }

      /*
      case COIN_OWNER_DATA_LENGTH:
        txContext.tx_parsing_state = COIN_OWNER_DATA_LENGTH;
        txContext.currentInputOutputOwnerLength = transaction_get_varint();
      */
      case COIN_DATA:
        txContext.tx_parsing_state = COIN_DATA;
        //Check if we can parse whole input (owner + amount + locktime)
        //is_available_to_parse(txContext.currentInputOutputOwnerLength + AMOUNT_LENGTH + LOCKTIME_LENGTH);
        //now we have whole input
        //transaction_offset_increase(txContext.currentInputOutputOwnerLength);
        //save amount
        nuls_swap_bytes(amount, txContext.bufferPointer, AMOUNT_LENGTH);

        if (transaction_amount_add_be(txContext.totalInputAmount, txContext.totalInputAmount, amount)) {
          THROW(EXCEPTION_OVERFLOW);
        }
        transaction_offset_increase(AMOUNT_LENGTH);
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
  while(txContext.remainingInputsOutputs != 0);
#endif
}

void parse_group_coin_output() {

  if(txContext.tx_parsing_group != COIN_OUTPUT) {
    THROW(INVALID_STATE);
  }

  switch(txContext.tx_parsing_state) {

      case BEGINNING:
        // Read how many outputs
        txContext.remainingInputsOutputs = transaction_get_varint(); //throw if it can't.

        // should be one cointo
        if(txContext.remainingInputsOutputs != 1) {
          THROW(INVALID_STATE);
        }

        txContext.currentInputOutput = 0;
        txContext.changeFound = false;
        txContext.nOut = 1;

      case COIN_DATA:
        txContext.tx_parsing_state = COIN_DATA;

        // cointo address
        uint32_t addrLen = transaction_get_varint();
        if(addrLen > ADDRESS_LENGTH) {
          THROW(INVALID_STATE);
        }
        is_available_to_parse(addrLen);
        os_memmove(&txContext.outputAddress[0], txContext.bufferPointer, addrLen);
        transaction_offset_increase(addrLen);

        // cointo chainid
        is_available_to_parse(2);
        txContext.outputChainId = nuls_read_u16(txContext.bufferPointer, 0, 0);
        transaction_offset_increase(2);

        //cointo assetid
        is_available_to_parse(2);
        txContext.outputAssetId = nuls_read_u16(txContext.bufferPointer, 0, 0);
        transaction_offset_increase(2);

        //cointo amount
        is_available_to_parse(AMOUNT_LENGTH);
        nuls_swap_bytes(txContext.totalOutputAmount, txContext.bufferPointer, AMOUNT_LENGTH);
        nuls_swap_bytes(&txContext.outputAmount[0], txContext.bufferPointer, AMOUNT_LENGTH);
        nuls_swap_bytes(txContext.changeAmount, txContext.bufferPointer, AMOUNT_LENGTH);
        transaction_offset_increase(AMOUNT_LENGTH);

        //cointo locktime
        is_available_to_parse(LOCKTIME_LENGTH);
        txContext.outputLockTime = nuls_read_u64(txContext.bufferPointer, 0, 0);
        transaction_offset_increase(LOCKTIME_LENGTH);

        //Do check about changeAddress
        //If is a change address -> remove from "txContext.outputAddress" and do "only-one" check
        if(reqContext.accountChange.pathLength > 0) { // -> user specified accountChange in input
            if(nuls_secure_memcmp(txContext.outputAddress[txContext.nOut-1], reqContext.accountChange.address, ADDRESS_LENGTH) == 0) {
              txContext.changeFound = true;
            }
            else {
              THROW(EXCEPTION_OVERFLOW);
            }
        }

        //update indexes
        txContext.remainingInputsOutputs--;
        txContext.currentInputOutput++;
        txContext.tx_parsing_group = CHECK_SANITY_BEFORE_SIGN;
        txContext.tx_parsing_state = BEGINNING;
        break;

      default:
        THROW(INVALID_STATE);
    }



#if 0
  bool isOpReturnOutput = false;

  do {
    switch(txContext.tx_parsing_state) {

      case BEGINNING:
        // Read how many outputs
        txContext.remainingInputsOutputs = transaction_get_varint(); //throw if it can't.

        if(txContext.remainingInputsOutputs > MAX_OUTPUT_TO_CHECK) {
          THROW(INVALID_PARAMETER);
        }

        txContext.currentInputOutput = 0;
        txContext.changeFound = false;
        txContext.nOut = 0;
        if(txContext.remainingInputsOutputs == 0) {
          //No output (???), let's check outputs
          txContext.tx_parsing_group = CHECK_SANITY_BEFORE_SIGN;
          txContext.tx_parsing_state = BEGINNING;
          break; //exit from this switch
        }

      case COIN_OWNER_DATA_LENGTH:
        txContext.tx_parsing_state = COIN_OWNER_DATA_LENGTH;
        isOpReturnOutput = false;
        txContext.currentInputOutputOwnerLength = transaction_get_varint();

      case COIN_DATA:
        txContext.tx_parsing_state = COIN_DATA;
        //Check if we can parse whole input (owner + amount + locktime)
        is_available_to_parse(txContext.currentInputOutputOwnerLength + AMOUNT_LENGTH + LOCKTIME_LENGTH);

        //Check if is an op_return script
        if(is_op_return_script(txContext.bufferPointer)) {
            isOpReturnOutput = true;
        } else {
            get_address_from_owner(
                    txContext.bufferPointer,
                    txContext.currentInputOutputOwnerLength,
                    txContext.outputAddress[txContext.nOut]);
        }

        //Owner
        transaction_offset_increase(txContext.currentInputOutputOwnerLength);
        //Amount
        nuls_swap_bytes(txContext.outputAmount[txContext.nOut], txContext.bufferPointer, AMOUNT_LENGTH);
        transaction_offset_increase(AMOUNT_LENGTH);
        //Locktime
        transaction_offset_increase(LOCKTIME_LENGTH);

        //Add to totalOutputAmount
        if(transaction_amount_add_be(txContext.totalOutputAmount, txContext.totalOutputAmount, txContext.outputAmount[txContext.nOut])) {
          THROW(EXCEPTION_OVERFLOW);
        }

        if(!isOpReturnOutput) {
            txContext.nOut++;

            //Do check about changeAddress
            //If is a change address -> remove from "txContext.outputAddress" and do "only-one" check
            if(reqContext.accountChange.pathLength > 0) { // -> user specified accountChange in input

                if(nuls_secure_memcmp(txContext.outputAddress[txContext.nOut-1], reqContext.accountChange.address, ADDRESS_LENGTH) == 0) {
                    txContext.changeFound = true;
                    //Add to changeAmount
                    if (transaction_amount_add_be(txContext.changeAmount, txContext.changeAmount, txContext.outputAmount[txContext.nOut-1])) {
                        THROW(EXCEPTION_OVERFLOW);
                    }
                    //Remove from "toShow"
                    txContext.nOut--;
                }
            }
        }

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
  while(txContext.remainingInputsOutputs != 0);
#endif
  //Calculate fees (input - output)
  if (transaction_amount_sub_be(txContext.fees, txContext.totalInputAmount, txContext.totalOutputAmount)) {
    THROW(EXCEPTION_OVERFLOW);
  }

  //Throw if change account is provided but change not found in output
  if(reqContext.accountChange.pathLength > 0 && !txContext.changeFound) {
    THROW(INVALID_PARAMETER);
  }
}

void check_sanity_before_sign() {
  if(txContext.tx_parsing_group != CHECK_SANITY_BEFORE_SIGN) {
    THROW(INVALID_STATE);
  }

  //Sanity checks about final parsing state
  if(txContext.bytesChunkRemaining != 0 || txContext.bytesRead != txContext.totalTxBytes) {
    THROW(INVALID_STATE);
  }

  if(reqContext.accountChange.pathLength > 0 && !txContext.changeFound) {
    THROW(INVALID_PARAMETER);
  }

  txContext.tx_parsing_group = TX_PARSED;
  txContext.tx_parsing_state = READY_TO_SIGN;
}


// Parser Utils
void cx_hash_finalize(unsigned char *dest, unsigned char size) {
  unsigned char fake[1];
  unsigned char tmpHash[DIGEST_LENGTH];
  cx_sha256_t localHash;

  cx_hash(&txContext.txHash.header, CX_LAST, fake, 0, tmpHash, DIGEST_LENGTH);
  // Rehash
  cx_sha256_init(&localHash);
  cx_hash(&localHash.header, CX_LAST, tmpHash, DIGEST_LENGTH, dest, size);
}

void cx_hash_increase(unsigned char value) {
  cx_hash(&txContext.txHash.header, 0, txContext.bufferPointer, value, NULL, 0);
}

void transaction_offset_increase(unsigned char value) {
  cx_hash_increase(value);
  txContext.bytesRead += value;
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

void get_address_from_owner(unsigned char *owner, unsigned long int ownerLength, unsigned char *address_out) {
    if (ownerLength == ADDRESS_LENGTH) {
        os_memmove(address_out, owner, ADDRESS_LENGTH);
        //If address is not a P2PKH, throw an error since it's not supported
        if(!is_p2pkh_addr(txContext.outputAddress[txContext.nOut][2]) && !is_contract_tx(txContext.type)) {
            THROW(INVALID_PARAMETER);
        }
    } else if (is_send_to_address_script(owner))
        os_memmove(address_out, owner + 3, ADDRESS_LENGTH);
    else if (is_send_to_p2sh_script(owner))
        os_memmove(address_out, owner + 2, ADDRESS_LENGTH);
    else
        THROW(NOT_SUPPORTED);
}

unsigned char transaction_amount_add_be(unsigned char *target, unsigned char *a, unsigned char *b) {
  unsigned char carry = 0;
  unsigned char i;
  for (i = 0; i < 32; i++) {
    unsigned short val = a[32 - 1 - i] + b[32 - 1 - i] + (carry ? 1 : 0);
    carry = (val > 255);
    target[32 - 1 - i] = (val & 255);
  }
  return carry;
}

unsigned char transaction_amount_sub_be(unsigned char *target, unsigned char *a, unsigned char *b) {
  unsigned char borrow = 0;
  unsigned char i;
  for (i = 0; i < 32; i++) {
    unsigned short tmpA = a[32 - 1 - i];
    unsigned short tmpB = b[32 - 1 - i];
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
    target[32 - 1 - i] = (unsigned char)(tmpA - tmpB);
  }
  return borrow;
}
