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
#include "hardware/flash.h"

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
#include "src/pplib/fonts/pixelmix_14_16.h"

#include "src/emumgr/emumgr.h"

/* ---- compiler switches ---- */
#define OVERCLOCK

unsigned short empty_line[320];

#pragma GCC optimize("Ofast")

#define ENABLE_DEBUG_OPTIONS 0

extern char __flash_binary_end;

/* ---- sound ---- */
// see compiler switch GWENESIS_AUDIO_SAMPLING_DIVISOR in gwenesis_bus.h for sampling quality
uint8_t z80_force_accurate = 1;
uint8_t snd_accurate = 1;  // 1: generate sound cycle accurate, 0: generate sound frame accurate

volatile int snd_speed_fract = 0;
volatile int snd_speed_div = 9;
uint32_t snd_speed_fract_tmp = 0;
uint32_t snd_speed_div_tmp = 9;

int snd_output_volume = 4;  // 2: old prototype, 4: new prototype

volatile uint8_t soll_snd = 0;
int audio_enabled = 0;

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

uint8_t enable_debug_display = 0;

//extern unsigned char* M68K_RAM;
//extern unsigned char* VRAM;
//extern unsigned char* ZRAM;

/* ---- input ---- */
/* Configurable keys mapping for A,B and C */
extern unsigned short button_state[3];

#define NB_OF_COMBO 6

uint32_t longpress_s_timer = 0;
uint8_t evoke_menu = 0;

/* ---- multi core sync ---- */
mutex_t core1_busy;

/* ======================= implementation ======================= */

/* --------------------- quirk management -------------------- */

#define QUIRKS_NUM 3

String rom_names[QUIRKS_NUM] = PROGMEM {
    // Sonic (US/EUR)
    "\x4f\x53\x49\x4e\x20\x43\x48\x54\x20\x45\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x45\x48\x47\x44\x48\x45\x47\x4f\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20",
    // Lionking (World)
    "\x49\x4c\x4e\x4f\x4b\x20\x4e\x49\x20\x47\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20",
    // Aladdin (Europe)
    "\x4c\x41\x44\x41\x49\x44\x20\x4e\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"

};

void manage_quirks() {
  char rom_name[49];
  memcpy(rom_name, (void*) (ROM_DATA + 0x120), 48);
  rom_name[48] = 0;

  // Sonic (US/EUR)
  if (!rom_names[0].compareTo(rom_name)) {
    soll_snd = audio_enabled = 1;
    z80_force_accurate = 0;
    snd_accurate = 0;
    snd_output_volume = 4;
  }

  // Lionking (World)
  if (!rom_names[1].compareTo(rom_name)) {
    soll_snd = audio_enabled = 0;
    z80_force_accurate = 1;
    snd_accurate = 1;
    snd_output_volume = 4;
  }

  // Aladdin (Europe)
  if (!rom_names[2].compareTo(rom_name)) {
    soll_snd = audio_enabled = 0;
    z80_force_accurate = 0;
    snd_accurate = 1;
    snd_output_volume = 4;
  }
}

/* ----------------------- emulator ---------------------- */

/* callback used by the emulator to capture keys */
void gwenesis_io_get_buttons() {

  uint16_t dpad = checkDPad();
  uint16_t buts = checkButtons();

  button_state[0] = ((dpad & DPAD_LEFT) != 0) << PAD_LEFT | ((dpad & DPAD_RIGHT) != 0) << PAD_RIGHT | ((dpad & DPAD_UP) != 0) << PAD_UP | ((dpad & DPAD_DOWN) != 0) << PAD_DOWN;

  /* Short press all three buttons to trigger a start button event */
  if (((buts & BUTTON_1) != 0) && ((buts & BUTTON_2) != 0) && ((buts & BUTTON_3) != 0)) {
    button_state[0] |= 1 << PAD_S;
  } else {
    button_state[0] |=
      ((buts & BUTTON_3) != 0) << PAD_A | ((buts & BUTTON_2) != 0) << PAD_B | ((buts & BUTTON_1) != 0) << PAD_C;
  }

  button_state[0] = ~button_state[0];
}

