#ifndef GBUFFERS_H
#define GBUFFERS_H

#include "typedefs.h"

#define BLIT_NO_ALPHA -1

glcdBuffer_t* glcdGetBuffer(uint16_t width, uint16_t height);

uint16_t glcdGetBufWidth(glcdBuffer_t* buf);
uint16_t glcdGetBufHeight(glcdBuffer_t* buf);

void glcdBlitBuf(glcdCoord_t pos_x, glcdCoord_t pos_y, glcdBuffer_t* src_buf, glcdBuffer_t* dst_buf, int32_t alpha, bool pgm);

#endif //GBUFFERS_H
