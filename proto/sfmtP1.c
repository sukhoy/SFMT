/* SFMT Search Code, M.Saito 2006/2/28 */
/* (1234) 巡回置換１回のみ */
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "sfmt.h"

#ifndef MEXP
#define MEXP 19937
#endif

#define WORDSIZE 128
#define NN (MEXP / WORDSIZE + 1)
#define N (NN -1)
#define MAXDEGREE (WORDSIZE * NN)

static uint32_t sfmt[N][4];
static uint32_t lung[4];
static unsigned int idx;

static unsigned int POS1 = 1;
static unsigned int SL1 = 11;
static unsigned int SL2 = 11;
static unsigned int SR1 = 7;
static unsigned int SR2 = 7;
static unsigned int MSK1 = 7;
static unsigned int MSK2 = 7;
static unsigned int MSK3 = 7;
static unsigned int MSK4 = 7;
//static unsigned int SL7 = 7;
//static unsigned int SL8 = 7;
//static unsigned int SR1 = 17;
//static unsigned int SR2 = 9;
//static unsigned int SR3 = 9;
//static unsigned int SR4 = 9;

static void gen_rand_all(void);

unsigned int get_rnd_maxdegree(void)
{
    return MAXDEGREE;
}

unsigned int get_rnd_mexp(void)
{
    return MEXP;
}

void setup_param(unsigned int p1, unsigned int p2, unsigned int p3, 
		 unsigned int p4, unsigned int p5, unsigned int p6,
		 unsigned int p7, unsigned int p8, unsigned int p9,
		 unsigned int p10, unsigned int p11, unsigned int p12,
		 unsigned int p13) {
    POS1 = p1 % (N-1) + 1;
    SL1 = p2 % (32 - 1) + 1;
    SL2 = (p13 % 4) * 2 + 1; 
    //SL3 = p12 % (32 - 1) + 1;
    //SR1 = p3 % (32 - 1) + 1;
    SR1 = p12 % (32 - 1) + 1;
    SR2 = (p3 % 4) * 2 + 1;
    MSK1= p4 | p5 | p6;
    MSK2= p6 | p7 | p8;
    MSK3= p8 | p9 | p10;
    MSK4= p10 | p11 | p12;
    //SL7 = p8 % (32 - 1) + 1;
    //SL8 = p9 % (32 - 1) + 1;
    //SR1 = p10 % (32 - 1) + 1;
    //SR2 = p11 % (32 - 1) + 1;
    //SR3 = p12 % (32 - 1) + 1;
    //SR4 = p13 % (32 - 1) + 1;
    memset(sfmt, 0, sizeof(sfmt));
}

void print_param(FILE *fp) {
    fprintf(fp, "POS1 = %u\n", POS1);
    fprintf(fp, "SL1 = %u\n", SL1);
    fprintf(fp, "SL2 = %u\n", SL2);
    fprintf(fp, "SR1 = %u\n", SR1);
    fprintf(fp, "SR2 = %u\n", SR2);
    fprintf(fp, "MSK1 = %08x\n", MSK1);
    fprintf(fp, "MSK2 = %08x\n", MSK2);
    fprintf(fp, "MSK3 = %08x\n", MSK3);
    fprintf(fp, "MSK4 = %08x\n", MSK4);
    //fprintf(fp, "SL7 = %u\n", SL7);
    //fprintf(fp, "SL8 = %u\n", SL8);
    //fprintf(fp, "SR1 = %u\n", SR1);
    //fprintf(fp, "SR2 = %u\n", SR2);
    //fprintf(fp, "SR3 = %u\n", SR3);
    //fprintf(fp, "SR4 = %u\n", SR4);
    fflush(fp);
}

void print_param2(FILE *fp) {
    fprintf(fp, "[POS1, SL1, SL2, SL3, SL4, SL5, SL6, SL7, SL8,"
	    " SR1, SR2, SR3, SR4] = "
	    "[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]\n", 
	    POS1, SL1, SR1, MSK1, MSK2, MSK3, MSK4, 0, 0, 
	    0, 0, 0, 0);
    fflush(fp);
}


inline static void rshift128(uint32_t out[4], const uint32_t in[4],
			     int shift) {
    uint64_t th, tl, oh, ol;

    th = ((uint64_t)in[3] << 32) | ((uint64_t)in[2]);
    tl = ((uint64_t)in[1] << 32) | ((uint64_t)in[0]);

    oh = th >> (shift * 8);
    ol = tl >> (shift * 8);
    ol |= th << (64 - shift * 8);
    out[1] = (uint32_t)(ol >> 32);
    out[0] = (uint32_t)ol;
    out[3] = (uint32_t)(oh >> 32);
    out[2] = (uint32_t)oh;
}

inline static void lshift128(uint32_t out[4], const uint32_t in[4],
			     int shift) {
    uint64_t th, tl, oh, ol;

    th = ((uint64_t)in[3] << 32) | ((uint64_t)in[2]);
    tl = ((uint64_t)in[1] << 32) | ((uint64_t)in[0]);

    oh = th << (shift * 8);
    ol = tl << (shift * 8);
    oh |= tl >> (64 - shift * 8);
    out[1] = (uint32_t)(ol >> 32);
    out[0] = (uint32_t)ol;
    out[3] = (uint32_t)(oh >> 32);
    out[2] = (uint32_t)oh;
}

static inline void do_recursion(uint32_t a[4], uint32_t b[4], uint32_t c[4]) {
    uint32_t x[4];

    lung[0] ^= a[0];
    lung[1] ^= a[1];
    lung[2] ^= a[2];
    lung[3] ^= a[3];
    lshift128(x, lung, SL2);
    a[0] = lung[0] ^ x[0] ^ ((b[0] >> SR1) & MSK1) ^ (c[0] << SL1);
    a[1] = lung[1] ^ x[1] ^ ((b[1] >> SR1) & MSK2) ^ (c[1] << SL1);
    a[2] = lung[2] ^ x[2] ^ ((b[2] >> SR1) & MSK3) ^ (c[2] << SL1);
    a[3] = lung[3] ^ x[3] ^ ((b[3] >> SR1) & MSK4) ^ (c[3] << SL1);
}

static void gen_rand_all(void) {
    int i;

    for (i = 0; i < N; i++) {
	do_recursion(sfmt[i], sfmt[(i + POS1) % N], sfmt[(i + N - 1) % N]);
    }
}

/* for 128 bit check */
uint32_t gen_rand(void)
{
    uint32_t r;

    if (idx >= N) {
	gen_rand_all();
	idx = 0;
    }
    r = sfmt[idx][0];
    idx++;
    return r;
}

/* for 128 bit check */
void init_gen_rand(uint32_t seed)
{
    int i;

    sfmt[0][0] = seed;
    for (i = 1; i < N * 4; i++) {
	sfmt[i/4][i%4] = 1812433253UL 
	    * (sfmt[(i - 1) / 4][(i - 1) % 4]
	       ^ (sfmt[(i - 1) / 4][(i - 1) % 4] >> 30)) 
	    + i;
    }
    lung[0] = 1812433253UL 
	    * (sfmt[(i - 1) / 4][(i - 1) % 4]
	       ^ (sfmt[(i - 1) / 4][(i - 1) % 4] >> 30)) 
	    + i;
    lung[1] = 1812433253UL * (lung[0] ^ (lung[0] >> 30)) + 1;
    lung[2] = 1812433253UL * (lung[1] ^ (lung[1] >> 30)) + 2;
    lung[3] = 1812433253UL * (lung[2] ^ (lung[2] >> 30)) + 3;
    idx = 0;
}
