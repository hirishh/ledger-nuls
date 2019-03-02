#ifndef GETPUBKEY_H
#define GETPUBKEY_H

#include <inttypes.h>
#include <stdbool.h>
#include "../../io.h"

void handleGetPublicKey(volatile unsigned int *flags, commPacket_t *packet);

#endif
