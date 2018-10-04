/**
 * @file time.h
 *
 * Time handling routines.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef TIME_H_
#define TIME_H_


#include <include/shared.h>
#include <include/c.h>


/** Period of the system timer.
 *
 * This is just a hard-wired constant that
 * does not reflect the actual wall-clock time.
 *
 */
#define TIMER_PERIOD  1000


/** Convert relative time from seconds to jiffies
 *
 */
static inline unsigned int sec_to_jiffies (unsigned int sec)
{
	return sec * TIMER_PERIOD;
}


/** Convert relative time from microseconds to jiffies
 *
 */
static inline unsigned int usec_to_jiffies (unsigned int usec)
{
	return (usec * TIMER_PERIOD + 999999) / 1000000;
}


/** Convert relative time from jiffies to seconds
 *
 * Always rounds up towards the higher value.
 *
 */
static inline unsigned int jiffies_to_sec (unsigned int jiffies)
{
	return (jiffies + TIMER_PERIOD - 1) / TIMER_PERIOD;
}


/** Convert relative time from jiffies to microseconds
 *
 * Always rounds up towards the higher value.
 *
 */
static inline unsigned int jiffies_to_usec (unsigned int jiffies)
{
	return (jiffies * 1000000 + TIMER_PERIOD - 1) / TIMER_PERIOD;
}


#endif /* TIME_H_ */
