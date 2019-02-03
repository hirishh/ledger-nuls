
#include "approval.h"
#include "../ui_utils.h"
#include "../io.h"
#include "./nuls_utils.h"
#include "./ui_elements_s.h"
#include "os_io_seproxyhal.h"


/**
 * Sign with address
 */
static const bagl_element_t approval_nano_ui[] = {
  CLEAN_SCREEN,
  TITLE_ITEM("Sign with", 0x01),
  ICON_CHECK(0x00),
  ICON_CROSS(0x00),
  LINEBUFFER,
};

void touch_deny() {
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;

  // Kill private key - shouldn't be necessary but just in case.
  os_memset(&reqContext.privateKey, 0, sizeof(reqContext.privateKey));

  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
  // Display back the original UX
  ui_idle();
}

void touch_approve() {
  uint8_t signature[64];

  // Derive priv-pub again
  nuls_private_derive_keypair(reqContext.bip32path, reqContext.bip32pathLength,
                              &reqContext.privateKey, &reqContext.publicKey, reqContext.chainCode);

  nuls_signverify_finalhash(&reqContext.privateKey, 1, reqContext.digest, 32, signature, 64);

  //Paranoid
  os_memset(&reqContext.privateKey, 0, sizeof(reqContext.privateKey));

  initResponse();
  addToResponse(signature, 64);

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;

  unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
  G_io_apdu_buffer[tx]   = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

  // Display back the original UX
  ui_idle();
}


unsigned int approval_nano_ui_button(unsigned int button_mask, unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      touch_approve();
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny();
      break;
  }
  return 0;
}

void ui_approval() {
  os_memset(lineBuffer, 0, 50);
  os_memmove(lineBuffer, &reqContext.address, 32);
  lineBuffer[32] = '\0';
  UX_DISPLAY(approval_nano_ui, NULL)
}



