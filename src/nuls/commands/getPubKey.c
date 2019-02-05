//
// Created by andrea on 08/12/18.
//

#include "getPubKey.h"

#include "../secp256k1.h"
#include "../nuls_utils.h"
#include "../ui_elements_s.h"
#include "../../ui_utils.h"
#include "../../io.h"
#include "../approval.h"
#include "os.h"


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
  initResponse();

  //ChainCode
  addToResponse(reqContext.chainCode, 32);

  //PubKey
  addToResponse(reqContext.compressedPublicKey, 33);

  //Base58Address
  addToResponse(reqContext.address, 32);
}

unsigned int verify_address_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      createPublicKeyResponse();

      unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
      G_io_apdu_buffer[tx]   = 0x90;
      G_io_apdu_buffer[tx+1] = 0x00;

      io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

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
  os_memmove(lineBuffer, &reqContext.address, 32);
  lineBuffer[32] = '\0';
  UX_DISPLAY(verify_address_ui, NULL);
}

void handleGetPublicKey(volatile unsigned int *flags, commPacket_t *packet) {
  setReqContextForGetPubKey(packet); //address is derived during extractBip32Data()

  if (reqContext.showConfirmation == true) { // show address?
    // Show on ledger
    *flags |= IO_ASYNCH_REPLY;
    ui_address();
  } else {
    createPublicKeyResponse();
  }
}
