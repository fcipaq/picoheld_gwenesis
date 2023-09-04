/*
 * emumgr - emulation manager for GWENESIS on the Pico Held
 *
 * Copyright (C) 2023 Daniel Kammer (daniel.kammer@web.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
#include <string.h>

/* Pico Held hw library */
#include "../pplib/graphics.h"
#include "../pplib/sound.h"
#include "../pplib/buttons.h"
#include "../pplib/fonts.h"
#include "../pplib/fonts/pixelmix_14_16.h"
#include "../pplib/fonts/arial.h"

#include "hardware/flash.h"
#include "pico/bootrom.h"

#include <SPI.h>
#include <SD.h>
//#include "../sd/RP2040_SD.h"

#include "emumgr.h"
//#include "emumgr_lang_de.h"
#include "emumgr_lang_en.h"

extern "C" {
/* Gwenesis Emulator */
#include "../gwenesis/cpus/M68K/m68k.h"
#include "../gwenesis/sound/z80inst.h"
#include "../gwenesis/sound/ym2612.h"
#include "../gwenesis/sound/gwenesis_sn76489.h"
#include "../gwenesis/bus/gwenesis_bus.h"
#include "../gwenesis/io/gwenesis_io.h"
#include "../gwenesis/vdp/gwenesis_vdp.h"
#include "../gwenesis/savestate/gwenesis_savestate.h"
}

extern uint8_t enable_debug_display;
extern uint8_t snd_accurate;
extern int snd_output_volume;
extern volatile uint8_t soll_snd;
File savestate_file;
extern const unsigned char* ROM_DATA;

#define MENU_FONT pixelmix_14_16
//#define MENU_FONT font_13x16

#define SAVESTATE_DIRECTORY "/md/save/"
#define ROM_DIRECTORY "/md/roms/"

#define RM_SUCCESS 0
#define RM_ERR_NO_DIR -1
#define RM_ERR_NO_ROMS -2
#define RM_ERR_READING_ROM -3
#define RM_ERR_FLASHING_ROM -4

#define SELECTION_DELAY 200
#define SELECTION_DELAY_ROMLIST 10


#define NUM_CONFIRM_MENU_ITEMS_OVERWRITE 3

const char* confirm_menu_items_overwrite[NUM_CONFIRM_MENU_ITEMS_OVERWRITE] PROGMEM = { STR_CONFIRM_OVERWRITE,
                                                                   STR_NO,
                                                                   STR_YES };

#define NUM_CONFIRM_MENU_ITEMS_BL 3

const char* confirm_menu_items_bl[NUM_CONFIRM_MENU_ITEMS_BL] PROGMEM = { STR_CONFIRM_BL,
                                                                   STR_NO,
                                                                   STR_YES };

#define NUM_MAIN_MENU_ITEMS 8

const char* main_menu_items[NUM_MAIN_MENU_ITEMS] PROGMEM = { STR_MAIN_TITLE,
                                                             STR_LOAD,
                                                             STR_SAVE,
                                                             STR_SEL_ROM,
                                                             STR_SOUND,
                                                             STR_BRIGHTNESS,
                                                             STR_OPTIONS,
                                                             STR_LAUNCH };

#define NUM_OPT_MENU_ITEMS 5

const char* opt_menu_items[NUM_OPT_MENU_ITEMS] PROGMEM = { STR_OPTIONS_MENU,
                                                           STR_BUTTONS,
                                                           STR_TOGGLE_FPS,
                                                           STR_LAUNCH_BL,
                                                           //                                                           "WAs",
                                                           STR_GO_BACK };

#define NUM_SAVE_MENU_ITEMS 8

const char* save_menu_items[NUM_SAVE_MENU_ITEMS] PROGMEM = { STR_SAVE_STATE,
                                                             STR_SLOT_1,
                                                             STR_SLOT_2,
                                                             STR_SLOT_3,
                                                             STR_SLOT_4,
                                                             STR_SLOT_5,
                                                             STR_SLOT_6,
                                                             STR_GO_BACK };

#define NUM_LOAD_MENU_ITEMS 8

const char* load_menu_items[NUM_LOAD_MENU_ITEMS] PROGMEM = { STR_LOAD_STATE,
                                                             STR_SLOT_1,
                                                             STR_SLOT_2,
                                                             STR_SLOT_3,
                                                             STR_SLOT_4,
                                                             STR_SLOT_5,
                                                             STR_SLOT_6,
                                                             STR_GO_BACK };

