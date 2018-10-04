/***
 * Read/Write lock test #2
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 ghort   cleaned up and integrated
 * 2003/11/07 pallas  more cleanups, converted to robust_thread_create, etc.
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Read/Write lock test #2\n"
    "Tests whether multiple readers are allowed to lock "
    "simultaneously.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of threads to test.
 */
#define READER_COUNT  (TASK_SIZE * 10)


/*
 * Number of reading threads and associated mutex.
 */
static volatile int threads_reading;
static struct mutex threads_reading_mtx;

/*
 * Global termination flag.
 */
static volatile int finish_flag;

/*
 * The tested shared rwlock.
 */
static struct rwlock rwlock;


static void *
thread_proc (void *data)
{
	assert (data == THREAD_MAGIC);
	
	rwlock_read_lock (&rwlock);
	inc_var_mutex (threads_reading, threads_reading_mtx);
	
	while (!finish_flag)
		thread_usleep (100 * 1000);
	
	rwlock_read_unlock (&rwlock);
	dec_var_mutex (threads_reading, threads_reading_mtx);
	
	return NULL;
}


void
test_run (void)
{
	int cnt;
	thread_t threads [READER_COUNT];
	
	printk (desc);
	
	// init
	finish_flag = 0;
	threads_reading = 0;
	
	rwlock_init (&rwlock);
	mutex_init (&threads_reading_mtx);
	
	
	/*
	 * Start the readers.
	 */
	for (cnt = 0; cnt < READER_COUNT; cnt++) {
		threads [cnt] = robust_thread_create (thread_proc,
		    THREAD_MAGIC, 0);
	}
	
	/*
	 * Wait for all the threads to start reading.
	 */
	printk ("Waiting for all the threads to start reading.\n");
	cond_wait_mutex_verbose (threads_reading < READER_COUNT,
	    threads_reading_mtx, threads_reading);
	
	/*
	 * End the test.
	 */
	finish_flag = 1;
	
	/*
	 * Reap the threads.
	 */
	printk ("Waiting for all the threads to finish.\n");
	for (cnt = 0; cnt < READER_COUNT; cnt++) {
		robust_thread_join (threads [cnt]);
	}
	
	
	// clean up
	mutex_destroy (&threads_reading_mtx);
	rwlock_destroy (&rwlock);
	
	// print the result
	printk ("Test passed...\n");
}
