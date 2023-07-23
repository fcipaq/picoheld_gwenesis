#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>

// buffer parameters
#define SND_NUM_BUFS 3
#define SND_BUFFER_HEADER_SIZE 2  //2 bytes for data size len
#define SND_BUFFER_SAMPLES 2000  // calculate time for buffer as follows: SND_BUFFER_SAMPLES * 23 us
#define SND_BUFFER_SIZE SND_BUFFER_SAMPLES + SND_BUFFER_HEADER_SIZE

// click and pop prevention
#define NULL_SND_BUFFER_SIZE 128
#define AUTO_QUIESECE_CNT 100

// play modes
#define PLAY_NONE  0
#define PLAY_BUF   1
#define PLAY_WAV   2

// function parameters
#define NONBLOCKING 0
#define BLOCKING 1

bool initSound();
int8_t enqueSndBuf(uint8_t *ext_buf, uint32_t buffersize, bool blocking, bool pgm);
bool sendWavFile(uint8_t *buf, uint32_t buffersize, bool blocking, bool pgm_mem);
void sndSetSpeed(uint16_t clock_div, uint16_t fract_div);
void putPIO(uint8_t b);

#endif //SOUND_H
