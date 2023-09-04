// TODO: tetragon functions need to be safe if points are out of framebuffer

#include "lcdcom.h"
#include "gbuffers.h"
#include "setup.h"
#include "graphics.h"

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "hardware/dma.h"
#include "../rp2040_pwm/rp2040_pwm.h" // PWM lib for background
#include "lcd_pio.pio.h"

//#include <mutex>

/* ----------------------------- NN -----------------------------*/
#if LCD_COLORDEPTH==8
uint16_t* clut8 = (uint16_t*) malloc(256 * sizeof(uint16_t));
#endif

/* ----------------------- backlight -----------------------------*/
RP2040_PWM* backlightpwm = new RP2040_PWM(LCD_PIN_BL_PWM, 100000, 5.0f);

/* ----------------------- scanout -----------------------------*/
volatile glcdBuffer_t* _in_flip = NULL;

volatile uint16_t dmaScanline = 0;
volatile uint8_t scanlineUneven = 0;

#if (LCD_COLORDEPTH==8 && !defined LCD_DOUBLE_PIXEL_LINEAR) || defined LCD_DOUBLE_PIXEL_NEAREST
uint16_t scanlineBuf[SCREEN_WIDTH * 2];
#elif defined LCD_DOUBLE_PIXEL_LINEAR && LCD_COLORDEPTH==16
glcdBuffer_t* scanlineBufLst = (glcdBuffer_t*) malloc(SCREEN_WIDTH * 2 * sizeof(glcdBuffer_t));
glcdBuffer_t* scanlineBufNxt = (glcdBuffer_t*) malloc(SCREEN_WIDTH * 2 * sizeof(glcdBuffer_t));
glcdBuffer_t* scanlineBufMid = (glcdBuffer_t*) malloc(SCREEN_WIDTH * 2 * sizeof(glcdBuffer_t));
#elif defined LCD_DOUBLE_PIXEL_LINEAR && LCD_COLORDEPTH==8
uint16_t* scanlineBufLst = (uint16_t*) malloc(SCREEN_WIDTH * 2 * sizeof(uint16_t));
uint16_t* scanlineBufNxt = (uint16_t*) malloc(SCREEN_WIDTH * 2 * sizeof(uint16_t));
uint16_t* scanlineBufMid = (uint16_t*) malloc(SCREEN_WIDTH * 2 * sizeof(uint16_t));
#else
// full resolution, full color scanout
uint16_t scanlineBuf0[SCREEN_WIDTH];
uint16_t scanlineBuf1[SCREEN_WIDTH];
#endif

/* ----------------------------- PIO -----------------------------*/
// Community RP2040 board package by Earle Philhower
PIO lcd_pio = pio0;
int8_t pio_sm = 0;

uint32_t program_offset  = 0;

uint32_t pull_stall_mask = 0;

uint32_t pio_instr_jmp8  = 0;

/* ----------------------------- DMA -----------------------------*/
int32_t  dma_tx_channel;
dma_channel_config dma_tx_config;

byte  DMA_Enabled = 0;

/* ----------------------------- lcd driver -----------------------------*/
#ifdef ILI9341_DRV_H
#include "lcd_drv/ili9341_init.h"
#elif defined ILI9488_DRV_H
#include "lcd_drv/ili9488_init.h"
#else
#error NO LCD DRIVER SPECIFIED
#endif

#if defined LCD_PIN_INVERSION
/*
 *  Swap bit order (from LSB to MSB). 
 *  First n bits of source byte are swapped.
 *  The remaining bits are set to 0.
 */

int swapBitOrderN(int v, int n) {
  int sv = 0;
  for (int h = 0; h < n; h++) {
    sv |= (v & 1);
    if (h != (n - 1))
      sv = sv << 1;
    v = v >> 1;
  }
  
  return sv;
}
#endif

void defaultClut()
{
 #if defined LCD_PIN_INVERSION
  // The data lines from the CPU to the display do cross on the board
  // therefore the bit order is swapped.
  // Compensation for command bytes and data bytes is done in the 
  // corresponding functions.
  // However the bit order cannot be swapped for picture content for
  // performance reason. Therefore the LUT is modified to undo the
  // bit order swap caused by inverse wiring.
  glcdSendCmdByte(ILI9341_COLOR_SET); 
  
  for (int r = 0; r < 32; r ++) {
    glcdSendDatByte(swapBitOrderN(r, 6));
  }
  
  for (int g = 0; g < 64; g ++) {
    glcdSendDatByte(swapBitOrderN(g, 6));
  }
  
  for (int b = 0; b < 32; b ++) {
    glcdSendDatByte(swapBitOrderN(b, 6));
  }
  
  glcdSendCmdByte(TFT_RAMWR); //memory write
 #endif
 
 #if LCD_COLORDEPTH==8
  // calculate standard color palette of 256 colors
  int rr, gg, bb;
  int i = 0;

  for (int r = 0; r < 8; r ++)
    for (int g = 0; g < 8; g ++)
      for (int b = 0; b < 4; b ++)
      {
        rr = r * 5;
        if (rr > 31) rr = 31;

        gg = g * 9;

        bb = b * 11;
        if (bb > 31) bb = 31;
        
        //clut8[i] = RGBColor565_565(rr, gg, bb);
        clut8[i] = rr << 11 | gg << 5 | bb;
        
        i++;
      }
  #endif
} //defaultClut()