#define NUM_BRI_MENU_ITEMS 7

const char* bri_menu_items[NUM_BRI_MENU_ITEMS] PROGMEM = { STR_BRIGHTNESS,
                                                           STR_20,
                                                           STR_40,
                                                           STR_60,
                                                           STR_80,
                                                           STR_100,
                                                           STR_GO_BACK };

#define NUM_SND_MENU_ITEMS 8

const char* snd_menu_items[NUM_SND_MENU_ITEMS] PROGMEM = { STR_SOUND,
                                                           STR_OFF,
                                                           STR_25,
                                                           STR_50,
                                                           STR_75,
                                                           STR_100,
                                                           STR_TOGGLE_SND_ACC,
                                                           STR_GO_BACK };

#define NUM_BTN_MENU_ITEMS 8

const char* btn_menu_items[NUM_SND_MENU_ITEMS] PROGMEM = { STR_BTN_MENU,
														   STR_BTN_ABC,
                                                           STR_BTN_ACB,
                                                           STR_BTN_BAC,
                                                           STR_BTN_BCA,
                                                           STR_BTN_CAB,
                                                           STR_BTN_CBA,
                                                           STR_GO_BACK };

#define ROM_FILE_LIST_NAME_LENGTH 25

#define NUM_ROM_FILE_LIST 8

const char* rom_file_list_header PROGMEM = STR_ROM_SELECTOR_CAPTION;

#define MSG_SAVE_NUM_ROWS 2

const char* message_save[MSG_SAVE_NUM_ROWS] PROGMEM = { STR_SAVING_GAME_STATE,
                                                        STR_PLEASE_WAIT };

#define MSG_ERROR_NUM_ROWS 1

const char* message_error[MSG_ERROR_NUM_ROWS] PROGMEM = { STR_ERROR };


void create_filename(char* s, int num, int ofs) {
  char a;
  for (int h = ofs; h < 32 * 2 + 1; h += 2) {
    a = s[h];
    s[h] = s[h + 1];
    s[h + 1] = a;
  }

  for (char* s2 = s; *s2; ++s2) {
    if (*s2 != ' ')
      *s++ = *s2;
  }
  *s++ = num + 0x30;
  *s = 0;
}

void clearscreen() {
}

#define MENU_BUFFER_HEIGHT 20
#define MENU_BUFFER_WIDTH 320

#define color_bkg RGBColor888_565(0, 10, 25)
#define color_shadow RGBColor888_565(64, 0, 0)
#define color_caption RGBColor888_565(164, 73, 164)
#define color_selected 0xf000
#define color_unselected 0xffff

void message(const char** message, int rows, int delay_ms) {
  glcdBuffer_t* scr_buf = glcdGetBuffer(MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT);

  assert(scr_buf);

  glcdFillRect(0, 0, MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT, color_bkg, scr_buf);

  for (int j = 0; j < 50; j++)
    glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);

  for (int h = 0; h < rows; h++) {
    glcdFillRect(0, 0, MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT, color_bkg, scr_buf);

    font_write_string_centered(SCREEN_WIDTH / 2 + 1, 0 + 1, color_shadow, (char*)message[h], (glcdFont_t*) MENU_FONT, scr_buf);
    font_write_string_centered(SCREEN_WIDTH / 2, 0, color_unselected, (char*)message[h], (glcdFont_t*) MENU_FONT, scr_buf);

    glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], MENU_BUFFER_WIDTH * MENU_BUFFER_HEIGHT);

    for (int j = 0; j < 5; j++)
      glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);
  }

  for (int h = 0; h < 240 - rows * (MENU_BUFFER_HEIGHT + 5) - 50; h++)
    glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);

  delay(50 + delay_ms);

  free((void*)scr_buf);
}

