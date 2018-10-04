/**
 * @file timer.h
 *
 * Timers.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef TIMER_H_
#define TIMER_H_


#include <include/shared.h>
#include <include/c.h>

#include <adt/list.h>


/** Timer callback function.
 *
 */
struct timer;

typedef void (* timer_fn) (struct timer *timer, void *data);


/** Timer control structure.
 *
 */
struct timer {
	/** A timer can be an item on a list */
	link_t link;
	
	/** Relative expiration time */
	unsigned int timeout;
	
	/** Absolute expiration time */
	unsigned int expires;
	
	/** Timer handler is running */
	bool running;
	
	/** Callback handler */
	timer_fn handler;
	
	/** Callback data */
	void *data;
};


/* Externals are commented with implementation */
extern int timers_init (void);
extern void timers_run (void);
extern void timer_init_jiffies (struct timer *timer, unsigned int timeout,
    timer_fn handler, void *data);
extern void timer_start (struct timer *timer);
extern void timer_destroy (struct timer *timer);


#endif /* TIMER_H_ */
