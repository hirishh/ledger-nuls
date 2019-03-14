#include <string.h>
#include "nuls_internals.h"
#include "../ui_utils.h"
#include "os_io_seproxyhal.h"
#include "../glyphs.h"

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

/**
 * Sets ui to idle.
 */
void ui_idle() {
  UX_MENU_DISPLAY(0, menu_main, NULL);
}
