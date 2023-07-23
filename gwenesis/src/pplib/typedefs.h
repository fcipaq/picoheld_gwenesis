#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <Arduino.h>
#include "setup.h"

/* ====================== defines ====================== */
#define SRC_RAM 0
#define SRC_PGM 1

/* ====================== type definitions ====================== */
typedef uint16_t glcdColor16_t;
typedef uint8_t glcdColor8_t;

#if LCD_COLORDEPTH==16
  // 16 bit / 65k colors
  #define BUF_HEADER_SIZE   2
  typedef glcdColor16_t glcdColor_t;
#elif LCD_COLORDEPTH==8
  // 8 bit / 256 colors
  #define BUF_HEADER_SIZE   4
  typedef glcdColor8_t glcdColor_t;
#endif

typedef glcdColor_t glcdBuffer_t;

typedef int glcdCoord_t;

typedef struct 
{
    glcdCoord_t x;
    glcdCoord_t y;
} glcdPoint_t;

typedef struct
{
    glcdCoord_t x1;
    glcdCoord_t y1;
    glcdCoord_t x2;
    glcdCoord_t y2;
} glcdRect_t;

/*----------------- fonts -----------------------*/
typedef uint8_t glcdFont_t;

#endif //#ifndef TYPEDEFS_H
