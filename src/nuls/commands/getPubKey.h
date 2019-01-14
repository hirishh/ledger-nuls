#include <inttypes.h>
#include <stdbool.h>
#include "../../io.h"

#ifndef LEDGER_NANO2_GETPUBKEY_H
#define LEDGER_NANO2_GETPUBKEY_H

void handleGetPublicKey(volatile unsigned int *flags, commPacket_t *packet);

#endif //LEDGER_NANO2_GETPUBKEY_H
