#include "common_parser.h"
#include "10_data.h"
#include "../signTx.h"
#include "../../nuls_internals.h"

static const bagl_element_t ui_10_data_nano[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Push Data From", 0x01),
  TITLE_ITEM("Data Hash", 0x02),
  TITLE_ITEM("Remark", 0x03),
  TITLE_ITEM("Fees", 0x04),
  ICON_ARROW_RIGHT(0x01),
  ICON_ARROW_RIGHT(0x02),
  ICON_ARROW_RIGHT(0x03),
  ICON_CHECK(0x04),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

static uint8_t stepProcessor_10_data(uint8_t step) {
  uint8_t nextStep = step + 1;
  if(step == 2 && txContext.remarkSize == 0) {
    nextStep++; // no remark
  }
  return nextStep;
}

static tx_type_specific_10_data_t *cc = &(txContext.tx_fields.data);

static void uiProcessor_10_data(uint8_t step) {
  unsigned short amountTextSize;
  os_memset(lineBuffer, 0, 50);
  switch (step) {
    case 1:
      //Push Data From
      os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, BASE58_ADDRESS_LENGTH);
      lineBuffer[BASE58_ADDRESS_LENGTH] = '\0';
      break;
    case 2:
      //Data Hash
      //snprintf(lineBuffer, 50, "%.*X", cc->digest);
      //os_memmove(lineBuffer + 46, "...\0", 4);
      snprintf(lineBuffer, 50, "%.*H...%.*H",
              4, cc->digest,
              4, cc->digest + DIGEST_LENGTH - 4);
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

void tx_parse_specific_10_data() {

  /* TX Structure:
   *
   * TX_SPECIFIC
   * - len:databytes
   *
   * */

  uint64_t tmpVarInt = 0;

  //NB: There are no break in this switch. This is intentional.
  switch(txContext.tx_parsing_state) {

    case BEGINNING:
      //init sha256
      cx_sha256_init(&cc->hash);

    case _10_DATA_TXHASH_LENGTH:
      txContext.tx_parsing_state = _10_DATA_TXHASH_LENGTH;
      cc->size = transaction_get_varint();
      cc->sizeMissing = cc->size;

    case _10_DATA_TXHASH_DATA:
      txContext.tx_parsing_state = _10_DATA_TXHASH_DATA;
      tmpVarInt = MIN(cc->sizeMissing, txContext.bytesChunkRemaining);
      cx_hash(&cc->hash.header, 0, txContext.bufferPointer, tmpVarInt, NULL, 0);
      cc->sizeMissing -= tmpVarInt;
      transaction_offset_increase(tmpVarInt);

      //Check if we need next chunk
      if(txContext.bytesChunkRemaining == 0 && cc->sizeMissing != 0) {
        THROW(NEED_NEXT_CHUNK);
      }

      if(cc->sizeMissing == 0) {
        //let's finalize the hash
        unsigned char fake[1];
        cx_hash(&cc->hash.header, CX_LAST, fake, 0, cc->digest, DIGEST_LENGTH);

        //It's time for CoinData
        txContext.tx_parsing_group = COIN_INPUT;
        txContext.tx_parsing_state = BEGINNING;
      }
      break;

    default:
      THROW(INVALID_STATE);
  }
}

void tx_finalize_10_data() {
  //Throw if:
  // - changeAddress is not provided
  if(reqContext.accountChange.pathLength == 0) {
    THROW(INVALID_PARAMETER);
  }

  ux.elements = ui_10_data_nano;
  ux.elements_count = 11;
  totalSteps = 4;
  step_processor = stepProcessor_10_data;
  ui_processor = uiProcessor_10_data;
}
