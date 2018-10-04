/***
 * Recursive mutex test #3
 *
 * Change Log:
 * 2003/10/20 ghort   created
 * 2003/11/02 pallas  cleaned up and integrated
 * 2003/11/07 pallas  converted to robust_thread_create, dec_var_mutex, etc.
 * 2004/10/25 pallas  updated for new assignment specification
 * 2015/10/26 md      update for recursive mutex
 */

static char * desc =
    "Recursive mutex test #3\n"
    "Spawns a number of threads and makes them all sleep on one\n"
    "mutex. Tests the blocking lock.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of seconds to let the threads play with the mutex.
 */
#define TEST_DURATION  5

/*
 * The number of threads of each kind to run concurrently.
 */
#define THREAD_COUNT  (TASK_SIZE * 5)

/*
 * The length of sleep in milliseconds.
 */
#define SLEEP_TIME_MS  10

/*
 * The tested mutex.
 */
static struct rmutex mtx;

/*
 * Global failure indicator.
 */
static volatile int test_failed;

/*
 * Global termination indicator.
 */
static volatile int finish_flag;

/*
 * Number of running threads and associated mutex.
 */
static volatile int threads_running;
static struct rmutex threads_running_mtx;


static void *
thread_proc_blocking (void * data)
{
	assert (data == THREAD_MAGIC);
	
	rmutex_lock (&mtx);
	
	if (!finish_flag) {
		/*
		 * The mutex released us before it was allowed to.
		 */
		printk ("%s: mutex acquired before it was possible\n",
		    __FUNCTION__);
		test_failed = 1;
	}
	
	// simulate recursive use
	rmutex_lock (&mtx);
	thread_usleep (SLEEP_TIME_MS * 1000);
	rmutex_unlock (&mtx);
	
	rmutex_unlock (&mtx);
	
	dec_var_rmutex (threads_running, threads_running_mtx);
	
	return NULL;
}


void
test_run (void)
{
	int cnt;
	thread_t threads [THREAD_COUNT];
	
	printk (desc);
	
	// init
	test_failed = 0;
	finish_flag = 0;
	threads_running = THREAD_COUNT;
	
	rmutex_init (&mtx);
	rmutex_init (&threads_running_mtx);
	
	
	/* 
	 * Lock the mutex and start the threads so they can sleep on it.
	 */
	rmutex_lock (&mtx);
	
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		threads [cnt] = robust_thread_create (
		    thread_proc_blocking, THREAD_MAGIC, 0);
	}
	
	/*
	 * Wait a while so that all the threads can play with the mutex.
	 */
	printk ("Giving the threads %d seconds to play...\n", TEST_DURATION);
	thread_sleep (TEST_DURATION);
	
	/*
	 * Signal the end of the test and unlock the mutex so the
	 * threads can wake up.
	 */
	finish_flag = 1;
	rmutex_unlock (&mtx);
	
	/*
	 * Wait for all the threads to finish.
	 */
	printk ("Waiting for all the threads to finish...\n");
	cond_wait_rmutex_verbose (threads_running > 0, threads_running_mtx,
	    threads_running);
	
	/*
	 * Reap the threads.
	 */
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		robust_thread_join (threads [cnt]);
	}
	
	/*
	 * Lock/unlock the mutex to verify that it is in correct state.
	 */
	rmutex_lock (&mtx);
	rmutex_unlock (&mtx);
	
	
	// clean up
	rmutex_destroy (&threads_running_mtx);
	rmutex_destroy (&mtx);
	
	// print the result
	if (!test_failed) {
		printk ("Test passed...\n");
	} else {
		printk ("Test failed...\n");
	}
}