#if LCD_COLORDEPTH==8
void glcdCustomClut(uint8_t* cclut) {
  for (byte h = 0; h < cclut[0]; h++)
    clut8[h] = ((cclut[h * 3 + 3] & 0xF8) << 8) | ((cclut[h * 3 + 2] & 0xFC) << 3) | (cclut[h * 3 + 1] >> 3);
}

void glcdSetClutColor(uint8_t c, uint8_t r, uint8_t g, uint8_t b) {
  clut8[c] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
#endif

#if !defined LCD_DOUBLE_PIXEL_NEAREST && !defined LCD_DOUBLE_PIXEL_LINEAR && LCD_COLORDEPTH==16
// TODO: needs some code for custom scanline scanout (e.g. for retro emulators)
/*
glcdBuffer_t* getScanlineBuf(byte n) {
  if (n == 0)
    return (glcdBuffer_t*) &scanlineBuf0[0];
  if (n == 1)
    return (glcdBuffer_t*) &scanlineBuf1[0];
}

void txScanline_internal() {
  glcdSendBufferWord((uint16_t*) &scanlineBuf[0], SCREEN_WIDTH);
}

void txScanline((uint16_t*) scanlineBuf) {
  glcdSendBufferWord(scanlineBuf, SCREEN_WIDTH);
}
*/
#endif

#if defined LCD_DOUBLE_PIXEL_NEAREST && LCD_COLORDEPTH==16
// It would probably be a little faster to use the PIO here and just scan out every pixel twice.
void txScanline() {
  if (scanlineUneven) {
    scanlineUneven = 0;
    glcdSendBufferWord((uint16_t*) &scanlineBuf[0], SCREEN_WIDTH * 2);
    return;
  }

  if (dmaScanline < SCREEN_HEIGHT)
    for (uint16_t h = 0; h < SCREEN_WIDTH * 2 - 1; h++)
      scanlineBuf[h] = _in_flip[dmaScanline * SCREEN_WIDTH + h / 2];

  scanlineUneven = 1;
  
  glcdSendBufferWord((uint16_t*) &scanlineBuf[0], SCREEN_WIDTH * 2);

  dmaScanline++;

} // txScanline_double_nearest 16bit
#endif

#if defined LCD_DOUBLE_PIXEL_NEAREST && LCD_COLORDEPTH==8
// It would probably be a little faster to use the PIO here and just scan out every pixel twice.
void txScanline() {
  if (scanlineUneven) {
    scanlineUneven = 0;
    glcdSendBufferWord((uint16_t*) &scanlineBuf[0], SCREEN_WIDTH * 2);
    return;
  }

  if (dmaScanline < SCREEN_HEIGHT)
    for (uint16_t h = 0; h < SCREEN_WIDTH * 2 - 1; h++)
      scanlineBuf[h] = clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h / 2]];

  scanlineUneven = 1;
  
  glcdSendBufferWord((uint16_t*) &scanlineBuf[0], SCREEN_WIDTH * 2);

  dmaScanline++;

} // txScanline_double_nearest 8bit
#endif

