#include "nuls_approval.h"
#include "nuls_internals.h"
#include "os_io_seproxyhal.h"


void touch_deny() {
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;

  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);

  reset_contexts();

  // Display back the original UX
  ui_idle();
}

void touch_approve() {
  uint8_t signature[100] = {0};

  // Derive priv-pub again
  nuls_private_derive_keypair(reqContext.accountFrom.path, reqContext.accountFrom.pathLength, reqContext.accountFrom.chainCode);

  unsigned short signatureSize = nuls_signverify_finalhash(&private_key, 1, txContext.digest, sizeof(txContext.digest), signature, sizeof(signature));

  initResponse();
  addToResponse(signature, signatureSize);

  unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
  G_io_apdu_buffer[tx]   = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

  reset_contexts();

  // Display back the original UX
  ui_idle();
}



