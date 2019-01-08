#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "os.h"
#include "cx.h"
#include "nulsutils.h"
#include "../io.h"

signContext_t signContext;

/**
 * Reads the packet, sets the signContext and patches the packet data values by skipping the header.
 * @param dataBuffer the  buffer to read from.
 * @return the amount of bytesRead
 */
uint32_t setSignContext(commPacket_t *packet) {
  //TODO
}