#if defined LCD_DOUBLE_PIXEL_LINEAR
void txScanline() {
  if (scanlineUneven) {
    scanlineUneven = 0;
    glcdSendBufferWord((uint16_t*) &scanlineBufMid[0], SCREEN_WIDTH * 2);
    return;
  }

  uint16_t* scanlineBufTmp = scanlineBufLst;
  scanlineBufLst = &scanlineBufNxt[0];
  scanlineBufNxt = scanlineBufTmp;

  // proof on concept only

  if (dmaScanline < SCREEN_HEIGHT) {
    for (uint16_t h = 0; h < SCREEN_WIDTH * 2 - 1; h++) {
      if (h % 2 == 0)
        #if LCD_COLORDEPTH==8
        scanlineBufNxt[h] = clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h / 2]];
        #elif LCD_COLORDEPTH==16
        scanlineBufNxt[h] = _in_flip[dmaScanline * SCREEN_WIDTH + h / 2];
        #endif
      else {
        #if LCD_COLORDEPTH==8
        uint8_t r1 = RGB565_red5(clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 ]]);
        uint8_t g1 = RGB565_green6(clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 ]]);
        uint8_t b1 = RGB565_blue5(clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 ]]);
        uint8_t r2 = RGB565_red5(clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 + 1]]);
        uint8_t g2 = RGB565_green6(clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 + 1]]);
        uint8_t b2 = RGB565_blue5(clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 + 1]]);
        #elif LCD_COLORDEPTH==16
        uint8_t r1 = (_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 ] >> 11) & 0x1f;
        uint8_t g1 = (_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 ] >> 5) & 0x3f;
        uint8_t b1 = (_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 ] & 0x1f);
        uint8_t r2 = (_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 + 1] >> 11) & 0x1f;
        uint8_t g2 = (_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 + 1] >> 5) & 0x3f;
        uint8_t b2 = (_in_flip[dmaScanline * SCREEN_WIDTH + h / 2 + 1] & 0x1f);
        #endif
        scanlineBufNxt[h] = (r1 + r2) / 2 << 11 | (g1 + g2) / 2 << 5 | (b1 + b2) / 2;
      }
    }
  } else {
    for (uint16_t h = 0; h < SCREEN_WIDTH * 2; h++)
      scanlineBufNxt[h] = 0;
  }

  if (dmaScanline != 0) {

    for (uint16_t h = 0; h < SCREEN_WIDTH * 2; h++) {
      uint8_t r1 = (scanlineBufNxt[h] >> 11) & 0x1f;
      uint8_t g1 = (scanlineBufNxt[h] >> 5) & 0x3f;
      uint8_t b1 = (scanlineBufNxt[h] & 0x1f);
      uint8_t r2 = (scanlineBufLst[h] >> 11) & 0x1f;
      uint8_t g2 = (scanlineBufLst[h] >> 5) & 0x3f;
      uint8_t b2 = (scanlineBufLst[h] & 0x1f);
      scanlineBufMid[h] = (r1 + r2) / 2 << 11 | (g1 + g2) / 2 << 5 | (b1 + b2) / 2;
    }

  }

  scanlineUneven = 1;
  
  dmaScanline++;

  glcdSendBufferWord((uint16_t*) &scanlineBufLst[0], SCREEN_WIDTH * 2);

} // txScanline_double_linear
#endif

#if LCD_COLORDEPTH==8 && !defined LCD_DOUBLE_PIXEL_NEAREST && !defined LCD_DOUBLE_PIXEL_LINEAR
void txScanline() {
  if (dmaScanline < SCREEN_HEIGHT)
    for (uint16_t h = 0; h < SCREEN_WIDTH * 2 - 1; h++)
      scanlineBuf[h] = clut8[_in_flip[dmaScanline * SCREEN_WIDTH + h]];

  dmaScanline++;

  glcdSendBufferWord((uint16_t*) &scanlineBuf[0], SCREEN_WIDTH);

} // txScanline_8_bit
#endif

int glcdIsDmaDone() {
  if (_in_flip == NULL)
    return 1;
  else
    return 0;
}

void __isr dmaComplete() {
  if (!dma_channel_get_irq0_status(dma_tx_channel))
    return;

  dma_channel_acknowledge_irq0(dma_tx_channel); // clear irq flag
  
  #if defined LCD_DOUBLE_PIXEL_LINEAR || defined LCD_DOUBLE_PIXEL_NEAREST || LCD_COLORDEPTH==8
  if (_in_flip == NULL)
//  if (!dmaScanline)
    return;

  if (dmaScanline >= SCREEN_HEIGHT && scanlineUneven == 0) {
    // all scanlines done. reset counter and exit
    dmaScanline = 0;
    scanlineUneven = 0;
    _in_flip = NULL;
    return;
  }

  multicore_reset_core1();
  multicore_launch_core1(txScanline);
  #else
  _in_flip = NULL;
  #endif
  
}

bool dmaBusy(void) {
  if (!DMA_Enabled)
    return false;

  if (dma_channel_is_busy(dma_tx_channel))
    return true;
  else
    return false;
}

void dmaWait(void) {
  if (!DMA_Enabled)
    return;

  while (dma_channel_is_busy(dma_tx_channel));
}

bool dmaInit() {
  if (DMA_Enabled) return false;

  dma_tx_channel = dma_claim_unused_channel(false);

  if (dma_tx_channel < 0) return false;

  dma_tx_config = dma_channel_get_default_config(dma_tx_channel);

  channel_config_set_transfer_data_size(&dma_tx_config, DMA_SIZE_16);  // 16 bit per transmission (7% speed improvement)

#if defined LCD_PIN_INVERSION
  channel_config_set_bswap(&dma_tx_config, true);
#else
  channel_config_set_bswap(&dma_tx_config, false);
#endif

  channel_config_set_dreq(&dma_tx_config, pio_get_dreq(lcd_pio, pio_sm, true));

  dma_channel_set_irq0_enabled(dma_tx_channel, true);

  irq_set_exclusive_handler(DMA_IRQ_0, dmaComplete);

  irq_set_enabled(DMA_IRQ_0, true);

  DMA_Enabled = true;

  return true;
}

