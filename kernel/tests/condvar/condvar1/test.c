/***
 * Condition variable test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 ghort   cleaned up and integrated
 * 2003/11/07 pallas  converted to robust_thread_create
 * 2004/10/25 pallas  updated for new assignment specification
 * 2004/11/08 pallas  removed extraneous mutex_unlock in cleanup
 */

static char * desc =
    "Condition variable test #1\n"
    "The well known producer/consumer task. There are several\n"
    "concurrent threads producing and several concurrent threads\n"
    "consuming items. The length of the queue is unlimited, \n"
    "therefore the production is non-blocking.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of cycles performed.
 */
#define CYCLE_COUNT  (TASK_SIZE * 100)

/*
 * The number of threads of each kind to run concurrently.
 */
#define PRODUCER_COUNT  (TASK_SIZE * 2)
#define CONSUMER_COUNT  (TASK_SIZE * 3)

/*
 * How many different producers and consumers we have.
 */
#define PRODUCER_TYPES  2

/*
 * Total number of producer/consumer threads.
 */
#define PRODUCER_THREADS  (PRODUCER_COUNT * PRODUCER_TYPES)
#define CONSUMER_THREADS  CONSUMER_COUNT


/*
 * Number of running threads and associated mutex.
 */
static volatile int threads_running;
static struct mutex threads_running_mtx;

/*
 * Global termination flag, should read PRODUCER_TYPES to end the test.
 */
static volatile int finish_flag;

/*
 * The main condition variable and associated mutex.
 */
static struct condvar shared_cond;
static struct mutex shared_mtx;

/*
 * The number of items consumed or available
 * (produced but not yet consumed).
 */
static volatile int consumed;
static volatile int available;


static void *
thread_proc_producer_signal_inside_mutex (void *data)
{
	int cnt;
	assert (data == THREAD_MAGIC);
	
	// produce items
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		mutex_lock (&shared_mtx);
		
		available++;
		condvar_signal (&shared_cond);
		thread_yield ();
		
		mutex_unlock (&shared_mtx);
	}
	
	mutex_lock (&threads_running_mtx);
	
	finish_flag++;
	if (finish_flag == PRODUCER_THREADS) {
		/*
		 * Wake up all remaining consumers waiting for items
		 * that will not be produced anymore.
		 */
		condvar_broadcast (&shared_cond);
	}
	
	threads_running--;
	mutex_unlock (&threads_running_mtx);
	
	return NULL;
} /* thread_proc_producer_signal_inside_mutex */

static void *
thread_proc_producer_signal_outside_mutex (void * data)
{
	int cnt;
	assert (data == THREAD_MAGIC);
	
	// produce items
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		inc_var_mutex (available, shared_mtx);
		
		condvar_signal (&shared_cond);
		thread_yield ();
	}
	
	mutex_lock (&threads_running_mtx);
	
	finish_flag++;
	if (finish_flag == PRODUCER_THREADS) {
		/*
		 * Wake up all remaining consumers waiting for items
		 * that will not be produced anymore.
		 */
		condvar_broadcast (&shared_cond);
	}
	
	threads_running--;
	mutex_unlock (&threads_running_mtx);
	
	return NULL;
} /* thread_proc_producer_signal_outside_mutex */


static void *
thread_proc_consumer_block (void *data)
{
	assert (data == THREAD_MAGIC);
	
	// consume items
	while (true) {
		mutex_lock (&shared_mtx);
		
		while ((!available) && (finish_flag != PRODUCER_THREADS)) {
			condvar_wait (&shared_cond, &shared_mtx);
		}
		
		if (!available) {
			/*
			 * This means that finish_flag == PRODUCER_THREADS,
			 * so we end the consumption.
			 */
			mutex_unlock (&shared_mtx);
			break;
		}
		
		available--;
		consumed++;
		
		mutex_unlock (&shared_mtx);
	}
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
} /* thread_proc_consumer_block */


void test_run (void)
{
	int cnt;
	thread_t producers [PRODUCER_COUNT] [PRODUCER_TYPES];
	thread_t consumers [CONSUMER_COUNT];
	
	printk (desc);
	
	// init
	finish_flag = 0;
	consumed = available = 0;
	threads_running = PRODUCER_THREADS + CONSUMER_THREADS;
	
	mutex_init (&threads_running_mtx);
	mutex_init (&shared_mtx);
	
	condvar_init (&shared_cond);
	
	
	/*
	 * Create the threads.
	 */
	for (cnt = 0; cnt < CONSUMER_COUNT; cnt++) {
		consumers [cnt] = robust_thread_create (
		    thread_proc_consumer_block, THREAD_MAGIC, 0);
	}
	
	for (cnt = 0; cnt < PRODUCER_COUNT; cnt++) {
		producers [cnt] [0] = robust_thread_create (
		    thread_proc_producer_signal_inside_mutex,
		    THREAD_MAGIC, 0);
		producers [cnt] [1] = robust_thread_create (
		    thread_proc_producer_signal_outside_mutex,
		    THREAD_MAGIC, 0);
	}
	
	/*
	 * Wait for all the threads to finish.
	 */
	printk ("Waiting for all the threads to finish...\n");
	cond_wait_mutex_verbose (threads_running > 0,
	    threads_running_mtx, threads_running);
	
	/*
	 * Reap the threads.
	 */
	for (cnt = 0; cnt < CONSUMER_COUNT; cnt++) {
		robust_thread_join (consumers [cnt]);
	}
	
	for (cnt = 0; cnt < PRODUCER_COUNT; cnt++) {
		robust_thread_join (producers [cnt] [0]);
		robust_thread_join (producers [cnt] [1]);
	}
	
	
	// clean up
	mutex_destroy (&threads_running_mtx);
	mutex_destroy (&shared_mtx);
	condvar_destroy (&shared_cond);
	
	// print the result
	printk ("Items consumed: %d\n", consumed);
	if (consumed == PRODUCER_THREADS * CYCLE_COUNT) {
		printk ("Test passed...\n");
	} else {
		printk ("The number of consumed items is not %d.\n"
		    "Test failed...\n", PRODUCER_THREADS * CYCLE_COUNT);
	}
}
