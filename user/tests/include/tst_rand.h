/**
 * Trivial random number generator
 *
 * Change Log:
 * 03/11/18 ghort  created
 */

#ifndef _TST_RAND_H_
#define _TST_RAND_H_

/**
 * rand
 *
 * A trivial random number generator.
 * Returns a random unsigned long number.
 */
inline static unsigned long tst_rand (void)
{
	static unsigned long random_seed = 12345678;
	
	random_seed = (random_seed * 873511) % 22348977 + 7;
	return (random_seed >> 8);
} /* rand */

#endif /* _TST_RAND_H_ */
