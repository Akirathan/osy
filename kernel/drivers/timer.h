/**
 * @file timer.h
 *
 * Processor timer support.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef DTIMER_H_
#define DTIMER_H_

#include <include/c.h>


/** Setup timer
 *
 * Setup the timer interrupt by setting the value of the
 * Compare register, which will trigger an interrupt when the
 * Count register reaches the value.
 *
 * @param quantum Number of ticks after which the interrupt
 *                is triggered.
 *
 */
static inline void timer_setup (unative_t quantum)
{
	write_cp0_compare (read_cp0_count () + quantum);
}


/** Get current timestamp
 *
 * @return Current timestamp in ticks.
 *
 */
static inline unative_t timer_get (void)
{
	return read_cp0_count ();
}


#endif
