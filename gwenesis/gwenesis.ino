/*
Gwenesis : Genesis & megadrive Emulator.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

Emulator by bzhxx, ported to the Pico Held by fcipaq (https://github.com/fcipaq)

__author__ = "bzhxx"
__contact__ = "https://github.com/bzhxx"
__license__ = "GPLv3"

*/

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"  // overclock
#include "hardware/vreg.h"

extern "C" {
/* Gwenesis Emulator */
#include "src/gwenesis/cpus/M68K/m68k.h"
#include "src/gwenesis/sound/z80inst.h"
#include "src/gwenesis/sound/ym2612.h"
#include "src/gwenesis/sound/gwenesis_sn76489.h"
#include "src/gwenesis/bus/gwenesis_bus.h"
#include "src/gwenesis/io/gwenesis_io.h"
#include "src/gwenesis/vdp/gwenesis_vdp.h"
#include "src/gwenesis/savestate/gwenesis_savestate.h"
}

/* Pico Held hw library */
#include "src/pplib/graphics.h"
#include "src/pplib/sound.h"
#include "src/pplib/buttons.h"
#include "src/pplib/fonts.h"
#include "src/pplib/fonts/f13x16.h"

/* ---- compiler switches ---- */
#define DEBUG_DISPLAY
#define OVERCLOCK
//#define ROM_HEADER_FILE
//#define RUN_Z80_AFTER_MAINLOOP

/* ---- ROM header file ---- */
#ifdef ROM_HEADER_FILE
//#include "rom_vdptest_be.h" // Test ROM by Tristan Seifert
#endif

unsigned short empty_line[320];

#pragma GCC optimize("Ofast")

#define ENABLE_DEBUG_OPTIONS 0

/* ---- sound ---- */
// 1: generate sound every line, 0: generate sound every frame
uint8_t snd_accurate = 0;  // 0 works great for sonic 1

volatile int snd_speed_fract = 0;
volatile int snd_speed_div = 9;
uint32_t snd_speed_fract_tmp = 0;
uint32_t snd_speed_div_tmp = 9;

int snd_output_volume = 3;  // 2: old prototype, 5: new prototype

volatile uint8_t soll_snd = 1;

uint8_t audio_enabled = 1;

/* shared variables with gwenesis_sn76589 */
int16_t gwenesis_sn76489_buffer[GWENESIS_AUDIO_BUFFER_LENGTH_PAL * 2];  // 888 = NTSC, PAL = 1056 (too big) //GWENESIS_AUDIO_BUFFER_LENGTH_PAL];
int sn76489_index;                                                      /* sn78649 audio buffer index */
int sn76489_clock;                                                      /* sn78649 clock in video clock resolution */

/* shared variables with gwenesis_ym2612 */
int8_t gwenesis_ym2612_buffer[GWENESIS_AUDIO_BUFFER_LENGTH_PAL * 2];  //GWENESIS_AUDIO_BUFFER_LENGTH_PAL];
int ym2612_index;                                                     /* ym2612 audio buffer index */
int ym2612_clock;                                                     /* ym2612 clock in video clock resolution */

/* ---- emulator ---- */
/* Clocks and synchronization */
/* system clock is video clock */
volatile int system_clock;

unsigned int lines_per_frame = LINES_PER_FRAME_NTSC;  //262; /* NTSC: 262, PAL: 313 */
unsigned int scan_line;

volatile uint8_t framedrop_cnt = 0;

/* ---- input ---- */
/* Configurable keys mapping for A,B and C */
extern unsigned short button_state[3];

#define NB_OF_COMBO 6

uint32_t longpress_s_timer = 0;
uint8_t toggle_s = 0;

/* ---- multi core sync ---- */
mutex_t core1_busy;

/* ======================= implementation ======================= */

