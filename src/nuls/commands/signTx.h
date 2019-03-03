#ifndef SIGNTX_H
#define SIGNTX_H


#include "../nuls_internals.h"
#include "../../ui_utils.h"
#include "../../io.h"

typedef void (*ui_processor_fn)(uint8_t curStep);
typedef uint8_t (*step_processor_fn)(uint8_t curStep);
extern step_processor_fn step_processor;
extern ui_processor_fn ui_processor;

void handleSignTxPacket(commPacket_t *packet, commContext_t *context);
void finalizeSignTx(volatile unsigned int *flags);

#endif
