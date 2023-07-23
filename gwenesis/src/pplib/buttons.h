#include "setup.h"

/*----------------- software assignment --------------- */
#define DPAD_NONE     0
#define DPAD_UP       1
#define DPAD_LEFT     2
#define DPAD_RIGHT    4
#define DPAD_DOWN     8

#define BUTTON_NONE   0
#define BUTTON_1      1
#define BUTTON_2      2
#define BUTTON_3      4
#define BUTTON_4      8

#define BUTTON_RED    BUTTON_1
#define BUTTON_BLUE   BUTTON_2
#define BUTTON_GREEN  BUTTON_3
#define BUTTON_YELLOW BUTTON_4

/*----------------- exposed funtions ------------------ */
void initButtons();
uint16_t checkDPad();
uint16_t checkButtons();
