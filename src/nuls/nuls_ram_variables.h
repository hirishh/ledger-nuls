#ifndef NULS_RAM_VARIABLES_H
#define NULS_RAM_VARIABLES_H

#include "nuls_context.h"

extern reqContext_t reqContext;
extern transaction_context_t txContext;

void nuls_tx_context_init();
void nuls_req_context_init();

#endif
