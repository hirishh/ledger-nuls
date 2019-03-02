#include <string.h>
#include "os_io_seproxyhal.h"
#include "nuls_internals.h"
#include "../glyphs.h"
#include "../ui_utils.h"

#define SCRATCH_SIZE 21

const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];
ux_state_t ux;

#define __NAME2(a, b) a##b
#define NAME2(a, b) __NAME2(a, b)

const ux_menu_entry_t menu_main[] = {
  {NULL, NULL, 0, &NAME2(C_badge, ) , "Use wallet to", "view accounts", 33, 12},
  {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t menu_about[] = {
  {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
  {NULL, NULL, 0, NULL, "Developer", "hirish", 0, 0},
  {menu_main, NULL, 2, &C_icon_back, "Back", NULL, 61, 40},
  UX_MENU_END
};

unsigned char nuls_hex_amount_to_displayable(unsigned char *amount, char *dest) {
  unsigned char LOOP1 = 13;
  unsigned char LOOP2 = 8;
  unsigned short scratch[SCRATCH_SIZE];
  unsigned char offset = 0;
  unsigned char nonZero = 0;
  unsigned char i;
  unsigned char targetOffset = 0;
  unsigned char workOffset;
  unsigned char j;
  unsigned char nscratch = SCRATCH_SIZE;
  unsigned char smin = nscratch - 2;
  unsigned char comma = 0;

  for (i = 0; i < SCRATCH_SIZE; i++) {
    scratch[i] = 0;
  }
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      unsigned char k;
      unsigned short shifted_in =
              (((amount[i] & 0xff) & ((1 << (7 - j)))) != 0) ? (short)1
                                                             : (short)0;
      for (k = smin; k < nscratch; k++) {
        scratch[k] += ((scratch[k] >= 5) ? 3 : 0);
      }
      if (scratch[smin] >= 8) {
        smin -= 1;
      }
      for (k = smin; k < nscratch - 1; k++) {
        scratch[k] =
                ((scratch[k] << 1) & 0xF) | ((scratch[k + 1] >= 8) ? 1 : 0);
      }
      scratch[nscratch - 1] = ((scratch[nscratch - 1] << 1) & 0x0F) |
                              (shifted_in == 1 ? 1 : 0);
    }
  }

  for (i = 0; i < LOOP1; i++) {
    if (!nonZero && (scratch[offset] == 0)) {
      offset++;
    } else {
      nonZero = 1;
      dest[targetOffset++] = scratch[offset++] + '0';
    }
  }
  if (targetOffset == 0) {
    dest[targetOffset++] = '0';
  }
  workOffset = offset;
  for (i = 0; i < LOOP2; i++) {
    unsigned char allZero = 1;
    unsigned char j;
    for (j = i; j < LOOP2; j++) {
      if (scratch[workOffset + j] != 0) {
        allZero = 0;
        break;
      }
    }
    if (allZero) {
      break;
    }
    if (!comma) {
      dest[targetOffset++] = '.';
      comma = 1;
    }
    dest[targetOffset++] = scratch[offset++] + '0';
  }
  return targetOffset;
}

/**
 * Sets ui to idle.
 */
void ui_idle() {
  UX_MENU_DISPLAY(0, menu_main, NULL);
}
