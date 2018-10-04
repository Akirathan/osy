/**
 * @file example.c
 *
 * Simple example with multiple threads.
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

#include <proc/thread.h>
#include <proc/process.h>
#include <sched/sched.h>
#include <lib/print.h>

#ifdef KERNEL_TEST
	#include <api.h>
#endif

#include <example.h>


#ifndef KERNEL_TEST

/** Random seed for the example */
static unsigned int seed = 0;

/** Activity threads */
static thread_t thread_one;
static thread_t thread_two;

/** User space process */
static process_t process;


/** Example thread activity
 *
 * The function simply prints a thread signature
 * and a few additional characters to show
 * thread activity.
 *
 * @param signature The thread signature string.
 * @param activity  The thread activity character.
 *
 */
static void activity (const char *signature, const char activity)
{
	for (unsigned int i = 0 ; i < 100 ; i++) {
		/* Print the thread signature with interrupts disabled
		   to make sure it is not mixed with output of other
		   threads. */
		
		ipl_t state = query_and_disable_interrupts ();
		puts (signature);
		conditionally_enable_interrupts (state);
		
		/* Print a random number of activity characters. */
		for (unsigned int i = 0 ; i < 10 + random (&seed) % 10 ; i++)
			putc (activity);
	}
}


/** Example thread one
 *
 */
static void *thread_one_function (void *data)
{
	activity (" [Thread 1] ", '+');
	return NULL;
}

/** Example thread two
 *
 */
static void *thread_two_function (void *data)
{
	activity (" [Thread 2] ", '*');
	return NULL;
}

#endif /* !KERNEL_TEST */


/** Main example function
 *
 * Use three threads to print gibberish on console
 * or run the kernel test.
 *
 */
void *example (void *data)
{
#ifdef KERNEL_TEST
	
	/* If kernel test is compiled in, run it. */
	test_run ();
	
#else /* KERNEL_TEST */
	
	/* Initialize the structures of two new threads. */
	int rc = thread_create (&thread_one, thread_one_function, NULL, 0);
	if (rc != EOK)
		panic ("Error creating the first activity thread (%d).", rc);
	
	rc = thread_create (&thread_two, thread_two_function, NULL, 0);
	if (rc != EOK)
		panic ("Error creating the second activity thread (%d).", rc);
	
	/* Exhibit some activity of our own. */
	activity (" [Thread 0] ", '-');
	
	thread_join (thread_one, NULL);
	thread_join (thread_two, NULL);
	
	printk ("\nCreating user space process ...\n");
	rc = process_create (&process, (void *) ADDR_IN_KSEG0 (PROCESS_BASE),
	    PROCESS_SIZE);
	if (rc != EOK)
		panic ("Error creating user space process.");
	
	process_join (process);
	
#endif /* KERNEL_TEST */
	
	/* The machine is halted as soon as any thread finishes. */
	msim_halt ();
	
	return NULL;
}