void message2(String m1, String m2, int rows, int delay_ms) {
  glcdBuffer_t* scr_buf = glcdGetBuffer(MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT);

  assert(scr_buf);

  glcdFillRect(0, 0, MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT, color_bkg, scr_buf);

  for (int j = 0; j < 50; j++)
    glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);

  for (int h = 0; h < rows; h++) {
    glcdFillRect(0, 0, MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT, color_bkg, scr_buf);

   #define MSG_MAX_LEN 40 
	
	char msg[MSG_MAX_LEN + 1];

    if (h == 0) {
        m1.toCharArray(msg, (m1.length() + 1) <= MSG_MAX_LEN ? m1.length() + 1 : MSG_MAX_LEN);
		msg[m1.length() <= MSG_MAX_LEN ? m1.length() : MSG_MAX_LEN] = 0;
	} else {
        m2.toCharArray(msg, (m2.length() + 1) <= MSG_MAX_LEN ? m2.length() + 1 : MSG_MAX_LEN);
		msg[m2.length() <= MSG_MAX_LEN ? m2.length() : MSG_MAX_LEN] = 0;
	}

    font_write_string_centered(SCREEN_WIDTH / 2 + 1, 0 + 1, color_shadow, (char*)msg, (glcdFont_t*) MENU_FONT, scr_buf);
    font_write_string_centered(SCREEN_WIDTH / 2, 0, color_unselected, (char*)msg, (glcdFont_t*) MENU_FONT, scr_buf);

    glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], MENU_BUFFER_WIDTH * MENU_BUFFER_HEIGHT);

    for (int j = 0; j < 5; j++)
      glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);
  }

  for (int h = 0; h < 240 - rows * (MENU_BUFFER_HEIGHT + 5) - 50; h++)
    glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);

  delay(50 + delay_ms);

  free((void*)scr_buf);
}

int selection_menu(const char** menu, int menu_items, int sel_delay, int cyclic, int preselect, int backkey) {
  int ret = 0;
  int debounce = 1;
  uint16_t color = 0;
  int8_t selected_item = preselect;

  glcdBuffer_t* scr_buf = glcdGetBuffer(MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT);

  assert(scr_buf);

  while (1) {

    for (int h = 0; h < menu_items; h++) {
      if (h == selected_item)
        color = color_selected;
      else
        color = color_unselected;

      if (h == 0)
        color = color_caption;

      glcdFillRect(0, 0, MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT, color_bkg, scr_buf);

      font_write_string_centered(SCREEN_WIDTH / 2 + 1, 0 + 1, color_shadow, (char*)menu[h], (glcdFont_t*) MENU_FONT, scr_buf);
      font_write_string_centered(SCREEN_WIDTH / 2, 0, color, (char*)menu[h], (glcdFont_t*) MENU_FONT, scr_buf);

      glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], MENU_BUFFER_WIDTH * MENU_BUFFER_HEIGHT);

      glcdFillRect(0, 0, MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT, color_bkg, scr_buf);


      int a = h == 0 ? 15 : 5;

      for (int j = 0; j < a; j++) {
        glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);
      }
    }

    for (int h = 0; h < 240 - menu_items * (MENU_BUFFER_HEIGHT + 5) - 10; h++)
      glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);

    uint16_t dpad = checkDPad();
    uint16_t bts = checkButtons();

    if (debounce == -1) {
      while (bts) {
        bts = checkButtons();
      }
      delay(sel_delay);
    }

    if (debounce > 0)
      debounce++;

    if (debounce > 30)
      debounce = 0;

    if (debounce == 0) {
      if (dpad & DPAD_DOWN) {
        debounce = 1;
        selected_item++;
      }
      if (dpad & DPAD_UP) {
        debounce = 1;
        selected_item--;
      }
      if ((dpad & DPAD_LEFT) && backkey) {
        debounce = 1;
        ret = -3;  // means left
        goto bailout;
      }

      if (selected_item < 1) {
        if (!cyclic) {
          ret = -1;
          goto bailout;
        }

        selected_item = menu_items - 1;
      }

      if (selected_item > menu_items - 1) {
        if (!cyclic) {
          ret = -2;
          goto bailout;
        }

        selected_item = 1;
      }

      if (bts) {
        while (bts) {
          bts = checkButtons();
        }
        delay(10);
        ret = selected_item;

        goto bailout;
      }
    }
  }

bailout:

  free((void*)scr_buf);
  return ret;
}

int calc_rom_checksum(uint8_t* rom_addr) {
  //todo
  return true;
}

