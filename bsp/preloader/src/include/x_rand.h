//===========================================================================
//
//      x_rand.h
//
//      ISO and POSIX 1003.1 standard random number generation functions
//
//===========================================================================

#ifndef X_RAND_H
#define X_RAND_H

#define x_srand srand
#define x_rand  rand

// Maximum value returned by rand()
#define X_RAND_MAX  2147483647

#include "targetConfig.h"
//=====================================================================
// Algorithm for random number generation. Default is RAND_KNUTH

typedef enum
{
	RAND_SIMPLEST,
	RAND_SIMPLE,
	RAND_KNUTH
} RAND_ALGORITHM;



//=====================================================================
// Function prototypes

void rand_set_algorithm(RAND_ALGORITHM algo);
void srand(unsigned int seed);
unsigned int rand_r(unsigned int* seed_p);
unsigned int rand(void);
unsigned int random(unsigned int mod);
#if(IS_FOR_LITTLE_SIZE==0)
extern unsigned long rand_pattern[1000];
#endif
#endif	// X_RAND_H
