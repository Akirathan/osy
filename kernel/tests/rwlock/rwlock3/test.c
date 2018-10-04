/***
 * Read/Write lock test #3
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 ghort   cleaned up and integrated
 * 2003/11/07 pallas  more cleanups, dynamic lock array, conversion to
 *                    robust_thread_create, etc.
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Read/Write lock test #3\n"
    "Creates an array of rwlocks, locks them all from reading\n"
    "and creates a thread for each lock attempting to lock it\n"
    "for writing.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of locks to use.
 */
#define RWL_COUNT  (TASK_SIZE * 10)

/*
 * The delay in seconds before considering all threads waiting.
 */
#define WAIT_DELAY  5

/*
 * The delay in seconds before considering all threads running.
 */
#define CHECK_DELAY  5


/*
 * Number of running threads and associated mutex.
 */
static volatile int threads_running;
static struct mutex threads_running_mtx;

/*
 * Read/write locks.
 */
static struct rwlock * rwl_array [RWL_COUNT];


static void *
thread_proc (void * data)
{
	int lock_idx;
	
	lock_idx = (int) data;
	rwlock_write_lock (rwl_array [lock_idx]);
	rwlock_write_unlock (rwl_array [lock_idx]);
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
}


void
test_run (void)
{
	int cnt;
	int success;
	struct rwlock * rwl_mem;
	thread_t threads [RWL_COUNT];
	
	printk (desc);
	
	// init
	success = 1;
	threads_running = RWL_COUNT;
	
	mutex_init (&threads_running_mtx);
	
	rwl_mem = (struct rwlock *) malloc (
	    RWL_COUNT * sizeof (struct rwlock));
	if (!rwl_mem) {
		printk ("Unable to allocate memory for rwlock array.\n"
		    "Test failed...\n");
		return;
	}
	
	for (cnt = 0; cnt < RWL_COUNT; cnt++) {
		rwlock_init (rwl_mem);
		rwl_array [cnt] = rwl_mem++;
	}
	
	rwl_mem = rwl_array [0];
	
	
	/*
	 * Create a writer thread for each of the locks.
	 */
	for (cnt = 0; cnt < RWL_COUNT; cnt++) {
		rwlock_read_lock (rwl_array [cnt]);
		threads [cnt] = robust_thread_create (
		    thread_proc, (void *) cnt, 0);
	}
	
	/*
	 * Wait a while so that all the writers go to sleep.
	 */
	printk ("Giving threads %d seconds to start waiting: ", WAIT_DELAY);
	for (cnt = 0; cnt < WAIT_DELAY; cnt++) {
		thread_sleep (1);
		putc ('.');
	}
	printk ("\n");
	
	/*
	 * Check if all the threads are still running.
	 */
	printk ("Checking waiting threads for %d seconds: ", CHECK_DELAY);
	for (cnt = 0; cnt < CHECK_DELAY; cnt++) {
		mutex_lock (&threads_running_mtx);
		
		if (threads_running != RWL_COUNT) {
			success = 0;
		}
		
		mutex_unlock (&threads_running_mtx);
		thread_sleep (1);
		putc ('.');
	}
	printk ("\n");
	
	/*
	 * Unlock the locks so that the threads can finish.
	 */
	printk ("Unlocking the read locks so the threads can finish.\n");
	for (cnt = 0; cnt < RWL_COUNT; cnt++) {
		rwlock_read_unlock (rwl_array [cnt]);
	}
	
	/*
	 * Wait for all the threads to finish.
	 */
	printk ("Waiting for all the threads to finish.\n");
	cond_wait_mutex_verbose (threads_running > 0, threads_running_mtx,
	    threads_running);
	
	/*
	 * Reap the threads.
	 */
	for (cnt = 0; cnt < RWL_COUNT; cnt++) {
		robust_thread_join (threads [cnt]);
	}
	
	
	// clean up
	mutex_destroy (&threads_running_mtx);
	
	for (cnt = 0; cnt < RWL_COUNT; cnt++) {
		rwlock_destroy (rwl_array [cnt]);
	}
	
	free (rwl_mem);
	
	
	// print the result
	if (success) {
		printk ("Test passed...\n");
	} else {
		printk ("One of the threads did not wait on the write lock.\n"
		    "Test failed...\n");
	}
}
