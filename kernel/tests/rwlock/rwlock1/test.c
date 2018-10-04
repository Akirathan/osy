/***
 * Read/Write lock test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 ghort   cleaned up and integrated
 * 2003/11/07 pallas  more cleanups, converted to robust_thread_create
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Read/Write lock test #1\n"
    "Creates a single read/write lock and lets multiple readers\n"
    "and multiple writers play with it.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of cycles we do in each of the test phases.
 */
#define CYCLE_COUNT  (TASK_SIZE)

/*
 * The number of milliseconds to wait before next read/write attempt.
 */
#define RW_DELAY_MS  10

/*
 * The number of readers/writers to run concurrently.
 */
#define READER_COUNT  (TASK_SIZE * 2)
#define WRITER_COUNT  (TASK_SIZE * 2)


/*
 * The number of running threads and associated mutex.
 */
static volatile int threads_running;
static struct mutex threads_running_mtx;

/*
 * The rwlock to test.
 */
static struct rwlock rwlock;

/*
 * Global success flag.
 */
static volatile int success;

/*
 * Global start flag.
 */
static volatile int start_flag;

/*
 * When we get in a writer section we set this state to writer, so we
 * can test from readers whether the lock works properly.
 */
enum {
	ST_WRITER,
	ST_NOWRITER
} test_state;


static void *
thread_proc_reader_block (void *data)
{
	int cnt;
	assert (data == THREAD_MAGIC);
	
	while (!start_flag) {
		thread_yield ();
	}
	
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		rwlock_read_lock (&rwlock);
		
		if (test_state != ST_NOWRITER) {
			success = 0;
		}
		
		thread_yield ();
		rwlock_read_unlock (&rwlock);
		
		// wait a while before next read attempt
		thread_usleep (RW_DELAY_MS * 1000);
	}
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
} /* thread_proc_reader_block */

static void *
thread_proc_writer_block (void * data)
{
	int cnt;
	assert (data == THREAD_MAGIC);
	
	while (!start_flag) {
		thread_yield ();
	}
	
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		rwlock_write_lock (&rwlock);
		
		test_state = ST_WRITER;
		thread_yield ();
		test_state = ST_NOWRITER;
		
		rwlock_write_unlock (&rwlock);
		
		// wait a while before next write attempt
		thread_usleep (RW_DELAY_MS * 1000);
	}
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
} /* thread_proc_writer_block */

void
test_run (void)
{
	int cnt;
	thread_t readers [READER_COUNT];
	thread_t writers [WRITER_COUNT];
	
	printk (desc);
	
	// init
	success = 1;
	test_state = ST_NOWRITER;
	threads_running = READER_COUNT + WRITER_COUNT;
	
	rwlock_init (&rwlock);
	mutex_init (&threads_running_mtx);
	
	
	/*
	 * Start the readers.
	 */
	printk ("Creating %d reader threads.\n", READER_COUNT);
	for (cnt = 0; cnt < READER_COUNT; cnt++) {
		readers [cnt] = robust_thread_create (
		    thread_proc_reader_block, THREAD_MAGIC, 0);
	}
	
	/*
	 * Start the writers.
	 */
	printk ("Creating %d writer threads.\n", WRITER_COUNT);
	for (cnt = 0; cnt < WRITER_COUNT; cnt++) {
		writers [cnt] = robust_thread_create (
		    thread_proc_writer_block, THREAD_MAGIC, 0);
	}
	
	/*
	 * Unleash the hordes...
	 */
	printk ("Unleashing the threads...\n");
	start_flag = 1;
	
	/*
	 * Wait for all the threads to finish.
	 */
	printk ("Waiting for all the threads to finish.\n");
	cond_wait_mutex_verbose (threads_running > 0, threads_running_mtx,
	    threads_running);
	
	/*
	 * Reap the threads.
	 */
	for (cnt = 0; cnt < READER_COUNT; cnt++) {
		robust_thread_join (readers [cnt]);
	}
	
	for (cnt = 0; cnt < WRITER_COUNT; cnt++) {
		robust_thread_join (writers [cnt]);
	}
	
	
	// clean up
	mutex_destroy (&threads_running_mtx);
	rwlock_destroy (&rwlock);
	
	// print the result
	if (success) {
		printk ("Test passed...\n");
	} else {
		printk ("Inconsistency in read.\n"
			"Test failed...\n");
	}
}
