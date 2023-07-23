#include <Arduino.h>
#include "buttons.h"

#include "pico/bootrom.h"

uint16_t x_0 = 480;
uint16_t y_0 = 480;
uint16_t max_x = 876;
uint16_t min_x = 173;
uint16_t max_y = 837;
uint16_t min_y = 145;

uint16_t checkDPad() {
  uint16_t cur_x = analogRead(PIN_ANALOG_X);
  uint16_t cur_y = analogRead(PIN_ANALOG_Y);

  uint16_t value = 0;

  if (cur_x > max_x)
    max_x = cur_x;

  if (cur_y > max_y)
    max_y = cur_y;

  if (cur_x < min_x)
    min_x = cur_x;

  if (cur_y < min_y)
    min_y = cur_y; 


  if (cur_x > 650)
    value |= DPAD_LEFT;

  if (cur_x < 300)
    value |= DPAD_RIGHT;

  if (cur_y < 300)
    value |= DPAD_UP;

  if (cur_y > 650)
    value |= DPAD_DOWN;

  return value;
}

uint16_t checkButtons() {
  uint16_t value = 0;

  if (digitalRead(PIN_BUTTON_1) == BUTTON_PRESSED)
    value |= BUTTON_1;

  if (digitalRead(PIN_BUTTON_2) == BUTTON_PRESSED)
    value |= BUTTON_2;

  if (digitalRead(PIN_BUTTON_3) == BUTTON_PRESSED)
    value |= BUTTON_3;

  #ifdef PIN_BUTTON_4
    if (digitalRead(PIN_BUTTON_4) == BUTTON_PRESSED)
      value |= BUTTON_4;
  #endif

  return value;
}

void checkBootsel() {
  // Enter bootloader in case all buttons are pressed during system reset
  if ((digitalRead(PIN_BUTTON_1) != BUTTON_PRESSED) ||
      (digitalRead(PIN_BUTTON_2) != BUTTON_PRESSED) ||
      (digitalRead(PIN_BUTTON_3) != BUTTON_PRESSED))
    return;

  reset_usb_boot(0, 0);
}

void initButtons() {
  pinMode(PIN_ANALOG_X, INPUT);
  pinMode(PIN_ANALOG_Y, INPUT);

  pinMode(PIN_BUTTON_1, BUTTON_PULL_MODE);
  pinMode(PIN_BUTTON_2, BUTTON_PULL_MODE);
  pinMode(PIN_BUTTON_3, BUTTON_PULL_MODE);
  #ifdef PIN_BUTTON_4
  pinMode(PIN_BUTTON_4, BUTTON_PULL_MODE);
  #endif

  checkBootsel();
}
