#ifndef NULS_BASE58_H
#define NULS_BASE58_H

#include <stdlib.h>

int nuls_decode_base58(const char *in, size_t length,
                         unsigned char *out, size_t *outlen);

int nuls_encode_base58(const unsigned char *in, size_t length,
                         unsigned char *out, size_t *outlen);

#endif
