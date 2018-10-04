/**
 * @file sched.c
 *
 * Round-robin kernel thread scheduler.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <include/shared.h>
#include <include/c.h>

#include <adt/list.h>
#include <proc/thread.h>
#include <drivers/dorder.h>
#include <drivers/timer.h>
#include <time/timer.h>

#include <sched/sched.h>

/** Synchronize the startup of CPUs */
ATOMIC_DECLARE(cpu_ready, 0);

/** Kernel jiffies */
volatile unsigned int jiffies = 0;

/** Number of ticks a thread is allowed to run */
#define THREAD_QUANTUM  4000

/** List of schedulable threads for each CPU */
static list_t runnable_list[MAX_CPU];


/** Scheduler initialization
 *
 * Initializes the scheduler structures and configures the scheduler interrupt.
 *
 */
void scheduler_init (void)
{
	/* Initialize the list of schedulable threads. */
	list_init (&runnable_list[cpuid()]);
	
	/*
	 * Configure the scheduler interrupt. A cleaner way would be
	 * moving this code to the timer framework.
	 */
	timer_setup (THREAD_QUANTUM);
}


/** Include thread in scheduling
 *
 * The thread is appended to the list of schedulable threads.
 *
 */
void sched_insert (thread_t thread)
{
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	list_append (&runnable_list[cpuid()], &thread->link);
	conditionally_enable_interrupts (state);
}

/** Exclude thread from scheduling
 *
 * The thread is removed from the list of schedulable threads.
 *
 */
void sched_remove (thread_t thread)
{
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	list_remove (&thread->link);
	conditionally_enable_interrupts (state);
}


/** Periodic scheduler timer handler
 *
 * The function is called from an interrupt handler.
 *
 */
void sched_timer (void)
{
	jiffies++;
	timers_run ();
	timer_setup (THREAD_QUANTUM);
	
	/*
	 * Reschedule if the current thread has been running
	 * for longer than the thread quantum.
	 */
	
	thread_t current = current_thread[cpuid()];
	unative_t timestamp = timer_get ();
	
	if (timestamp - current->scheduled >= THREAD_QUANTUM)
		schedule ();
}


/** Schedule the next thread to run
 *
 */
void schedule (void)
{
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	/* Just take the first thread on the list of schedulable threads. */
	link_t *link = list_rotate (&runnable_list[cpuid()]);
	if (link != NULL) {
		thread_t next_thread = list_item (link, struct thread, link);
		next_thread->scheduled = timer_get ();
		thread_switch (next_thread);
	}
	
	conditionally_enable_interrupts (state);
}
