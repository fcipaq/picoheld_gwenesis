// TODO: rename all defines to "LCD_"
// TODO: vertical tiling für fullscreen buffer --> damage in buffer header

#ifndef LCDCOM_H
#define LCDCOM_H

#include <Arduino.h>
#include "typedefs.h"

/* ---------------------- screen operation modes ----------------------*/
#if defined LCD_DOUBLE_PIXEL_LINEAR || defined LCD_DOUBLE_PIXEL_NEAREST
  #if LCD_ROTATION==0 || LCD_ROTATION==2
    #define SCREEN_WIDTH   (PHYS_SCREEN_WIDTH / 2)
    #define SCREEN_HEIGHT  (PHYS_SCREEN_HEIGHT / 2)
  #elif LCD_ROTATION==1 || LCD_ROTATION==3
    #define SCREEN_WIDTH   (PHYS_SCREEN_HEIGHT / 2)
    #define SCREEN_HEIGHT  (PHYS_SCREEN_WIDTH / 2)
  #endif
#else
  #if LCD_ROTATION==0 || LCD_ROTATION==2
    #define SCREEN_WIDTH   PHYS_SCREEN_WIDTH
    #define SCREEN_HEIGHT  PHYS_SCREEN_HEIGHT
  #elif LCD_ROTATION==1 || LCD_ROTATION==3
    #define SCREEN_WIDTH   PHYS_SCREEN_HEIGHT
    #define SCREEN_HEIGHT  PHYS_SCREEN_WIDTH
  #endif
#endif

/* ---------------------- function declarations ----------------------*/

void glcdInitGraphics();

void glcdDisplayOn(void);             // switch display on, booster on, sleep out
void glcdDisplayOff(void);            // switch display off, booster off, sleep in

void glcdBacklight(byte level);

void dmaWait(void);
void pioWait(void);
int glcdIsDmaDone();

void glcdSetAddr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
 
void glcdSendBufferWord(uint16_t *buf, uint32_t buffersize);

void glcdSendDatByte(uint8_t cmd);
void glcdSendCmdByte(uint8_t cmd);
void set_rs(byte value);
void set_rst(byte value);

#if LCD_COLORDEPTH==8
void glcdDefaultClut();
void glcdCustomClut(uint8_t* cclut);
void glcdSetClutColor(uint8_t c, uint8_t r, uint8_t g, uint8_t b);
#endif

#endif //LCD_COM_H
