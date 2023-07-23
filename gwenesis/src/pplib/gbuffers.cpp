#include <Arduino.h>
#include "gbuffers.h"
#include "setup.h"

uint16_t glcdGetBufWidth(glcdBuffer_t* buf) {
  #if LCD_COLORDEPTH==16
  return buf[0];
  #elif LCD_COLORDEPTH==8
  uint16_t* tmp = (uint16_t*) &buf[0];
  return *tmp;
  #endif
}

uint16_t glcdGetBufHeight(glcdBuffer_t* buf) {
  #if LCD_COLORDEPTH==16
  return buf[1];
  #elif LCD_COLORDEPTH==8
  uint16_t* tmp = (uint16_t*) &buf[2];
  return *tmp;
  #endif
}

glcdBuffer_t* glcdGetBuffer(uint16_t width, uint16_t height) {
  #if LCD_COLORDEPTH==8
  // buffers need to even in 8 bit mode, because the buffer is transmitted
  // in 16 bit words. This gives approx. 7% speed improvement.
  if ((width * height) % 2)
    return NULL;
  #endif

  // CAVE: need to multiply BUF_HEADER_SIZE * sizeof(glcdColor_t) because the buffer ist later interpreted as an array of glcdColor_t
  glcdBuffer_t* buf = (glcdBuffer_t*) malloc(BUF_HEADER_SIZE * sizeof(glcdColor_t) + width * height * sizeof(glcdColor_t));
  if (!buf)
    return NULL;

  for (uint32_t i = 0; i < width * height; i++)
    buf[BUF_HEADER_SIZE + i] = 0;
  //memset((void*) (buf + BUF_HEADER_SIZE * sizeof(glcdCoord_t)), 255, width * height * bpp);

  // save width and height to buffer header
  #if LCD_COLORDEPTH==16
  buf[0] = width;
  buf[1] = height;
  #elif LCD_COLORDEPTH==8
  uint16_t* tmp = (uint16_t*) &buf[0];
  *tmp = width;
  tmp = (uint16_t*) &buf[2];
  *tmp = height;
  #endif
  
  return buf;
}

/*
 * Blits a buffer to another buffer
 * alpha must equal either BLIT_NO_ALPHA (not alpha channel) or the
 * color that shall be transparent
 */
void glcdBlitBuf(glcdCoord_t pos_x, glcdCoord_t pos_y, glcdBuffer_t* src_buf, glcdBuffer_t* dst_buf, int32_t alpha, bool pgm) {
  uint16_t srcBufWidth = glcdGetBufWidth(src_buf);
  uint16_t srcBufHeight = glcdGetBufHeight(src_buf);
  uint16_t dstBufWidth = glcdGetBufWidth(dst_buf);
  uint16_t dstBufHeight = glcdGetBufHeight(dst_buf);

  // out of buffer area
  if (pos_x >= dstBufWidth || (pos_x + srcBufWidth < 0) ||
      pos_y >= dstBufHeight || (pos_y + srcBufHeight < 0) )
    return;

  // check if the blitting area needs to be cropped
  uint16_t start_x = 0;
  uint16_t start_y = 0;
  uint16_t end_x = 0;
  uint16_t end_y = 0;

  uint32_t cpybuf_s = 0;

  if (pos_x < 0) {
    start_x = 0;
    cpybuf_s = -pos_x;
  } else {
    start_x = pos_x;
  }

  if (pos_y < 0) {
    start_y = 0;
    cpybuf_s += (-pos_y) * srcBufWidth;
  } else {
    start_y = pos_y;
  }

  if (pos_x + srcBufWidth > dstBufWidth)
    end_x = dstBufWidth;
  else
    end_x = pos_x + srcBufWidth;
    
  if (pos_y + srcBufHeight > dstBufHeight)
    end_y = dstBufHeight;
  else
    end_y = pos_y + srcBufHeight;

  // prepare start values
  uint32_t cpybuf_d = start_y * dstBufWidth + start_x;

  // copy the buffer
  if (alpha == BLIT_NO_ALPHA) {
    if (pgm == SRC_RAM) {
      for (glcdCoord_t y = start_y; y < end_y; y++) {
        for (glcdCoord_t x = start_x; x < end_x; x++) {
          dst_buf[cpybuf_d + BUF_HEADER_SIZE] = src_buf[cpybuf_s + BUF_HEADER_SIZE];
  
          cpybuf_s++;
          cpybuf_d++;
        }
        cpybuf_s += srcBufWidth - (end_x - start_x);
        cpybuf_d += dstBufWidth - (end_x - start_x);
      }
    } else {
      for (glcdCoord_t y = start_y; y < end_y; y++) {
        for (glcdCoord_t x = start_x; x < end_x; x++) {
          #if LCD_COLORDEPTH==8
            dst_buf[cpybuf_d + BUF_HEADER_SIZE] = pgm_read_byte((glcdBuffer_t*) &src_buf[cpybuf_s + BUF_HEADER_SIZE]);
          #elif LCD_COLORDEPTH==16
            dst_buf[cpybuf_d + BUF_HEADER_SIZE] = pgm_read_word((glcdBuffer_t*) &src_buf[cpybuf_s + BUF_HEADER_SIZE]);
          #endif
  
          cpybuf_s++;
          cpybuf_d++;
        }
        cpybuf_s += srcBufWidth - (end_x - start_x);
        cpybuf_d += dstBufWidth - (end_x - start_x);
      }
    }
  } else {
    if (pgm == SRC_RAM) {
      for (glcdCoord_t y = start_y; y < end_y; y++) {
        for (glcdCoord_t x = start_x; x < end_x; x++) {
          if (src_buf[cpybuf_s + BUF_HEADER_SIZE] != alpha)
            dst_buf[cpybuf_d + BUF_HEADER_SIZE] = src_buf[cpybuf_s + BUF_HEADER_SIZE];
  
          cpybuf_s++;
          cpybuf_d++;
        }
        cpybuf_s += srcBufWidth - (end_x - start_x);
        cpybuf_d += dstBufWidth - (end_x - start_x);
      }
    } else {
      for (glcdCoord_t y = start_y; y < end_y; y++) {
        for (glcdCoord_t x = start_x; x < end_x; x++) {
          #if LCD_COLORDEPTH==8
            glcdColor_t col = pgm_read_byte((glcdBuffer_t*) &src_buf[cpybuf_s + BUF_HEADER_SIZE]);
            if (col != alpha)
              dst_buf[cpybuf_d + BUF_HEADER_SIZE] = col;
          #elif LCD_COLORDEPTH==16
            glcdColor_t col = pgm_read_word((glcdBuffer_t*) &src_buf[cpybuf_s + BUF_HEADER_SIZE]);
            if (col != alpha)
              dst_buf[cpybuf_d + BUF_HEADER_SIZE] = col;
          #endif
  
          cpybuf_s++;
          cpybuf_d++;
        }
        cpybuf_s += srcBufWidth - (end_x - start_x);
        cpybuf_d += dstBufWidth - (end_x - start_x);
      }
    }
  }
  
}

void glcdFreeBuffer(glcdBuffer_t* buf) {
  free(buf);
}