/* callback used by the emulator to capture keys */
void gwenesis_io_get_buttons() {

  uint16_t dpad = checkDPad();
  uint16_t buts = checkButtons();

  button_state[0] = ((dpad & DPAD_LEFT) != 0) << PAD_LEFT | ((dpad & DPAD_RIGHT) != 0) << PAD_RIGHT | ((dpad & DPAD_UP) != 0) << PAD_UP | ((dpad & DPAD_DOWN) != 0) << PAD_DOWN;

  if (((buts & BUTTON_1) != 0) && ((buts & BUTTON_2) != 0) && ((buts & BUTTON_3) != 0)) {
    button_state[0] |= 1 << PAD_S;
  } else {
    button_state[0] |=
      //    ((buts & BUTTON_YELLOW) != 0) << PAD_S |
      ((buts & BUTTON_RED) != 0) << PAD_A | ((buts & BUTTON_BLUE) != 0) << PAD_B | ((buts & BUTTON_GREEN) != 0) << PAD_C;
  }

  /* Short press all three buttons to trigger a start button event */
  /* Long press all three buttons to to toggle sound output */
  if ((button_state[0] & (1 << PAD_S)) != 0) {
    if (toggle_s == 0) {
      toggle_s = 1;
      longpress_s_timer = millis();
    }
  } else {
    if (toggle_s == 1) {
      toggle_s = 0;
      if (millis() - longpress_s_timer > 2000) {
        soll_snd++;
        if (soll_snd == 2)
          soll_snd = 0;
      }
    }
  }

  button_state[0] = ~button_state[0];
}

static void gwenesis_system_init() {
  /* init emulator sound system with shared audio buffer */

  //assert(SND_BUFFER_SAMPLES > GWENESIS_AUDIO_BUFFER_LENGTH_PAL);

  memset(gwenesis_sn76489_buffer, 0, sizeof(gwenesis_sn76489_buffer));
  memset(gwenesis_ym2612_buffer, 0, sizeof(gwenesis_ym2612_buffer));
}

/*
// TODO: reimplement save/load state

static bool gwenesis_system_SaveState(char *pathName) {
  int size = 0;
  printf("Saving state...\n");
  size = saveGwenesisState((unsigned char *)ACTIVE_FILE->save_address,ACTIVE_FILE->save_size);
  printf("Saved state size:%d\n", size);
  return true;
}

static bool gwenesis_system_LoadState(char *pathName) {
  printf("Loading state...\n");
  return loadGwenesisState((unsigned char *)ACTIVE_FILE->save_address);
}
*/

void makeGraphics() {
  while (1) {
    mutex_enter_blocking(&core1_busy);
    mutex_enter_blocking(&core1_busy);
    mutex_exit(&core1_busy);
    uint32_t system_clock_tmp = system_clock;

    if (!framedrop_cnt) {
      /* render scan_line */
      gwenesis_vdp_render_line(scan_line); 
    }

    if (system_clock_tmp == 0 && audio_enabled) {
      //z80_run(system_clock + VDP_CYCLES_PER_LINE * 262);
      ym2612_run(VDP_CYCLES_PER_LINE * 262 / GWENESIS_AUDIO_SAMPLING_DIVISOR);
      gwenesis_SN76489_run(VDP_CYCLES_PER_LINE * 262 / GWENESIS_AUDIO_SAMPLING_DIVISOR);
      uint8_t snd_buf[ym2612_index * GWENESIS_AUDIO_SAMPLING_DIVISOR * 2];

      for (int h = 0; h < ym2612_index * GWENESIS_AUDIO_SAMPLING_DIVISOR * 2; h++)
        snd_buf[h] = gwenesis_ym2612_buffer[h / 2 / GWENESIS_AUDIO_SAMPLING_DIVISOR] + 
                  (gwenesis_sn76489_buffer[h / 2 / GWENESIS_AUDIO_SAMPLING_DIVISOR] / (2 << (11-snd_output_volume))) + 128;

      if (enqueSndBuf(snd_buf, ym2612_index * GWENESIS_AUDIO_SAMPLING_DIVISOR * 2, NONBLOCKING, SRC_RAM) <= 1) {
        // data coming in too slowly - slow down output
        if (snd_speed_fract > 0) {
          snd_speed_fract -= 1;
        } else {
          snd_speed_fract = 255;
          snd_speed_div--;
        }
        sndSetSpeed(snd_speed_div, snd_speed_fract);
      } else {
        // data coming in too fast - speed up output
        if (snd_speed_fract < 255) {
          snd_speed_fract += 1;
        } else {
          snd_speed_fract = 0;
          snd_speed_div++;
        }
        sndSetSpeed(snd_speed_div, snd_speed_fract);
      }
    }  // enque buffer
  }    // loop
}

