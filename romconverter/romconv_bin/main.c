/*
 * Rom converter for Gwenesis for the Pico Held Handheld
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

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t getvalue(uint32_t* b, int n) {
  uint32_t tmp;
  memcpy(&tmp, b, n);
  return tmp;
}

int main(int argc, char** argv) {
  printf("Byte swapper (swaps the bytes in a 16 bit word).\n");
  printf("Swaps ROM bytes for the Pico Held Sega Genesis emulator.\n\n");
  printf("This tools creates a BINARY file.\n\n");

  if (argc != 3) {
    printf("Please state one input and one output file.\n");
    return -1;
  }

  if (sizeof(uint32_t) != 4 || sizeof(uint16_t) != 2) {
    printf("Type size incorrect. Please recompile.\n");
    return -1;
  }

  FILE* f_in = fopen(argv[1], "rb");
  
  if (f_in == NULL) {
    printf("*** Error opening input file.\n\n");
    return -1;
  }
  
  fseek(f_in, 0L, SEEK_END);
  uint32_t sz = ftell(f_in);
  rewind(f_in);

  FILE* f_out = fopen(argv[2], "wb");

  if (f_out == NULL) {
    fclose(f_in);
    printf("*** Error opening output file.\n\n");
    return -1;
  }
  
  uint8_t b, c;

  for (int h = 0; h < sz; h += 2) {
    b = fgetc(f_in);
    c = fgetc(f_in);
    fprintf(f_out, "%c", c);
    fprintf(f_out, "%c", b);
  }

  printf("done.\n\n");

  fclose(f_in);
  fclose(f_out);

  return 0;
}