static void gwenesis_system_init() {
  /* init emulator sound system with shared audio buffer */

  //assert(SND_BUFFER_SAMPLES > GWENESIS_AUDIO_BUFFER_LENGTH_PAL);

  memset(gwenesis_sn76489_buffer, 0, sizeof(gwenesis_sn76489_buffer));
  assert(gwenesis_sn76489_buffer);
  memset(gwenesis_ym2612_buffer, 0, sizeof(gwenesis_ym2612_buffer));
  assert(gwenesis_ym2612_buffer);
}

void makeSound() {
  if (audio_enabled) {
    // if snd_accurate == 1 then sound of a whole frame will be generated here
    // if snd_accurate == 0 then only the missing sound samples for a whole frame will be generated
    ym2612_run(0 + VDP_CYCLES_PER_LINE * 262); 
    gwenesis_SN76489_run(0 + VDP_CYCLES_PER_LINE * 262);

    uint8_t snd_buf[ym2612_index * 2 * GWENESIS_AUDIO_SAMPLING_DIVISOR];

    for (int h = 0; h < ym2612_index * 2 * GWENESIS_AUDIO_SAMPLING_DIVISOR; h++)
      snd_buf[h] = gwenesis_ym2612_buffer[h / 2 / GWENESIS_AUDIO_SAMPLING_DIVISOR] + 
                (gwenesis_sn76489_buffer[h / 2/ GWENESIS_AUDIO_SAMPLING_DIVISOR] / (2 << (11-snd_output_volume))) + 128;

    if (enqueSndBuf(snd_buf, ym2612_index * 2 * GWENESIS_AUDIO_SAMPLING_DIVISOR, NONBLOCKING, SRC_RAM) <= 1) {
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

    ym2612_clock = 0;
    ym2612_index = 0;

    sn76489_clock = 0;
    sn76489_index = 0;
  
  }  // enque buffer  
}

void core1_mainloop() {
  while (1) {
    mutex_enter_blocking(&core1_busy);
    mutex_enter_blocking(&core1_busy);
    mutex_exit(&core1_busy);

    uint32_t system_clock_tmp = system_clock;

    if (!framedrop_cnt) {
      /* render scan_line */
      gwenesis_vdp_render_line(scan_line);
    }

    if (audio_enabled && !snd_accurate && system_clock_tmp == 0) {
      z80_run(VDP_CYCLES_PER_LINE * 262);      
      makeSound();
    }

  }  // loop
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

  glcdBacklight(50);

  /* Load ROM  */
#if 0
  uint32_t rom_addr = (uint32_t) &__flash_binary_end + FLASH_SECTOR_SIZE * 2;
  if (rom_addr % FLASH_SECTOR_SIZE) {
    rom_addr = (rom_addr / FLASH_SECTOR_SIZE + 1) * FLASH_SECTOR_SIZE;
  }
  ROM_METADATA = (const unsigned char*) rom_addr;
#else
  ROM_METADATA = (const unsigned char*) 0x100e1000;  // fixed offset: 900 KB application
#endif

  ROM_DATA = ROM_METADATA + FLASH_SECTOR_SIZE;  // header is the size of one erase block (FLASH_SECTOR_SIZE)

  /* setup graphics output */
  unsigned short* screen = glcdGetBuffer(320, 1);
  assert(screen);

//  VRAM = (unsigned char*) malloc(VRAM_MAX_SIZE);
//  assert(VRAM);
//  M68K_RAM = (unsigned char*) malloc(MAX_RAM_SIZE);
//  assert(M68K_RAM);
//  ZRAM = (unsigned char*) malloc(MAX_Z80_RAM_SIZE);
//  assert(ZRAM);

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

  //if (!check_rom_valid((uint8_t*) (ROM_DATA)))

  /* emulator init */
  manage_quirks();
  load_cartridge();
  gwenesis_system_init();
  power_on();
  reset_emulation();
  emulator_menu();

  /* Setup 2nd CPU core */
  mutex_init(&core1_busy);
  multicore_reset_core1();
  multicore_launch_core1(core1_mainloop);

  /* screen overlay */
  glcdBuffer_t* scr_overlay_buf = glcdGetBuffer(30, 20);
  assert(scr_overlay_buf);
  memset(&scr_overlay_buf[BUF_HEADER_SIZE], 0xaf, 30 * 20 * 2);
  char debug_print[11];
  uint32_t t0;
  uint16_t cnt = 0;
  t0 = millis();

  /* fps control */
  uint32_t frame_timer_start = micros();
  uint8_t frame_cnt = 0;

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

    scan_line = 0;

    /* enable/disable sound */
    audio_enabled = soll_snd;

    /* Z80 CPU  */
    if (!audio_enabled && !z80_force_accurate)
      z80_run(VDP_CYCLES_PER_LINE * 262);

    while (scan_line < lines_per_frame) {

      /* CPUs  */
      m68k_run(system_clock + VDP_CYCLES_PER_LINE);
      if ((audio_enabled && snd_accurate) || (z80_force_accurate))
        z80_run(system_clock + VDP_CYCLES_PER_LINE);

      // as long as the mutex is not blocked, core1 is still busy
      while (mutex_try_enter(&core1_busy, NULL) == true)
        mutex_exit(&core1_busy);
      mutex_exit(&core1_busy);  // When it's finally blocked, release it again

      /* Video */
      if (!framedrop_cnt) {
        /* Video */
        if (scan_line < SCREEN_HEIGHT) {
          // WA for VPD running one line ahead of CPU
          // TODO: fixme
          if (scan_line > 0 && scan_line < (SCREEN_HEIGHT - vert_screen_offset)) {
            if (enable_debug_display)
              glcdBlitBuf(SCREEN_WIDTH - 30, -scan_line + 3, scr_overlay_buf, screen, 0xafaf, SRC_RAM);
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

    if (framedrop_mode == 2) {
      framedrop_cnt++;
      if (framedrop_cnt > 1)
        framedrop_cnt = 0;
    }

    /* OSD */
    if (enable_debug_display) {
      cnt++;
      if (millis() - t0 > 1000) {
        glcdFillRect(0, 0, glcdGetBufWidth(scr_overlay_buf) - 1, glcdGetBufHeight(scr_overlay_buf) - 1, 0xafaf, scr_overlay_buf);

        itoa((float)cnt * 1000. / (float)(millis() - t0), debug_print, 10);
        writeString(2, 2, 0x0000, 0x1, FONT_TRANSP, debug_print, (glcdFont_t*)pixelmix_14_16, scr_overlay_buf);
        writeString(0, 0, RGBColor888_565(255, 255, 0), 0x1, FONT_TRANSP, debug_print, (glcdFont_t*)pixelmix_14_16, scr_overlay_buf);

        t0 = millis();
        cnt = 0;
      }
    }
    // reset m68k cycles to the begin of next frame cycle
    m68k.cycles -= system_clock;

    /* Long press all three buttons to to toggle sound output */
    uint16_t buts = checkButtons();

    if (((buts & BUTTON_1) != 0) && ((buts & BUTTON_2) != 0) && ((buts & BUTTON_3) != 0)) {
      if (evoke_menu == 0) {
        evoke_menu = 1;
        longpress_s_timer = millis();
      }
      if (evoke_menu == 1) {
        if (millis() - longpress_s_timer > 2000) {
          evoke_menu = 0;
          emulator_menu();
        }
      }
    } else {
      evoke_menu = 0;
    }

    if (audio_enabled && snd_accurate)
      makeSound();

    // Frame rate limiter - in case we ever might be too fast
    #if 1
      #define FRAME_AVG 6
      frame_cnt++;
      if (frame_cnt == FRAME_AVG) {
        while (micros() - frame_timer_start < 16667 * FRAME_AVG);  // 60 Hz
        frame_timer_start = micros();
        frame_cnt = 0;
      }
    #endif

  }  // end of emulator loop
}
