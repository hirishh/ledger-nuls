#include "nuls_helpers.h"
#include "nuls_internals.h"

void nuls_write_u32_be(unsigned char *buffer, unsigned long int value) {
  buffer[0] = ((value >> 24) & 0xff);
  buffer[1] = ((value >> 16) & 0xff);
  buffer[2] = ((value >> 8) & 0xff);
  buffer[3] = (value & 0xff);
}

void nuls_write_u32_le(unsigned char *buffer, unsigned long int value) {
  buffer[0] = (value & 0xff);
  buffer[1] = ((value >> 8) & 0xff);
  buffer[2] = ((value >> 16) & 0xff);
  buffer[3] = ((value >> 24) & 0xff);
}

void nuls_write_u16_be(unsigned char *buffer, unsigned short int value) {
  buffer[0] = ((value >> 8) & 0xff);
  buffer[1] = (value & 0xff);
}

void nuls_write_u16_le(unsigned char *buffer, unsigned short int value) {
  buffer[0] = (value & 0xff);
  buffer[1] = ((value >> 8) & 0xff);
}


unsigned short int nuls_read_u16(unsigned char *buffer, unsigned char be, unsigned char skipSign) {
  unsigned char i;
  unsigned short int result = 0;
  unsigned char shiftValue = (be ? 8 : 0);
  for (i = 0; i < 2; i++) {
    unsigned char x = (unsigned char)buffer[i];
    if ((i == 0) && skipSign) {
      x &= 0x7f;
    }
    result += ((unsigned short int)x) << shiftValue;
    if (be) {
      shiftValue -= 8;
    } else {
      shiftValue += 8;
    }
  }
  return result;
}

unsigned long int nuls_read_u32(unsigned char *buffer, unsigned char be, unsigned char skipSign) {
  unsigned char i;
  unsigned long int result = 0;
  unsigned char shiftValue = (be ? 24 : 0);
  for (i = 0; i < 4; i++) {
    unsigned char x = (unsigned char)buffer[i];
    if ((i == 0) && skipSign) {
      x &= 0x7f;
    }
    result += ((unsigned long int)x) << shiftValue;
    if (be) {
      shiftValue -= 8;
    } else {
      shiftValue += 8;
    }
  }
  return result;
}

void nuls_swap_bytes(unsigned char *target, unsigned char *source, unsigned char size) {
  unsigned char i;
  for (i = 0; i < size; i++) {
    target[i] = source[size - 1 - i];
  }
}