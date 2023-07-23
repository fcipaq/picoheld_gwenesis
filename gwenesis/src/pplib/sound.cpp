#include "sound.h"
#include "setup.h"
#include "typedefs.h"

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "hardware/dma.h"
#include "snd_pio.pio.h"

/* ----------------------------- PIO -----------------------------*/
// Community RP2040 board package by Earle Philhower
PIO snd_pio = pio1;
int8_t snd_pio_sm = 0;

uint32_t snd_pio_program_offset  = 0;

uint32_t snd_pio_pull_stall_mask = 0;

/* ----------------------------- DMA -----------------------------*/
int32_t  snd_dma_tx_channel;
dma_channel_config snd_dma_tx_config;

uint8_t snd_dma_enabled = 0;

/* --------------------------- sound tx ---------------------------*/
volatile uint8_t* __playing = NULL;
uint32_t __playing_len = 0;
uint32_t __playing_pos = 0;

uint8_t* snd_buffer[SND_NUM_BUFS];

// No of times null_snd_buffer will be scanned out before the machine is shut down
// undefine to disable auto quiesce (not recommended)
// This section is for means against click and pop noise.
// If a buffer runs empty, a null-buffer will be sent in order to prevent noise
// However if after AUTO_QUIESECE_CNT times no new buffer is submitted, the machine
// will be automatically muted.
uint8_t null_snd_buffer[NULL_SND_BUFFER_SIZE];
volatile uint8_t quiesce_machine = 1;  // start in quiet state
volatile uint16_t quiesce_cnt = 0;

volatile int8_t irq_by_null_snd = 0;
volatile uint8_t* cur_snd_buffer;
volatile uint8_t num_cur_snd_buffer;
volatile uint8_t num_nxt_snd_buffer;

bool buf_pgm = false;
uint8_t play_mode = PLAY_NONE;

/* ------------------------- mutex and semaphores -------------------------*/

mutex_t snd_busy;
mutex_t buf_unsent;
semaphore_t num_free_bufs;

void prepNxtBuf() {
  uint16_t* tx_len = (uint16_t*) &cur_snd_buffer[0];

  *tx_len = SND_BUFFER_SAMPLES < (__playing_len - __playing_pos) ? SND_BUFFER_SAMPLES : (__playing_len - __playing_pos);

  for (uint16_t h = 0; h < *tx_len; h++) {
    if (buf_pgm == SRC_PGM)
      cur_snd_buffer[h + SND_BUFFER_HEADER_SIZE] = pgm_read_byte((uint8_t*) &__playing[__playing_pos + h]);
    else
      cur_snd_buffer[h + SND_BUFFER_HEADER_SIZE] = __playing[__playing_pos + h];
  }

  __playing_pos += *tx_len;

  // turn off the PWM when finished playing
  if (__playing_pos == __playing_len)
    cur_snd_buffer[SND_BUFFER_HEADER_SIZE + *tx_len - 1] = 0;
}

void txNullSound() {
  if (dma_channel_is_busy(snd_dma_tx_channel) || quiesce_machine)
    return;
  
  #ifdef AUTO_QUIESECE_CNT
    quiesce_cnt++;
    if (quiesce_cnt == AUTO_QUIESECE_CNT) {
      quiesce_cnt = 0;
      quiesce_machine = 1;
    }
  #endif

  dma_channel_configure(snd_dma_tx_channel, &snd_dma_tx_config, &snd_pio->txf[snd_pio_sm], (uint8_t*) &null_snd_buffer[0], NULL_SND_BUFFER_SIZE, true);
  
  irq_by_null_snd = 1;
}

void txSound() {
  quiesce_machine = 0;

  if (dma_channel_is_busy(snd_dma_tx_channel))
    return;
  
  irq_by_null_snd = 0;

  uint16_t* tx_len = (uint16_t*) &cur_snd_buffer[0];

  dma_channel_configure(snd_dma_tx_channel, &snd_dma_tx_config, &snd_pio->txf[snd_pio_sm], (uint8_t*) &cur_snd_buffer[SND_BUFFER_HEADER_SIZE], *tx_len, true);

  // cycle through available buffers
  num_cur_snd_buffer++;
  
  if (num_cur_snd_buffer == SND_NUM_BUFS)
   num_cur_snd_buffer = 0;
  
  cur_snd_buffer = snd_buffer[num_cur_snd_buffer];

  //  auto prepare next buffer in case a PCM file is played
  if (play_mode == PLAY_WAV)
    prepNxtBuf();
}

