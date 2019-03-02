#include "nuls_ram_variables.h"
#include "nuls_internals.h"

void nuls_tx_context_init() {
  cx_sha256_init(&txContext.txHash);
  os_memset(&txContext.digest, 0, sizeof(txContext.digest));
  txContext.type = 0;
  txContext.tx_parsing_state = BEGINNING;
  txContext.tx_parsing_group = COMMON;
  txContext.remainingInputsOutputs = 0;
  txContext.currentInputOutput = 0;
  txContext.currentInputOutputOwnerLength = 0;
  os_memset(&txContext.totalInputAmount, 0, sizeof(txContext.totalInputAmount));
  os_memset(&txContext.totalOutputAmount, 0, sizeof(txContext.totalOutputAmount));
  txContext.bytesRead = 0;
  txContext.bytesChunkRemaining = 0;
  txContext.bufferPointer = NULL;
  os_memset(&txContext.saveBufferForNextChunk, 0, sizeof(txContext.saveBufferForNextChunk));
  txContext.saveBufferLength = 0;
  txContext.totalTxBytes = 0;
  os_memset(&txContext.remark, 0, sizeof(txContext.remark));
  txContext.remarkSize = 0;
  os_memset(&txContext.fees, 0, sizeof(txContext.fees));
  os_memset(&txContext.outputAddress, 0, sizeof(txContext.outputAddress));
  os_memset(&txContext.outputAmount, 0, sizeof(txContext.outputAmount));
  os_memset(&txContext.changeAddress, 0, sizeof(txContext.changeAddress));
  os_memset(&txContext.changeAmount, 0, sizeof(txContext.changeAmount));
}

void nuls_local_address_init(local_address_t *localAddress) {
  os_memset(localAddress->address, 0, sizeof(localAddress->address));

}

void nuls_req_context_init() {
  reqContext.showConfirmation = 0;
  reqContext.signableContentLength = 0;

  os_memset(&reqContext.privateKey, 0, sizeof(reqContext.privateKey));
  os_memset(&reqContext.publicKey, 0, sizeof(reqContext.publicKey));

  os_memset(&reqContext.accountFrom.compressedPublicKey, 0, sizeof(reqContext.accountFrom.compressedPublicKey));
  os_memset(&reqContext.accountFrom.path, 0, sizeof(reqContext.accountFrom.path));
  reqContext.accountFrom.pathLength = 0;
  os_memset(&reqContext.accountFrom.chainCode, 0, sizeof(reqContext.accountFrom.chainCode));
  reqContext.accountFrom.chainId = 0;
  reqContext.accountFrom.type = 0;
  os_memset(&reqContext.accountFrom.address, 0, sizeof(reqContext.accountFrom.address));

  os_memset(&reqContext.accountChange.compressedPublicKey, 0, sizeof(reqContext.accountChange.compressedPublicKey));
  os_memset(&reqContext.accountChange.path, 0, sizeof(reqContext.accountChange.path));
  reqContext.accountChange.pathLength = 0;
  os_memset(&reqContext.accountChange.chainCode, 0, sizeof(reqContext.accountChange.chainCode));
  reqContext.accountChange.chainId = 0;
  reqContext.accountChange.type = 0;
  os_memset(&reqContext.accountChange.address, 0, sizeof(reqContext.accountChange.address));
}