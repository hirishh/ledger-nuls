#include "getPubKey.h"
#include "../nuls_internals.h"


static const bagl_element_t verify_address_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Verify Address", 0x00),
  ICON_CROSS(0x00),
  ICON_CHECK(0x00),
  LINEBUFFER,
};

/**
 * Creates the response for the getPublicKey command.
 * It returns both publicKey and derived Address
 */
static void createPublicKeyResponse() {
  uint8_t prefixLen = 5;
  
  initResponse();

  //ChainCode
  addToResponse(reqContext.accountFrom.chainCode, 32);

  //PubKey
  addToResponse(reqContext.accountFrom.compressedPublicKey, 33);

  //Base58Address 
  os_memset(reqContext.accountFrom.addressWithPrefix, 0, 40);
  os_memmove(reqContext.accountFrom.addressWithPrefix, "NULSd", prefixLen);
  if (reqContext.accountFrom.chainId != 1) {
    prefixLen = 6;
    os_memmove(reqContext.accountFrom.addressWithPrefix, "tNULSd", prefixLen);
  }
  os_memmove(reqContext.accountFrom.addressWithPrefix + prefixLen, &reqContext.accountFrom.addressBase58[0], 32);
  addToResponse(reqContext.accountFrom.addressWithPrefix, 32 + prefixLen);
}

unsigned int verify_address_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      createPublicKeyResponse();

      unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
      G_io_apdu_buffer[tx]   = 0x90;
      G_io_apdu_buffer[tx+1] = 0x00;

      io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

      reset_contexts();

      // Display back the original UX
      ui_idle();
      break;
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny();
      break;
  }
  return 0;
}

static void ui_address(void) {
  os_memset(lineBuffer, 0, 50);
  os_memmove(lineBuffer, &reqContext.accountFrom.addressBase58, 32);
  lineBuffer[32] = '\0';
  UX_DISPLAY(verify_address_ui, NULL);
}

void handleGetPublicKey(volatile unsigned int *flags, commPacket_t *packet) {

  //reset contexts
  os_memset(&reqContext, 0, sizeof(reqContext));
  os_memset(&txContext, 0, sizeof(txContext));
  setReqContextForGetPubKey(packet); //address is derived there

  if (reqContext.showConfirmation == true) { // show address?
    // Show on ledger
    *flags |= IO_ASYNCH_REPLY;
    ui_address();
  } else {
    createPublicKeyResponse();
  }
}
