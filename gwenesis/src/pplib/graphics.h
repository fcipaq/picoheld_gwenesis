#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Arduino.h>
#include "setup.h"
#include "typedefs.h"
#include "gbuffers.h"

#define GLCD_SUCCESS        0
#define GLCD_ERROR         -1

/* ========================== function definitions ========================= */

glcdPoint_t glcdPoint(glcdCoord_t x, glcdCoord_t y);

// Drawing primitives
void glcdFillRect(glcdCoord_t x1, glcdCoord_t y1, glcdCoord_t x2, glcdCoord_t y2, glcdColor_t color, glcdBuffer_t *fb);

// Color
glcdColor16_t RGBColor565_565(uint8_t r, uint8_t g, uint8_t b);
glcdColor16_t RGBColor888_565(uint8_t r, uint8_t g, uint8_t b);
uint8_t RGB565_red5(glcdColor16_t col);
uint8_t RGB565_green6(glcdColor16_t col);
uint8_t RGB565_blue5(glcdColor16_t col);
glcdColor8_t RGBColor332_332(uint8_t r, uint8_t g, uint8_t b);

glcdCoord_t glcdGetScreenWidth(void);
glcdCoord_t glcdGetScreenHeight(void);

#define glcdSwapCoord(x, y) {glcdCoord_t temporary_swap_coordinate = x; x = y; y = temporary_swap_coordinate;}
#define glcdCheckCoord(x, y) {if (x > y) {glcdSwapCoord(x, y)}}

void glcdShowDisplayBuffer(glcdBuffer_t*  buf);
void glcdBacklight(byte level);

#endif // #ifndef GRAPHICS_H
