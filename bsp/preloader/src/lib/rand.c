//===========================================================================
//
//      rand.c
//
//      ISO and POSIX 1003.1 standard random number generation functions
//
//===========================================================================

#include "x_rand.h"
#include "targetConfig.h"
static unsigned int s_rand_seed = 0;
static RAND_ALGORITHM s_algo = RAND_KNUTH;




// FUNCTIONS
#if(IS_FOR_LITTLE_SIZE==0)
unsigned int rand(void)
{
	return rand_r(&s_rand_seed);
} // rand()


unsigned int rand_r(unsigned int *seed)
{
	unsigned int s = 0, t;
	unsigned int uret;
	int retval = 0;

	switch (s_algo)
	{

	case RAND_SIMPLEST:
		// This algorithm sucks in the lower bits
		*seed = (*seed * 1103515245) + 12345; // permutate seed
		retval = (unsigned int)( *seed & X_RAND_MAX );
		break;


	case RAND_SIMPLE:
		// The above algorithm sucks in the lower bits, so we shave them off
		// and repeat a couple of times to make it up

		s = (s * 1103515245) + 12345; // permutate seed
		// Only use top 11 bits
		uret = s & 0xffe00000;

		s = (s * 1103515245) + 12345; // permutate seed
		// Only use top 14 bits
		uret += (s & 0xfffc0000) >> 11;

		s = (s * 1103515245) + 12345; // permutate seed
		// Only use top 7 bits
		uret += (s & 0xfe000000) >> (11+14);

		retval = (unsigned int)(uret & X_RAND_MAX);
		*seed = s;
		break;

	case RAND_KNUTH:
		// This is the code supplied in Knuth Vol 2 section 3.6 p.185 bottom

#define MM 2147483647    // a Mersenne prime
#define AA 48271         // this does well in the spectral test
#define QQ 44488         // (long)(MM/AA)
#define RR 3399          // MM % AA; it is important that RR<QQ

		s = AA * (*seed % QQ);
		t = RR * (unsigned int)(*seed / QQ);
		if (s > t)
			*seed = s - t;
		else
			*seed = MM+ s - t;

		retval = (unsigned int)(*seed & X_RAND_MAX);
		break;

	default:
		// No valid algorithm selected
//		ASSERT(0);
		break;
	}

    return retval;
} // rand_r()



void srand(unsigned int seed)
{
    s_rand_seed = seed;
} // srand()



void rand_set_algorithm(RAND_ALGORITHM algo)
{
	s_algo = algo;
}



unsigned int random(unsigned int mod)
{
	return rand() % mod;
}
#endif