int check_rom_valid(uint8_t* rom_addr) {
  char system_id_val1[16] PROGMEM = "SEGA MEGA DRIVE";
  char system_id_val2[13] PROGMEM = "SEGA GENESIS";
  char system_id[16];

  memcpy(system_id, (void*)((uint32_t)rom_addr), 15);
  system_id[15] = 0;

  if (strcmp(system_id_val1, system_id))
    if (calc_rom_checksum(rom_addr))
      return true;

  system_id[12] = 0;

  if (strcmp(system_id_val2, system_id))
    if (calc_rom_checksum(rom_addr))
      return true;

  return false;
}

#define BAR_START_Y 110
#define BAR_END_Y BAR_START_Y + 26
#define BAR_START_X 50
#define BAR_END_X SCREEN_WIDTH - BAR_START_X
#define BAR_BORDER_COLOR color_unselected
#define BAR_COLOR RGBColor888_565(0, 255, 0)
#define BAR_CAPTION_Y 50

void draw_bar(int percent, char* title) {
  glcdBuffer_t* scr_buf = glcdGetBuffer(MENU_BUFFER_WIDTH, MENU_BUFFER_HEIGHT);

  assert(scr_buf);

  if (percent < 0)
    percent = 0;

  if (percent > 100)
    percent = 10;

  int fill = (SCREEN_WIDTH - 2 * BAR_START_X - 8) * percent / 100;

  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    for (int h = 0; h < SCREEN_WIDTH; h++)
      scr_buf[BUF_HEADER_SIZE + h] = color_bkg;

    if (i >= BAR_CAPTION_Y && i < BAR_CAPTION_Y + getFontHeight((glcdFont_t*) MENU_FONT)) {
      font_write_string_centered(SCREEN_WIDTH / 2 + 1, BAR_CAPTION_Y - i + 1, color_shadow, title, (glcdFont_t*) MENU_FONT, scr_buf);
      font_write_string_centered(SCREEN_WIDTH / 2, BAR_CAPTION_Y - i, color_unselected, title, (glcdFont_t*) MENU_FONT, scr_buf);
    }

    if (i >= BAR_START_Y && i < BAR_START_Y + 2)
      for (int h = BAR_START_X; h < BAR_END_X; h++)
        scr_buf[BUF_HEADER_SIZE + h] = BAR_BORDER_COLOR;

    if (i >= BAR_START_Y + 4 && i <= BAR_END_Y - 4)
      for (int h = BAR_START_X + 2; h < BAR_START_X + fill; h++)
        scr_buf[BUF_HEADER_SIZE + h] = BAR_COLOR;

    if (i >= BAR_END_Y && i < BAR_END_Y + 2)
      for (int h = BAR_START_X; h < BAR_END_X; h++)
        scr_buf[BUF_HEADER_SIZE + h] = BAR_BORDER_COLOR;

    if (i >= BAR_START_Y && i < BAR_END_Y + 2) {
      for (int h = BAR_START_X - 2; h < BAR_START_X; h++)
        scr_buf[BUF_HEADER_SIZE + h] = BAR_BORDER_COLOR;

      for (int h = BAR_END_X; h < BAR_END_X + 2; h++)
        scr_buf[BUF_HEADER_SIZE + h] = BAR_BORDER_COLOR;
    }

    glcdSendBufferWord(&scr_buf[BUF_HEADER_SIZE], 320);
  }

  free((void*)scr_buf);
}

