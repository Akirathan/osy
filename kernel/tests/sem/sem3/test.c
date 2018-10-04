/***
 * Semaphore test #3
 *
 * Change Log:
 * 2003/11/02 pallas  created
 * 2003/11/07 pallas  converted to robust_thread_create
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Semaphore test #3\n"
    "Creates a convoy of threads on a single zero semaphore\n"
    "and the wakes them up sequentially by incrementing the\n"
    "semaphore.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of threads to start.
 */
#define THREAD_COUNT  (TASK_SIZE * 10)


/*
 * Shared semaphore.
 */
static struct semaphore	shared_sem;

/*
 * Number of running threads and associated semaphore mutex.
 */
static volatile unsigned int threads_running;
static struct semaphore threads_running_sem;


static void *
thread_proc (void * data)
{
	assert (data == THREAD_MAGIC);
	
	// let the master thread know about us
	inc_var_sem (threads_running, threads_running_sem);
	
	// decrement the semaphore (which should put us to sleep)
	sem_down (&shared_sem);
	
	// let the master know we woke up
	dec_var_sem (threads_running, threads_running_sem);
	
	return NULL;
}


void
test_run (void)
{
	int cnt;
	thread_t threads [THREAD_COUNT];
	
	printk (desc);
	
	// init
	threads_running = 0;
	
	sem_init (&shared_sem, 0);
	sem_init (&threads_running_sem, 1);
	
	
	/*
	 * Start the threads.
	 */
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		threads [cnt] = robust_thread_create (
		    thread_proc, THREAD_MAGIC, 0);
	}
	
	/*
	 * Wait for the threads to initialize.
	 */
	printk ("Waiting for all threads to initialize...\n");
	cond_wait_sem_verbose (threads_running < THREAD_COUNT,
	    threads_running_sem, threads_running);
	
	/*
	 * Wake the threads.
	 */
	printk ("Waking up the threads...\n");
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		sem_up (&shared_sem);
	}
	
	/*
	 * Wait for all the threads to finish.
	 */
	printk ("Waiting for all threads to end...\n");
	cond_wait_sem_verbose (threads_running > 0, threads_running_sem,
	    threads_running);
	
	/*
	 * Reap the threads.
	 */
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		robust_thread_join (threads [cnt]);
	}
	
	
	// clean up
	sem_destroy (&threads_running_sem);
	sem_destroy (&shared_sem);
	
	// print the result
	printk ("Test passed...\n");
}
