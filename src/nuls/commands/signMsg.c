#include "signMsg.h"
#include "../nuls_internals.h"
#include "cx.h"
#include "os.h"
#include "../../io.h"
#include "../../ui_utils.h"
#include "../nuls_approval.h"
#include "txs/common_parser.h"

#define SIGNED_MESSAGE_PREFIX "\x18NULS Signed Message:\n"

const bagl_element_t sign_message_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify text", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER
};

unsigned int sign_message_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      touch_approve();
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny(NULL);
      break;
  }
  return 0;
}


void handleSignMessagePacket(commPacket_t *packet, commContext_t *context) {

  uint32_t headerBytesRead = 0;

  // if first packet with signing header
  if ( packet->first ) {
    PRINTF("SIGN MSG - First Packet\n");
    // Reset sha256 and context
    os_memset(&reqContext, 0, sizeof(reqContext));
    os_memset(&txContext, 0, sizeof(txContext));
    cx_sha256_init(&txContext.txHash);
    txContext.bufferPointer = NULL;

    // IMPORTANT this logic below only works if the first packet contains the needed information (Which it should)
    // Set signing context by parsing bip32paths and other info. function returns number of bytes read (not part of TX)
    headerBytesRead = setReqContextForSign(packet);
    txContext.totalTxBytes = reqContext.signableContentLength;

    // Signing header.
    uint8_t varint[9] = {0};
    uint64_t prefixLength = strlen(SIGNED_MESSAGE_PREFIX);
    uint8_t varintLength = nuls_encode_varint(prefixLength, varint);

    cx_hash(&txContext.txHash.header, 0, varint, varintLength, NULL, 0);
    cx_hash(&txContext.txHash.header, 0, SIGNED_MESSAGE_PREFIX, prefixLength, NULL, 0);

    os_memset(varint, 0, sizeof(varint));
    varintLength = nuls_encode_varint(reqContext.signableContentLength, varint);
    cx_hash(&txContext.txHash.header, 0, varint, varintLength, NULL, 0);
    PRINTF("headerBytesRead: %d\n", headerBytesRead);

    //Prepare LineBuffer to show
    os_memset(lineBuffer, 0, sizeof(lineBuffer));
    uint8_t msgDisplayLenth = MIN(50, packet->length - headerBytesRead);
    PRINTF("msgDisplayLenth: %d\n", msgDisplayLenth);
    os_memmove(lineBuffer, packet->data + headerBytesRead, msgDisplayLenth);
    PRINTF("1 linebuffer: %s\n", lineBuffer);

    if (msgDisplayLenth > 46) {
      os_memmove(lineBuffer+46, "...\0", 4);
    }
    PRINTF("2 linebuffer: %s\n", lineBuffer);

    uint8_t npc = 0; //Non Printable Chars Counter
    for (uint8_t i=0; i < msgDisplayLenth; i++) {
      npc += IS_PRINTABLE(lineBuffer[i]) ?
             0 /* Printable Char */:
             1 /* Non Printable Char */;
    }
    PRINTF("npc: %d\n", npc);

    // We rewrite the line buffer to <binary data> in case >= than 40% is non printable or first char is not printable.
    if ((npc*100) / msgDisplayLenth >= 40 || ! IS_PRINTABLE(lineBuffer[0])) {
      os_memmove(lineBuffer, "< binary data >\0", 16);
    }

  }

  cx_hash(&txContext.txHash.header, 0, packet->data + headerBytesRead , packet->length - headerBytesRead, NULL, 0);
}


void processSignMessage(volatile unsigned int *flags) {
  // Close sha256 and hash again
  cx_hash_finalize(txContext.digest, DIGEST_LENGTH);

  // Init user flow.
  *flags |= IO_ASYNCH_REPLY;
  UX_DISPLAY(sign_message_ui, NULL);
}