/* Main */
int main() {
  /* setup hardware */
  initButtons();

#ifdef OVERCLOCK
  vreg_set_voltage(VREG_VOLTAGE_1_20);
  sleep_ms(10);
  uint32_t sys_clock_khz = 276000;  //252000 //276000
  set_sys_clock_khz(sys_clock_khz, true);
#endif

  glcdInitGraphics();

  initSound();
  sndSetSpeed(3, 127);

  glcdBacklight(30);

  /* Load ROM  */
#ifdef ROM_HEADER_FILE
  ROM_DATA = ROM_DATA_FLASH;
#else
  ROM_DATA = (const unsigned char*)0x100c3500;  // 800 KB application, the rest is data
#endif

  /* emulator init */
  load_cartridge();
  gwenesis_system_init();
  power_on();
  reset_emulation();

  /* Setup 2nd CPU core */
  mutex_init(&core1_busy);
  multicore_reset_core1();
  multicore_launch_core1(makeGraphics);

  /* setup graphics output */
  unsigned short* screen = glcdGetBuffer(320, 1);

  gwenesis_vdp_set_buffer(&screen[BUF_HEADER_SIZE]);

  uint8_t framedrop_mode = 2;  // 0 = disabled, 1 = "adaptive" (experimental), 2 = skip every other frame
  int32_t frame_delay = 0;

  extern unsigned char gwenesis_vdp_regs[0x20];
  extern unsigned int gwenesis_vdp_status;
  extern unsigned int screen_width, screen_height;
  static int vert_screen_offset = REG1_PAL ? 0 : (240 - 224);
  int hint_counter;

  extern int hint_pending;

  /* upper black bar */
  for (int h = 0; h < vert_screen_offset / 2; h++)
    glcdSendBufferWord(empty_line, 320);

  /* screen overlay */
#ifdef DEBUG_DISPLAY
  glcdBuffer_t* scr_overlay_buf = glcdGetBuffer(100, 50);
#endif

  uint32_t frame_timer_start = micros();
  uint32_t frame_timer_end;

#ifdef DEBUG_DISPLAY
  char debug_print[11];
  uint32_t t0;
  uint16_t cnt = 0;
  t0 = millis();
#endif

  while (true) {

    /* Eumulator loop */
    hint_counter = gwenesis_vdp_regs[10];

    screen_height = REG1_PAL ? 240 : 224;
    screen_width = REG12_MODE_H40 ? 320 : 256;
    lines_per_frame = REG1_PAL ? LINES_PER_FRAME_PAL : LINES_PER_FRAME_NTSC;
    vert_screen_offset = REG1_PAL ? 0 : (240 - 224);

    gwenesis_vdp_render_config();

    /* Reset the difference clocks and audio index */
    system_clock = 0;
    zclk = 0;

    ym2612_clock = 0;
    ym2612_index = 0;

    sn76489_clock = 0;
    sn76489_index = 0;

    scan_line = 0;

    /* enable/disable sound */
    audio_enabled = soll_snd;

#ifndef RUN_Z80_AFTER_MAINLOOP
    /* Z80 CPU  */
    if (audio_enabled && !snd_accurate)
      z80_run(VDP_CYCLES_PER_LINE * 262);
#endif

    while (scan_line < lines_per_frame) {

      /* CPUs  */
      m68k_run(system_clock + VDP_CYCLES_PER_LINE);
      if (audio_enabled && snd_accurate)
        z80_run(system_clock + VDP_CYCLES_PER_LINE);

      /* Audio & Video */
      // as long as the mutex is not blocked, core1 is still busy
      while (mutex_try_enter(&core1_busy, NULL) == true)
        mutex_exit(&core1_busy);
      mutex_exit(&core1_busy);  // When it's finally blocked, release it again

      if (!framedrop_cnt) {
        /* Video */
        if (scan_line < SCREEN_HEIGHT) {
          // WA for VPD running one line ahead of CPU
          // TODO: fixme
          if (scan_line > 0 && scan_line < (SCREEN_HEIGHT - vert_screen_offset)) {
#ifdef DEBUG_DISPLAY
            glcdBlitBuf(210, -scan_line + 10, scr_overlay_buf, screen, 0x0001, SRC_RAM);
#endif
            glcdSendBufferWord(&screen[BUF_HEADER_SIZE], 320);
          } else
            glcdSendBufferWord(empty_line, 320);
        }
      }

      // On these lines, the line counter interrupt is reloaded
      if ((scan_line == 0) || (scan_line > screen_height)) {
        //  if (REG0_LINE_INTERRUPT != 0)
        //    printf("HINTERRUPT counter reloaded: (scan_line: %d, new
        //    counter: %d)\n", scan_line, REG10_LINE_COUNTER);
        hint_counter = REG10_LINE_COUNTER;
      }

      // interrupt line counter
      if (--hint_counter < 0) {
        if ((REG0_LINE_INTERRUPT != 0) && (scan_line <= screen_height)) {
          hint_pending = 1;
          // printf("Line int pending %d\n",scan_line);
          if ((gwenesis_vdp_status & STATUS_VIRQPENDING) == 0)
            m68k_update_irq(4);
        }
        hint_counter = REG10_LINE_COUNTER;
      }

      scan_line++;

      // vblank begin at the end of last rendered line
      if (scan_line == screen_height) {

        if (REG1_VBLANK_INTERRUPT != 0) {
          // printf("IRQ VBLANK\n");
          gwenesis_vdp_status |= STATUS_VIRQPENDING;
          m68k_set_irq(6);
        }
        z80_irq_line(1);
      }

      if (scan_line == (screen_height + 1)) {
        z80_irq_line(0);
      }

      system_clock += VDP_CYCLES_PER_LINE;
    }

    /* Frame rate control */
    frame_timer_end = micros() - frame_timer_start;
    frame_delay -= 16667 - frame_timer_end;

// works for lost vikings (with sound_accurate == 0)
#ifdef RUN_Z80_AFTER_MAINLOOP
      /* Z80 CPU  */
        if (audio_enabled && !snd_accurate)
          z80_run(VDP_CYCLES_PER_LINE * 262);
#endif

    if (framedrop_mode == 1) {
      if (frame_delay > 16667 * 2)
        framedrop_cnt = 1;
      else
        framedrop_cnt = 0;
    }

    if (framedrop_mode == 2) {
      framedrop_cnt++;
      if (framedrop_cnt > 1)
        framedrop_cnt = 0;
    }

    // Frame rate limiter - in case we ever might be too fast

    if (frame_delay < 0) [[unlikely]] {
      frame_delay = 0;
      while (micros() - frame_timer_start < 16667);  // 60 Hz
    }
    
    /*  OSD */
#ifdef DEBUG_DISPLAY
    cnt++;
    if (millis() - t0 > 1000) {
      glcdFillRect(0, 0, glcdGetBufWidth(scr_overlay_buf) - 1, glcdGetBufHeight(scr_overlay_buf) - 1, 0x0001, scr_overlay_buf);

      itoa((float)cnt * 1000. / (float)(millis() - t0), debug_print, 10);
      uint16_t col = audio_enabled ? RGBColor888_565(255, 255, 0) : 0xffff;
      writeString(42, 2, 0x0000, 0xffff, FONT_TRANSP, debug_print, font_13x16, scr_overlay_buf);
      writeString(40, 0, col, 0xffff, FONT_TRANSP, debug_print, font_13x16, scr_overlay_buf);

      t0 = millis();
      cnt = 0;
    }
#endif

    // reset m68k cycles to the begin of next frame cycle
    m68k.cycles -= system_clock;

  }  // end of emulator loop
}
