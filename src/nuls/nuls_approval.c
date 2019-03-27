#include "nuls_approval.h"
#include "nuls_internals.h"
#include "os_io_seproxyhal.h"


void touch_deny() {
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;

  // Kill private key - shouldn't be necessary but just in case.
  os_memset(&private_key, 0, sizeof(private_key));

  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
  // Display back the original UX
  ui_idle();
}

void touch_approve() {
  PRINTF("-- touch_approve\n");
  uint8_t signature[100] = {0};
  PRINTF("-- A\n");
  // Derive priv-pub again
  nuls_private_derive_keypair(reqContext.accountFrom.path, reqContext.accountFrom.pathLength, reqContext.accountFrom.chainCode);
  PRINTF("-- B\n");
  unsigned short signatureSize = nuls_signverify_finalhash(&private_key, 1, txContext.digest, sizeof(txContext.digest), signature, sizeof(signature));
  PRINTF("-- C. signatureSize %d\n", signatureSize);
  PRINTF("-- C. signature %.*H\n", signatureSize, signature);

  //Paranoid
  os_memset(&private_key, 0, sizeof(private_key));

  //unsigned char pd[3] = {0x00, 0x01, 0x02};

  PRINTF("-- D\n");
  initResponse();
  PRINTF("-- E\n");
  addToResponse(signature, signatureSize);
  //addToResponse(pd, 3);
  PRINTF("-- F\n");

  unsigned int tx = flushResponseToIO(G_io_apdu_buffer);
  G_io_apdu_buffer[tx]   = 0x90;
  G_io_apdu_buffer[tx+1] = 0x00;
  PRINTF("-- G\n");

  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx+2);

  //reset contexts
  os_memset(&reqContext, 0, sizeof(reqContext));
  os_memset(&txContext, 0, sizeof(txContext));
  os_memset(&commContext, 0, sizeof(commContext));
  os_memset(&commPacket, 0, sizeof(commPacket));

  PRINTF("-- H\n");

  // Allow restart of operation
  commContext.started = false;
  commContext.read = 0;
  PRINTF("-- I\n");

  // Display back the original UX
  ui_idle();
}



