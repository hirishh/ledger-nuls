#ifndef NULS_HELPERS_H
#define NULS_HELPERS_H

void nuls_write_u32_be(unsigned char *buffer, unsigned long int value);
void nuls_write_u32_le(unsigned char *buffer, unsigned long int value);
void nuls_write_u16_be(unsigned char *buffer, unsigned short int value);
void nuls_write_u16_le(unsigned char *buffer, unsigned short int value);
unsigned short int nuls_read_u16(unsigned char *buffer, unsigned char be, unsigned char skipSign);
unsigned long int nuls_read_u32(unsigned char *buffer, unsigned char be, unsigned char skipSign);
void nuls_swap_bytes(unsigned char *target, unsigned char *source, unsigned char size);
unsigned char nuls_int_to_string(unsigned long int number, char *out);

#endif