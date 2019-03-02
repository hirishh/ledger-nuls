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


