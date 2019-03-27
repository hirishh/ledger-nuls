#ifndef SIGNMSG_H
#define SIGNMSG_H


#include "../nuls_internals.h"
#include "../../ui_utils.h"
#include "../../io.h"

void handleSignMessagePacket(commPacket_t *packet, commContext_t *context);
void processSignMessage(volatile unsigned int *flags);

#endif