void __isr sndDmaComplete() {
  if (!dma_channel_get_irq1_status(snd_dma_tx_channel))
    return;

  // clear irq flag
  dma_channel_acknowledge_irq1(snd_dma_tx_channel);

  if (play_mode == PLAY_BUF) {
    // deque buffer if it's txSound why we're here
    if (!irq_by_null_snd)
      sem_release(&num_free_bufs);
    
    // only call txSound if there are more buffer available
    if (sem_available(&num_free_bufs) != SND_NUM_BUFS) {
      txSound();
      return;
    } else
      play_mode = PLAY_NONE;  // no more buffers to play, stopping
  }

  txNullSound();  // prevent click and pop noise

  if (play_mode == PLAY_WAV) {
    txSound();
    if (__playing_pos == __playing_len) {
      __playing = NULL;
      play_mode = PLAY_NONE;
      mutex_exit(&snd_busy);
      irq_set_enabled(DMA_IRQ_1, false);
    }
    return;
  }

}

bool sndDmaInit() {
  if (snd_dma_enabled) return false;

  snd_dma_tx_channel = dma_claim_unused_channel(false);

  if (snd_dma_tx_channel < 0) return false;

  snd_dma_tx_config = dma_channel_get_default_config(snd_dma_tx_channel);

  channel_config_set_transfer_data_size(&snd_dma_tx_config, DMA_SIZE_8);

  channel_config_set_dreq(&snd_dma_tx_config, pio_get_dreq(snd_pio, snd_pio_sm, true));

  dma_channel_set_irq1_enabled(snd_dma_tx_channel, true);

  irq_set_exclusive_handler(DMA_IRQ_1, sndDmaComplete);

  snd_dma_enabled = true;

  return true;
}

void sndDmaShutdown(void) {
  if (!snd_dma_enabled)
    return;

  dma_channel_unclaim(snd_dma_tx_channel);

  snd_dma_enabled = false;
}

void sndSetSpeed(uint16_t clock_div, uint16_t fract_div) {
  pio_sm_set_clkdiv_int_frac(snd_pio, snd_pio_sm, clock_div, fract_div);
}

bool sndPioInit() {
  snd_pio_sm = pio_claim_unused_sm(snd_pio, false);
  
  if (snd_pio_sm < 0)
    return false;

  snd_pio_program_offset = pio_add_program(snd_pio, &snd_output_program);

  pio_gpio_init(snd_pio, SND_PIN);

  pio_sm_set_consecutive_pindirs(snd_pio, snd_pio_sm, SND_PIN, 1, true);

  pio_sm_config c = snd_output_program_get_default_config(snd_pio_program_offset);
  sm_config_set_sideset_pins(&c, SND_PIN);
  pio_sm_init(snd_pio, snd_pio_sm, snd_pio_program_offset, &c);

  pio_sm_put_blocking(snd_pio, snd_pio_sm, 256);
  pio_sm_exec(snd_pio, snd_pio_sm, pio_encode_pull(false, false));
  pio_sm_exec(snd_pio, snd_pio_sm, pio_encode_out(pio_isr, 32));

  pio_sm_set_enabled(snd_pio, snd_pio_sm, true);

  snd_pio_pull_stall_mask = 1u << (PIO_FDEBUG_TXSTALL_LSB + snd_pio_sm);

  return true;
} //pioInit

/*
 * Enque a buffer to be played
 * A buffer will be dequed and assigned to played. If there is currently no
 * free buffer then the function will either wait for a free buffer (BLOCKING)
 * or return false (NONBLOCKING)
 * In BLOCKING mode "false" will be returned in case a free buffer had to be
 * waited for.
 * In NONBLOCKING mode "false" will be returned in case there was no free buffer
 */
