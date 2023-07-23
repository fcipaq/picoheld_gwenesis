#include <Arduino.h>
#include "graphics.h"
#include "gbuffers.h"

glcdPoint_t glcdPoint (glcdCoord_t x, glcdCoord_t y)
{
  glcdPoint_t p;

  p.x = x;
  p.y = y;

  return p;
}

void glcdSanitizeRect(glcdCoord_t *x1, glcdCoord_t *y1, glcdCoord_t *x2, glcdCoord_t *y2) {
  // range checking
  if (*x1 < 0)
    *x1 = 0;

  if (*x1 > (SCREEN_WIDTH - 1))
    *x1 = SCREEN_WIDTH - 1;

  if (*x2 < 0)
    *x2 = 0;

  if (*x2 > (SCREEN_WIDTH - 1))
    *x2 = SCREEN_WIDTH - 1;

  if (*y1 < 0)
    *y1 = 0;

  if (*y1 > (SCREEN_HEIGHT - 1))
    *y1 = SCREEN_HEIGHT - 1;

  if (*y2 < 0)
    *y2 = 0;

  if (*y2 > (SCREEN_HEIGHT - 1))
    *y2 = SCREEN_HEIGHT - 1;

  if (*x1 > *x2) {
    glcdCoord_t tmp;
    tmp = *x1;
    *x1 = *x2;
    *x2 = tmp;
  }

  if (*y1 > *y2) {
    glcdCoord_t tmp;
    tmp = *y1;
    *y1 = *y2;
    *y2 = tmp;
  }

}

void glcdFillRect(glcdCoord_t x1, glcdCoord_t y1, glcdCoord_t x2, glcdCoord_t y2, glcdColor_t color, glcdBuffer_t *fb)
{
  glcdSanitizeRect(&x1, &y1, &x2, &y2);

  uint16_t rect_width = x2 - x1 + 1;
  uint16_t rect_height = y2 - y1 + 1;

  // render to framebuffer
  // optimizer makes this loop equally fast as memcpy
  uint16_t fb_width = glcdGetBufWidth(fb);

  //jumpy  off by one?!
  for (uint16_t y = y1; y < (y2 + 1); y++)
    for (uint16_t x = x1; x < (x2 + 1); x++)
      fb[BUF_HEADER_SIZE + y * fb_width + x] = color;

}

/*============================ color calculations ============================*/

/*
 * Returns the color in 8 bit machine readable format when given
 * r g and b values in 8 bits. R-G-B 3-3-2
 * Ranges from r: 0..7, g: 0..7 and b: 0..3
 */
glcdColor8_t RGBColor332_332(uint8_t r, uint8_t g, uint8_t b)
{
  return (r * 64 + g * 8 + b);
}


/*
   Returns the color in 8 bit in machine readable format when given
   r g and b values in 24 bits. R-G-B 8-8-8
   So r ranges from 0..8, g: 0..8 and b: 0..3
*/
/*
glcdColor_t RGBColor888_332(uint8_t r, uint8_t g, uint8_t b)
{
  // NN
}
*/

/*
 * Returns the red component of a RGB565 color as a value ranging from 0..31
 */
uint8_t RGB565_red5(glcdColor16_t col) {
  return (col >> 11) & 0x1f;
}

/*
 * Returns the green component of a RGB565 color as a value ranging from 0..63
 */
uint8_t RGB565_green6(glcdColor16_t col) {
  return (col >> 5) & 0x3f;
}

/*
 * Returns the blue component of a RGB565 color as a value ranging from 0..31
 */
uint8_t RGB565_blue5(glcdColor16_t col) {
  return (col & 0x1f);
}

/*
   Returns the color in 16 bit machine readable format when given
   r g and b values in 16 bits. R-G-B 5-6-5
   So r ranges from 0..31, g: 0..63 and b: 0..31
*/
glcdColor16_t RGBColor565_565(uint8_t r, uint8_t g, uint8_t b)
{
  return (r << 11 | g << 5 | b);
}

/*
   Returns the color in 16 bit machine readable format when given
   r g and b values in 24 bits. R-G-B 8-8-8
   So r ranges from 0..255, g: 0..255 and b: 0..255
*/
glcdColor16_t RGBColor888_565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

glcdCoord_t glcdGetScreenWidth() {
  return SCREEN_WIDTH;
}

glcdCoord_t glcdGetScreenHeight() {
  return SCREEN_HEIGHT;
}
