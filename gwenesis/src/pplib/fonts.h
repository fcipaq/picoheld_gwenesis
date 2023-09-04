#ifndef FONTS_H
#define FONTS_H

#include <Arduino.h>
#include "typedefs.h"

#define FONT_HEADER_FNT_WIDTH 2
#define FONT_HEADER_FNT_HEIGHT 3
#define FONT_HEADER_FRST_CHAR 5
#define FONT_HEADER_LST_CHAR 6
#define FONT_HEADER_CHAR_WIDTH 7

#define FONT_TRANSP true
#define FONT_BKG_COL false

glcdCoord_t getFontHeight(glcdFont_t* font);

void putChar(glcdCoord_t pos_x, glcdCoord_t pos_y, glcdColor_t c1, glcdColor_t c2, bool alpha, char c, glcdFont_t* font, glcdBuffer_t* buf);
void writeString(glcdCoord_t pos_x, glcdCoord_t pos_y, glcdColor_t c1, glcdColor_t c2, bool alpha, char* str, glcdFont_t* font, glcdBuffer_t* buf);
void font_write_string_centered(glcdCoord_t pos_x, glcdCoord_t y, glcdColor_t col, char* str, glcdFont_t* font, glcdBuffer_t* dst);

#endif //FONTS_H
