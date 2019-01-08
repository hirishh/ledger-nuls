#include <stdbool.h>
#include "os.h"
#include <inttypes.h>
#include "../io.h"
#ifndef STRUCT_TX
#define STRUCT_TX

typedef struct signContext_t {
    cx_ecfp_private_key_t privateKey;
    cx_ecfp_public_key_t publicKey;
    uint16_t signableContentLength;
    uint8_t reserved;

    // Holds digest to sign
    uint8_t digest[32];
} signContext_t;

extern signContext_t signContext;

#endif

uint32_t setSignContext(commPacket_t *packet);
