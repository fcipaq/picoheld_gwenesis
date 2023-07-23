#include "fonts.h"
#include "graphics.h"
#include "gbuffers.h"

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

uint32_t plus = 0;

glcdCoord_t getFontFrstChar(glcdFont_t* font) {
  return pgm_read_byte((uint8_t*) &font[FONT_HEADER_FRST_CHAR]);
}

glcdCoord_t getFontLstChar(glcdFont_t* font) {
  return pgm_read_byte((uint8_t*) &font[FONT_HEADER_LST_CHAR]);
}

glcdCoord_t getFontHeight(glcdFont_t* font) {
  return pgm_read_byte((uint8_t*) &font[FONT_HEADER_FNT_HEIGHT]);
}

glcdCoord_t getFontWidth(glcdFont_t* font) {
  return pgm_read_byte((uint8_t*) &font[FONT_HEADER_FNT_WIDTH]);
}

glcdCoord_t getFontCharWidth(char c, glcdFont_t* font) {
  if (c < getFontFrstChar(font) || c > getFontLstChar(font))
    return -1;

  c -= getFontFrstChar(font);
  return pgm_read_byte((uint8_t*) &font[FONT_HEADER_CHAR_WIDTH + c]);
}

uint16_t getFontDataOfs(glcdFont_t* font) {
  return FONT_HEADER_CHAR_WIDTH + getFontLstChar(font) - getFontFrstChar(font) + 1;
}

void putChar(glcdCoord_t pos_x, glcdCoord_t pos_y, glcdColor_t c1, glcdColor_t c2, bool alpha, char c, glcdFont_t* font, glcdBuffer_t* buf) {
  glcdCoord_t charHeight = getFontHeight(font);
  glcdCoord_t charWidth = getFontCharWidth(c, font);

  glcdCoord_t bufWidth = glcdGetBufWidth(buf);
  glcdCoord_t bufHeight = glcdGetBufHeight(buf);

  if (c < getFontFrstChar(font) || c > getFontLstChar(font))
    return;
    
  uint8_t charnum = c - getFontFrstChar(font);

  uint32_t total_width = 0;
  
  for (char i = getFontFrstChar(font); i < c; i++)
      total_width += getFontCharWidth(i, font);

  uint32_t total_bits = (total_width * charHeight);

  uint16_t data_ofs = getFontDataOfs(font) + total_bits / 8;

  uint8_t bitcnt = (uint32_t) total_bits % 8;
  uint8_t datcnt = 0;

  char fnt_dat = pgm_read_byte((uint8_t*) &font[data_ofs]);

   for (uint16_t x = 0; x < charWidth; x++)
    for (uint16_t y = 0; y < charHeight; y++)
    {
      if (bitcnt == 8) {
        bitcnt = 0;
        datcnt++;
        fnt_dat = pgm_read_byte((uint8_t*) &font[data_ofs + datcnt]);
      }
            
      if ((pos_y + y) >= 0 && (pos_y + y) < bufHeight &&
          (pos_x + x) >= 0 && (pos_x + x) < bufWidth) {
        if (fnt_dat & (1 << bitcnt)) {
          buf[BUF_HEADER_SIZE + (pos_y + y) * bufWidth + (pos_x + x)] = c1;
        } else {
          if (!alpha)
            buf[BUF_HEADER_SIZE + (pos_y + y) * bufWidth + (pos_x + x)] = c2;
        }
      }

      bitcnt++;
    }
  
}

void writeString(glcdCoord_t pos_x, glcdCoord_t pos_y, glcdColor_t c1, glcdColor_t c2, bool alpha, char* str, glcdFont_t* font, glcdBuffer_t* buf) {
  int h = 0;
  glcdCoord_t ofs_x = 0;
  
  while (str[h] != 0) {
    putChar(pos_x + ofs_x, pos_y, c1, c2, alpha, str[h], font, buf);
    ofs_x += getFontCharWidth(str[h], font) + 1;
    h++;
  }

}