int8_t enqueSndBuf(uint8_t *ext_buf, uint32_t buffersize, bool blocking, bool pgm) {
  uint8_t* buf;
  uint8_t ret;

  if (play_mode == PLAY_WAV)
    return -1;

  ret = sem_available(&num_free_bufs);

  // request a buffer
  if (!sem_try_acquire(&num_free_bufs)) {
    if (blocking)
      // wait for a free buffer in blocking mode
      sem_acquire_blocking(&num_free_bufs);
    else
      // leave in non-blocking mode (not waiting for a buffer)
      return ret;
  }
  
  num_nxt_snd_buffer++;

  if (num_nxt_snd_buffer == SND_NUM_BUFS)
   num_nxt_snd_buffer = 0;

  buf = snd_buffer[num_nxt_snd_buffer];
  
  // prepare buffer size for copying
  if (buffersize > SND_BUFFER_SAMPLES)
    buffersize = SND_BUFFER_SAMPLES;

  if (!pgm) {
    for (int h = 0; h < buffersize; h++)
      buf[h + SND_BUFFER_HEADER_SIZE] = ext_buf[h];
  } else {
    #if 1
    for (int h = 0; h < buffersize; h++)
      buf[h + SND_BUFFER_HEADER_SIZE] = pgm_read_byte((uint8_t*) &ext_buf[h]);
    #else
    for (int h = 0; h < buffersize - 1; h++)
      buf[h + SND_BUFFER_HEADER_SIZE] = (pgm_read_byte((uint8_t*) &ext_buf[h]) + pgm_read_byte((uint8_t*) &ext_buf[h + 1])) / 2;
      buf[buffersize - 1 + SND_BUFFER_HEADER_SIZE] =pgm_read_byte((uint8_t*) &ext_buf[buffersize - 1]);
    #endif
  }

  uint16_t* tx_len = (uint16_t*) &buf[0];
  *tx_len = buffersize;

  play_mode = PLAY_BUF;

  // if there is only one buffer in use (the one that
  // has just been dequed) then that means
  // there is currently no ongoin transmission and
  // therefore the transmission must be reinitiated
  if (sem_available(&num_free_bufs) == (SND_NUM_BUFS - 1)) {
    num_cur_snd_buffer = num_nxt_snd_buffer;
    cur_snd_buffer = snd_buffer[num_cur_snd_buffer];
    irq_set_enabled(DMA_IRQ_1, true);
    txSound();
  }
  
  return ret;
}

// send a buffer of data bytes
bool sendWavFile(uint8_t *buf, uint32_t buffersize, bool blocking, bool pgm) {
  if (play_mode == PLAY_BUF)
    return false;

  #if 0
  for (uint32_t h = 0; h < buffersize; h++)
    pio_sm_put_blocking(snd_pio, snd_pio_sm, pgm_read_byte((uint8_t*) &buf[h]));
    pio_sm_put_blocking(snd_pio, snd_pio_sm, 0);
  return true;
  #endif
  
  if ((buffersize == 0) || (!snd_dma_enabled))
    return false;

  if (blocking) {
    mutex_enter_blocking(&snd_busy);
  } else {
    if (!mutex_try_enter(&snd_busy, NULL))
      return false;
  }
  
  // Wait for DMA to be ready
  while (dma_channel_is_busy(snd_dma_tx_channel));

  buf_pgm = pgm;
  __playing = buf;
  __playing_len = buffersize;
  __playing_pos = 0;
  
  prepNxtBuf();

  // turn on IRQ and transmit fist data packet
  irq_set_enabled(DMA_IRQ_1, true);
  
  play_mode = PLAY_WAV;
  
  txSound();

  return true;
}

bool initSound()
{
  // 3 is slowest
  // 1 is fastest
  if (!sndPioInit())
    return false;

  if (!sndDmaInit())
    return false;

  // setup mutexes and semaphores
  mutex_init(&snd_busy);
  mutex_init(&buf_unsent);
  
  sem_init(&num_free_bufs, SND_NUM_BUFS, SND_NUM_BUFS);

  sndSetSpeed(3, 0);

  for (int h = 0; h < SND_NUM_BUFS; h++)
    snd_buffer[h] = (uint8_t*) malloc(SND_BUFFER_SIZE);

  for (int h = 0; h < NULL_SND_BUFFER_SIZE; h++)
    null_snd_buffer[h] = 128;

  num_cur_snd_buffer = 0;
  cur_snd_buffer = snd_buffer[num_cur_snd_buffer];
  
  return true;
}
