/**
 * @file timer.c
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

#include <include/shared.h>
#include <include/c.h>

#include <proc/thread.h>
#include <sched/sched.h>

#include <time/timer.h>


/** List of timers */
static list_t timers_list;

/** Timer thread */
static thread_t timer_thread;


/** Check if timer is pending.
 *
 */
static int timer_pending (struct timer *timer)
{
	return link_connected (&timer->link);
}


/** Remove timer from the list of timers
 *
 */
static void timer_remove (struct timer *timer)
{
	list_remove (&timer->link);
}


/** Process expired timers
 *
 * If there are any expired timers, they are executed within
 * the context of this thread.
 *
 * TODO: The performance of this code is really poor. This could
 * be improved by sorting the list of timers according to the
 * expiration time.
 *
 */
static void *timer_thread_func (void *data)
{
	while (true) {
		/*
		 * Handle expired timers.
		 */
		
		struct timer *expired_timer;
		timer_fn handler;
		void *data;
		
		do {
			expired_timer = NULL;
			handler = NULL;
			data = NULL;
			
			ipl_t status = query_and_disable_interrupts ();
			
			list_foreach (timers_list, struct timer, link, timer) {
				if (timer->expires < jiffies) {
					expired_timer = timer;
					break;
				}
			}
			
			if (expired_timer != NULL) {
				timer_remove (expired_timer);
				expired_timer->running = true;
				handler = expired_timer->handler;
				data = expired_timer->data;
			}
			
			conditionally_enable_interrupts (status);
			
			/* Run the timer */
			if (handler != NULL)
				handler (expired_timer, data);
			
			status = query_and_disable_interrupts ();
			
			if (expired_timer != NULL)
				expired_timer->running = false;
			
			conditionally_enable_interrupts (status);
		} while (expired_timer != NULL);
		
		thread_suspend ();
	}
	
	return NULL;
}

/** Initialize the timers framework
 *
 */
int timers_init (void)
{
	/* Initialize the list of schedulable threads. */
	list_init (&timers_list);
	
	/* Create the timer thread. */
	return thread_create (&timer_thread, timer_thread_func, NULL, 0);
}


/** Check for expired timers
 *
 * If there are any expired timers, the timer thread is woken up
 * to service them.
 *
 * This function is called from sched_timer() with disabled
 * interrupts. Therefore it is safe to access the timer list.
 *
 * TODO: The performance of this code is really poor. This could
 * be improved by sorting the list of timers according to the
 * expiration time.
 *
 */
void timers_run (void)
{
	list_foreach (timers_list, struct timer, link, timer) {
		if (timer->expires < jiffies) {
			thread_wakeup (timer_thread);
			break;
		}
	}
}


/** Initialize timer
 *
 * Initialize the timer control structure with a timeout specified
 * in jiffies.
 *
 * @param @timer  Timer to initialize.
 * @param timeout Timeout in jiffies.
 * @param handle  Timer handler function called at expiration.
 * @param data    Data to be passed to the handler.
 *
 */
void timer_init_jiffies (struct timer *timer, unsigned int timeout,
    timer_fn handler, void *data)
{
	link_init (&timer->link);
	timer->expires = 0;
	timer->timeout = timeout;
	timer->running = false;
	timer->handler = handler;
	timer->data = data;
}


/** Activate timer
 *
 * Add the timer into the list of pending timers.
 *
 * @param timer Timer to activate.
 *
 */
void timer_start (struct timer *timer)
{
	ipl_t status = query_and_disable_interrupts ();
	
	timer->expires = jiffies + timer->timeout;
	list_append (&timers_list, &timer->link);
	
	conditionally_enable_interrupts (status);
}


/** Destroy a timer.
 *
 * Delete the timer from the list of timers and clean up the timer
 * control structure. If the timer handler is running, wait for it to
 * complete.
 *
 * @param timer Timer to destroy.
 *
 */
void timer_destroy (struct timer *timer)
{
	ipl_t status = query_and_disable_interrupts ();
	
	do {
		/*
		 * If the timer is pending, remove it.
		 */
		if (timer_pending (timer))
			timer_remove (timer);
		
		/*
		* Wait for the handler to finish.
		*/
		while (timer->running)
			thread_yield ();
	} while (timer_pending (timer));
	
	conditionally_enable_interrupts (status);
}