void dmaShutdown(void) {
  if (!DMA_Enabled)
    return;

  dma_channel_unclaim(dma_tx_channel);

  DMA_Enabled = false;
}

void glcdShowDisplayBuffer(glcdBuffer_t* buf)
{
  while (dmaScanline != 0); // wait for any pending transmission
  
  //dmaWait();  //might be omitted here, because there is another check in glcdSendBufferWord

  _in_flip = &buf[BUF_HEADER_SIZE];

  #if defined LCD_DOUBLE_PIXEL_LINEAR || defined LCD_DOUBLE_PIXEL_NEAREST || LCD_COLORDEPTH==8
  multicore_reset_core1();
  multicore_launch_core1(txScanline);
  #else
  glcdSendBufferWord(&buf[BUF_HEADER_SIZE], glcdGetBufWidth(buf) * glcdGetBufHeight(buf));
  #endif
}

bool pioInit(uint16_t clock_div, uint16_t fract_div) {

  pio_sm = pio_claim_unused_sm(lcd_pio, false);
  
  if (pio_sm < 0) {
    return false;
  }
  
  program_offset = pio_add_program(lcd_pio, &lcd_output_program);

  pio_gpio_init(lcd_pio, LCD_PIN_WR);

  for (int i = 0; i < 8; i++) {
    pio_gpio_init(lcd_pio, LCD_PIN_D0 + i);
  }

  pio_sm_set_consecutive_pindirs(lcd_pio, pio_sm, LCD_PIN_WR, 1, true);
  pio_sm_set_consecutive_pindirs(lcd_pio, pio_sm, LCD_PIN_D0, 8, true);

  pio_sm_config c = lcd_output_program_get_default_config(program_offset);
  sm_config_set_sideset_pins(&c, LCD_PIN_WR);
  sm_config_set_out_pins(&c, LCD_PIN_D0, 8);
  sm_config_set_clkdiv_int_frac(&c, clock_div, fract_div);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  sm_config_set_out_shift(&c, false, true, 16); // ori
  pio_sm_init(lcd_pio, pio_sm, program_offset + lcd_output_offset_tx16, &c); // ori

  pio_sm_set_enabled(lcd_pio, pio_sm, true);

  pull_stall_mask = 1u << (PIO_FDEBUG_TXSTALL_LSB + pio_sm); // ori

  pio_instr_jmp8  = pio_encode_jmp(program_offset + lcd_output_offset_tx8);

  return true;
} //pioInit

void pioWait() {
  lcd_pio->fdebug = pull_stall_mask;
  while (!(lcd_pio->fdebug & pull_stall_mask));
}

void set_rs(byte value) {
  digitalWrite(LCD_PIN_DC, value);
}

void set_rst(byte value) {
  digitalWrite(LCD_PIN_RST, value);
}

void glcdSendDatByte(uint8_t cmd) {
  #if defined LCD_PIN_INVERSION
  cmd = swapBitOrderN(cmd, 8);
  #endif
    
  lcd_pio->sm[pio_sm].instr = pio_instr_jmp8;

  lcd_pio->txf[pio_sm] = cmd;

  pioWait();
}

void glcdSendCmdByte(uint8_t cmd) {
  #if defined LCD_PIN_INVERSION
  cmd = swapBitOrderN(cmd, 8);
  #endif
  
  set_rs(LOW);
  
  lcd_pio->sm[pio_sm].instr = pio_instr_jmp8;

  lcd_pio->txf[pio_sm] = cmd;

  set_rs(HIGH);
}

// send a buffer of DATA bytes to the display
void glcdSendBufferWord(uint16_t *buf, uint32_t buffersize) {
  if ((buffersize == 0) || (!DMA_Enabled)) return;

  dmaWait();
  pioWait();

  dma_channel_configure(dma_tx_channel, &dma_tx_config, &lcd_pio->txf[pio_sm], buf, buffersize, true);

}

void glcdBacklight(byte level) {
  backlightpwm->setPWM(LCD_PIN_BL_PWM, 1000000, level);
}

void glcdInitGraphics()
{
  // configure pin modes
  pinMode(LCD_PIN_RST, OUTPUT);
  pinMode(LCD_PIN_DC, OUTPUT);

  pioInit(3, 0);
  dmaInit();

  lcdControllerInit();  // hardware dependent

  defaultClut();
}