#ifndef NULS_HELPERS_H
#define NULS_HELPERS_H

#include <inttypes.h>
#include "os.h"

void nuls_swap_bytes(unsigned char *target, unsigned char *source, unsigned char size);
void nuls_write_u32_be(unsigned char *buffer, unsigned long int value);
void nuls_write_u32_le(unsigned char *buffer, unsigned long int value);
void nuls_write_u16_be(unsigned char *buffer, unsigned short int value);
void nuls_write_u16_le(unsigned char *buffer, unsigned short int value);
unsigned short int nuls_read_u16(unsigned char *buffer, unsigned char be, unsigned char skipSign);
unsigned long int nuls_read_u32(unsigned char *buffer, unsigned char be, unsigned char skipSign);
unsigned char nuls_secure_memcmp(void WIDE *buf1, void WIDE *buf2, unsigned short length);
unsigned char nuls_int_to_string(unsigned long int number, char *out);
unsigned char nuls_hex_amount_to_displayable(unsigned char *amount, char *dest);
unsigned char nuls_encode_varint(unsigned long int value, unsigned char *dest);
void nuls_double_to_displayable(double f, int ndigits, char *dest);


#endif