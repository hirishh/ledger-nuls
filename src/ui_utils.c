#include "ui_utils.h"

/**
 * Line buffer (50chars)
 */
char lineBuffer[100];

unsigned int currentStep = 0;
unsigned int totalSteps = 0;

/**
 * Used to verify what is going to be displayed
 * @param element
 * @return 0 or 1
 */
unsigned int uiprocessor(const bagl_element_t *element) {
  if (element->component.userid == 0x0) {
    return 1;
  }
  if ((element->component.type & (~BAGL_FLAG_TOUCHABLE)) == BAGL_NONE) {
    return 0;
  }
  if (element->component.userid == currentStep) {
    return 1;
  }
  return 0;
}