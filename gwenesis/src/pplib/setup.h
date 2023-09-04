#ifndef SETUP_H
#define SETUP_H

// ================================== Pico Held pin assignment ====================================================
/*----------------- buttons assignment --------------- */
#define BUTTON_PULL_MODE INPUT_PULLDOWN
#define BUTTON_PRESSED HIGH

#define PIN_ANALOG_X  26
#define PIN_ANALOG_Y  27

#define PIN_BUTTON_1  21
#define PIN_BUTTON_2  22
#define PIN_BUTTON_3  20

/* ------------------------ Power management pins ------------------------*/
#define BAT_PIN_ADC   29
//#define BAT_PIN_SRC   24

/* ------------------------ Sound assignment ------------------------*/
#define SND_PIN       0 
//LED_BUILTIN

/* ------------------------ LCD pin assignment ------------------------*/
#define LCD_PIN_BL_PWM 14

#define LCD_PIN_TE    2    // tearing pin
#define LCD_PIN_DC    3    // data/command control pin
#define LCD_PIN_WR    4    // clock
#define LCD_PIN_RST   5    // reset pin

// Note: the data pins are PIO hardware configured and
// driven and need to be in consecutive order
#define LCD_PIN_D0    6     // 1st of the 8 data pins

// SD pins
#define PIN_SD_MISO       16
#define PIN_SD_MOSI       19
#define PIN_SD_SCK        18
#define PIN_SD_CS         17

/* ------------------------ screen orientatin ------------------------*/
// 0 = 0° (portrait), 1 = 90° (landscape), 2 = 180° (portrait), 3 = 270° (landscape)
#define LCD_ROTATION 1

/* ---------------------- color depth ----------------------*/
// currently supported are 8 bits and 16 bits of colors
#define LCD_COLORDEPTH 16

/* ----------------- pixel doubling / panel fitting -----------------*/
// please choose 1 or 0 fitters
//#define LCD_DOUBLE_PIXEL_LINEAR // only working in 16 bit mode
//#define LCD_DOUBLE_PIXEL_NEAREST

/* ---------------------- TFT driver ----------------------*/
#include "lcd_drv/ili9341_defines.h"
//#include "lcd_drv/ili9488_defines.h"

/* ---------------------- TFT driver ----------------------*/
// needs to be called AFTER setup
#include "lcdcom.h"

#endif // SETUP_H