int flash_rom(File rom_file, uint8_t* rom_addr) {
  String rom_file_str = rom_file.name();

  long rom_file_size = rom_file.size();
  long bytes_remaining = rom_file_size;
  long bytes_written = 0;

  uint32_t rom_addr_int = ((uint32_t)&rom_addr[0]) - XIP_BASE;

  //  if (PICO_FLASH_SIZE_BYTES - (__flash_binary_end - XIP_BASE) < rom_file_size)
  if (PICO_FLASH_SIZE_BYTES - rom_addr_int < rom_file_size) {
    message2(STR_ERROR, STR_ROM_TOO_BIG, 2, 2000);
    return false;
  }

  int ret = true;
  uint8_t percent = 0;
  uint8_t percent_old = 0;

  long erase_size = rom_file_size;

  if (erase_size % FLASH_SECTOR_SIZE)
    erase_size = (erase_size / FLASH_SECTOR_SIZE + 1) * FLASH_SECTOR_SIZE;

  for (int i = 0; i < erase_size / FLASH_SECTOR_SIZE; i++) {
    flash_range_erase(rom_addr_int + i * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
    //	percent = 100 * (i * FLASH_SECTOR_SIZE) / erase_size;
    percent = 66 * (i * FLASH_SECTOR_SIZE) / erase_size;
    if (percent - percent_old > 5) {
      percent_old = percent;
      draw_bar(percent, STR_LOADING);
    }
  }

  //percent_old = 0;

  while (bytes_remaining > 0) {
    //memset(M68K_RAM, 0, FLASH_SECTOR_SIZE);

    int read_len = rom_file.readBytes((char*)M68K_RAM, bytes_remaining < FLASH_SECTOR_SIZE ? bytes_remaining : FLASH_SECTOR_SIZE);

    // swap hi/lo byte --> emu performance improvement due to different endianess
    for (int i = 0; i < FLASH_SECTOR_SIZE; i += 2) {
      uint8_t tmp = M68K_RAM[i];
      M68K_RAM[i] = M68K_RAM[i + 1];
      M68K_RAM[i + 1] = tmp;
    }

    // erasing flash here does not work...
    //flash_range_erase(rom_addr_int + bytes_written, FLASH_SECTOR_SIZE);
    flash_range_program(rom_addr_int + bytes_written, M68K_RAM, FLASH_SECTOR_SIZE);

    //percent = 100 - 100 * bytes_remaining / rom_file_size;
    percent = 100 - 33 * bytes_remaining / rom_file_size;
    if (percent - percent_old > 5) {
      percent_old = percent;
      draw_bar(percent, STR_LOADING);
    }

    bytes_remaining -= FLASH_SECTOR_SIZE;
    bytes_written += FLASH_SECTOR_SIZE;
  }

  rom_file.close();

#if WRITE_BACK_FILE_TO_SD
  rom_file = SD.open("/cmp.bin", FILE_WRITE);
  bytes_remaining = rom_file_size;
  bytes_written = 0;
  while (bytes_remaining > 0) {
    rom_file.write(&ROM_DATA[bytes_written], FLASH_SECTOR_SIZE < bytes_remaining ? FLASH_SECTOR_SIZE : bytes_remaining);
    bytes_remaining -= FLASH_SECTOR_SIZE;
    bytes_written += FLASH_SECTOR_SIZE;
  }
  rom_file.close();
#endif

  // reset machine
  // let's just hope noone messed with the WD settings...
  watchdog_reboot(0, 0, 1);
  watchdog_enable(1, false);

  return ret;
}


int load_rom() {
  int ret = RM_SUCCESS;
  long ofs = 0;
  int h = 1;
  int eod = 0;  // end of dir
  int preselect = 1;
  File rom_file;
  int sel = 0;

  char* rom_file_list[NUM_ROM_FILE_LIST];

  rom_file_list[0] = (char*)rom_file_list_header;

  for (int i = 1; i < NUM_ROM_FILE_LIST; i++) {
    rom_file_list[i] = (char*)malloc(ROM_FILE_LIST_NAME_LENGTH + 1);
    assert(rom_file_list[i]);
  }

  File rom_dir = SD.open(ROM_DIRECTORY);

  if (!rom_dir.isDirectory()) {
    rom_dir.close();
    message2(STR_ERROR, STR_READ_ROM_DIR, 2, 2000);
    ret = RM_ERR_NO_DIR;
    goto bailout;
  }

  message2(STR_PRESS_LEFT, STR_RETURN, 2, 1000);

  while (sel <= 0) {

    if (sel != -2) {
      if (sel == -1) {
        rom_dir.rewindDirectory();

        ofs--;
        if (ofs < 0)
          ofs = 0;

        for (h = 0; h < ofs; h++) {
          while (1) {
            rom_file = rom_dir.openNextFile();
            if (!rom_file)
              goto allofsfilesread;
            if (!rom_file.isDirectory())
              break;
          }
        }
      }

allofsfilesread:

      for (h = 1; h < NUM_ROM_FILE_LIST; h++) {

        do {
          rom_file = rom_dir.openNextFile();
        } while (rom_file && rom_file.isDirectory());

        if (!rom_file) {
          eod = 1;
          break;
        } else {
          eod = 0;
        }

        String rom_file_str = rom_file.name();

        rom_file_str = rom_file_str.substring(0, ROM_FILE_LIST_NAME_LENGTH);
        rom_file_str.toCharArray(rom_file_list[h], ROM_FILE_LIST_NAME_LENGTH);
      }
    } else {
      do {
        rom_file = rom_dir.openNextFile();
      } while (rom_file && rom_file.isDirectory());

      if (!rom_file) {
        eod = 1;
        goto no_more_roms;
      } else {
        eod = 0;
        ofs++;
      }

      for (h = 1; h < NUM_ROM_FILE_LIST - 1; h++)
        for (int i = 0; i < ROM_FILE_LIST_NAME_LENGTH + 1; i++)
          rom_file_list[h][i] = rom_file_list[h + 1][i];

      h++;  // for selection_menu

      String rom_file_str = rom_file.name();
      rom_file_str = rom_file_str.substring(0, ROM_FILE_LIST_NAME_LENGTH);
      rom_file_str.toCharArray(rom_file_list[NUM_ROM_FILE_LIST - 1], ROM_FILE_LIST_NAME_LENGTH);
    }

    if (h == 1 && eod == 1 && ofs == 0) {
      message2(STR_NO_ROMS, ROM_DIRECTORY, 2, 2000);
      ret = RM_ERR_NO_ROMS;
      goto bailout;
    }

no_more_roms:

    if (sel == -1)
      preselect = 1;

    if (sel == -2)
      preselect = h - 1;

    sel = selection_menu((const char**)rom_file_list, h, SELECTION_DELAY_ROMLIST, 0, preselect, 1);

    if (sel == -3)
      goto bailout;
  }

  message2(STR_FLASHING_ROM, STR_PLEASE_WAIT, 2, 0);

  rom_dir.rewindDirectory();

  for (int i = 0; i < ofs + sel; i++) {
    do {
      rom_file = rom_dir.openNextFile();
    } while (rom_file && rom_file.isDirectory());
    if (!rom_file) {
      message2(STR_ERROR, STR_READING_ROM, 2, 2000);
      ret = RM_ERR_READING_ROM;
      goto bailout;
    }
  }

  if (!flash_rom(rom_file, (uint8_t*)ROM_DATA)) {
    ret = RM_ERR_FLASHING_ROM;
    message2(STR_ERROR, STR_FLASHING_ROM, 2, 2000);
  }

bailout:

  for (int i = 1; i < NUM_ROM_FILE_LIST; i++)
    free((void*)rom_file_list[i]);

  return ret;
}

/* ----------------------- game state save ---------------------- */

int saveGwenesisStateGet(SaveState* state, const char* tagName) {
  int value = 0;
  saveGwenesisStateGetBuffer(state, tagName, &value, sizeof(int));
  return value;
}

void saveGwenesisStateGetBuffer(SaveState* state, const char* tagName, void* buffer, int length) {
  savestate_file.read((uint8_t*)buffer, length);
}

void saveGwenesisStateSet(SaveState* state, const char* tagName, int value) {
  saveGwenesisStateSetBuffer(state, tagName, &value, sizeof(int));
}

void saveGwenesisStateSetBuffer(SaveState* state, const char* tagName, void* buffer, int length) {
  savestate_file.write((uint8_t*)buffer, length);
}

SaveState* saveGwenesisStateOpenForRead(const char* fileName) {
  return (SaveState*)1;
}

SaveState* saveGwenesisStateOpenForWrite(const char* fileName) {
  return (SaveState*)1;
}

bool init_sd_card() {
  SPI.setRX(PIN_SD_MISO);
  SPI.setTX(PIN_SD_MOSI);
  SPI.setSCK(PIN_SD_SCK);

  if (!SD.begin(PIN_SD_CS))
    return false;
  else
    return true;
}

bool savestate(int num) {
  char filename[100] = SAVESTATE_DIRECTORY;
  int a = strlen(filename);

  // concatenate strings without library
  memcpy((void*)(filename + a), (void*)((unsigned long)&ROM_DATA[0] + 0x150), 32);
  filename[a + 31] = '/';
  memcpy((void*)(filename + a + 32 + 0), (void*)((unsigned long)&ROM_DATA[0] + 0x150), 32);
  filename[a + 32 * 2] = 0;

  create_filename(filename, num, a);

  if (SD.exists(filename)) {

    uint8_t sel = selection_menu(confirm_menu_items_overwrite, NUM_CONFIRM_MENU_ITEMS_OVERWRITE, SELECTION_DELAY, 1, 1, 0);

    if (sel != 2)
      return false;

    SD.remove(filename);
  }

  message(message_save, MSG_SAVE_NUM_ROWS, 0);

  SD.mkdir(SAVESTATE_DIRECTORY);

  savestate_file = SD.open(filename, FILE_WRITE);

  if (!savestate_file) {
    message(message_error, MSG_ERROR_NUM_ROWS, 2000);
    return false;
  }

  gwenesis_save_state();

  savestate_file.close();

  return true;
}

bool loadstate(int num) {
  char filename[100] = SAVESTATE_DIRECTORY;
  int a = strlen(filename);

  // concatenate strings without library
  memcpy((void*)(filename + a), (void*)((unsigned long)&ROM_DATA[0] + 0x150), 32);
  filename[a + 31] = '/';
  memcpy((void*)(filename + a + 32 + 0), (void*)((unsigned long)&ROM_DATA[0] + 0x150), 32);
  filename[a + 32 * 2] = 0;

  create_filename(filename, num, a);

  savestate_file = SD.open(filename, FILE_READ);

  if (!savestate_file) {
    message(message_error, MSG_ERROR_NUM_ROWS, 2000);
    return false;
  }

  gwenesis_load_state();

  savestate_file.close();

  return true;
}

/* ----------------------- main function -----------------------*/

void emulator_menu() {
  uint8_t sel = 0;
  uint8_t sel2 = 0;

  while (sel != 7) {
    sel = selection_menu(main_menu_items, NUM_MAIN_MENU_ITEMS, SELECTION_DELAY, 1, 1, 0);

    switch (sel) {
      case 1:
        sel2 = selection_menu(load_menu_items, NUM_LOAD_MENU_ITEMS, SELECTION_DELAY, 1, 1, 0);
        if (sel2 != 7) {
          init_sd_card();
          if (loadstate(sel2))
            return;
        }
        break;
      case 2:
        sel2 = selection_menu(save_menu_items, NUM_SAVE_MENU_ITEMS, SELECTION_DELAY, 1, 1, 0);
        if (sel2 != 7) {
          init_sd_card();
          if (savestate(sel2))
            return;
        }
        break;
      case 3:
        init_sd_card();
        load_rom();
        break;
      case 4:
        sel2 = selection_menu(snd_menu_items, NUM_SND_MENU_ITEMS, SELECTION_DELAY, 1, 1, 0);
        if (sel2 <= 5) {
          if (sel2 != 1) {
            snd_output_volume = sel2 - 1;
            soll_snd = 1;
          } else {
            soll_snd = 0;
          }
        }
        if (sel2 == 6) {
          snd_accurate++;
          if (snd_accurate == 2) {
            snd_accurate = 0;
            message2(STR_SOUND_ACCURACY, STR_LOW, 2, 1000);
          } else {
            message2(STR_SOUND_ACCURACY, STR_HIGH, 2, 1000);
          }
        }
        break;
      case 5:
        sel2 = selection_menu(bri_menu_items, NUM_BRI_MENU_ITEMS, SELECTION_DELAY, 1, 1, 0);
        if (sel2 != 6)
          glcdBacklight(sel2 * 20);
        break;
      case 6:
        sel2 = selection_menu(opt_menu_items, NUM_OPT_MENU_ITEMS, SELECTION_DELAY, 1, 1, 0);
		if (sel2 == 1) {
		  int sel3 = selection_menu(btn_menu_items, NUM_BTN_MENU_ITEMS, SELECTION_DELAY, 1, 1, 0);
		  // TODO
		}
        if (sel2 == 2) {
          enable_debug_display++;
          if (enable_debug_display == 2) {
            enable_debug_display = 0;
            message2(STR_FPS, STR_DISABLED, 2, 1000);
          } else {
            message2(STR_FPS, STR_ENABLED, 2, 1000);
          }
        }
		if (sel2 == 3) {
			uint8_t sel3 = selection_menu(confirm_menu_items_bl, NUM_CONFIRM_MENU_ITEMS_BL, SELECTION_DELAY, 1, 1, 0);
			if (sel3 == 2) 
               reset_usb_boot(0, 0);
        }
		break;
      default:
        break;
    }
  }
}
