/***
 * Semaphore test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 pallas  cleaned up and integrated
 * 2003/11/07 pallas  converted to robust_thread_create
 * 2004/10/25 pallas  updated for new assignment specification
 * 2015/10/14 md      modify for blocking primitives only
 */

static char * desc =
    "Semaphore test #1\n"
    "The well known producer/consumer task. The test has three\n"
    "phases, a) with more producers than consumers, b) with the\n"
    "number of producers equal to the number of consumers, and\n"
    "c) with more consumers than producers. Producers and\n"
    "consumers are implemented using blocking primitives.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of cycles do in each of the phases of this test.
 */
#define CYCLE_COUNT  (TASK_SIZE * 10)

/*
 * The base number of producers/consumers.
 */
#define BASE_COUNT  (TASK_SIZE / 2)

/*
 * Timeout duration in the non-blocking version (miliseconds).
 */
#define DOWN_TIMEOUT_MS  1

/*
 * The size of the queue the produced items are stored to.
 */
#define QUEUE_LENGTH  (TASK_SIZE * TASK_SIZE)

/*
 * Phase duration in seconds.
 */
#define PHASE_DURATION  (TASK_SIZE / 2)

/*
 * The number of phases.
 */
#define PHASE_COUNT  3


/*
 * Numbers of producers/consumers in individual phases.
 */
static const int consumers_count [PHASE_COUNT] = {
	BASE_COUNT, BASE_COUNT * 2, BASE_COUNT * 3
};

static const int producers_count [PHASE_COUNT] = {
	BASE_COUNT * 3, BASE_COUNT * 2, BASE_COUNT
};

/*
 * Number of running threads and associated mutex.
 */
static volatile int threads_running;
static struct mutex threads_running_mtx;

/*
 * Semaphores for the queue.
 */
static struct semaphore queue_full;
static struct semaphore queue_free;


static void *
thread_proc_producer_block (void * data)
{
	int cnt;
	assert (data == THREAD_MAGIC);
	
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		sem_down (&queue_free);
		sem_up (&queue_full);
		thread_yield ();
	}
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
} /* thread_proc_producer_block */

static void *
thread_proc_consumer_block (void * data)
{
	int cnt;
	assert (data == THREAD_MAGIC);
	
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		sem_down (&queue_full);
		sem_up (&queue_free);
		thread_yield ();
	}
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
} /* thread_proc_consumer_block */


void
test_run (void)
{
	int phase;
	char * result;
	thread_t producers [PHASE_COUNT] [3 * BASE_COUNT];
	thread_t consumers [PHASE_COUNT] [3 * BASE_COUNT];
	
	printk (desc);
	
	// init
	threads_running = 0;
	
	sem_init (&queue_full, 0);
	sem_init (&queue_free, QUEUE_LENGTH);
	
	mutex_init (&threads_running_mtx);
	
	
	/*
	 * Go through all the phases.
	 */
	for (phase = 0; phase < PHASE_COUNT; phase++) {
		int cnt;
		
		printk ("Entering phase %d... spawning %d producers and %d "
		    "consumers\n", phase, producers_count [phase],
		    consumers_count [phase]);
		
		/*
		 * Spawn the producers.
		 */
		for (cnt = 0; cnt < producers_count [phase]; cnt++) {
			/*
			 * Increment the numbers of running threads. Each
			 * thread decrements the counter as it exits.
			 */
			add_var_mutex (1, threads_running, threads_running_mtx);
			
			producers [phase] [cnt] = robust_thread_create (
			    thread_proc_producer_block, THREAD_MAGIC, 0);
		}
		
		/*
		 * Spawn the consumers.
		 */
		for (cnt = 0; cnt < consumers_count [phase]; cnt++) {
			/*
			 * Increment the numbers of running threads. Each
			 * thread decrements the counter as it exits.
			 */
			add_var_mutex (1, threads_running, threads_running_mtx);
			
			consumers [phase] [cnt] = robust_thread_create (
			    thread_proc_consumer_block, THREAD_MAGIC, 0);
		}
		
		thread_sleep (PHASE_DURATION);
	} // phase loop
	
	/*
	 * Wait for all the threads to finish.
	 */
	printk ("Waiting for all the threads to finish...\n");
	cond_wait_mutex_verbose (threads_running > 0,
	    threads_running_mtx, threads_running);
	
	/*
	 * Reap the threads.
	 */
	for (phase = 0; phase < PHASE_COUNT; phase++) {
		int cnt;
		
		// reap the producers
		for (cnt = 0; cnt < producers_count [phase]; cnt++) {
			robust_thread_join (producers [phase] [cnt]);
		}
		
		// reap the consumers
		for (cnt = 0; cnt < consumers_count [phase]; cnt++) {
			robust_thread_join (consumers [phase] [cnt]);
		}
	} // phase loop
	
	
	/*
	 * Determine the result.
	 */
	if (sem_get_value (&queue_full) != 0) {
		result = "Queue still contains items.\nTest failed...\n";
	} else if (sem_get_value (&queue_free) != QUEUE_LENGTH) {
		result = "Queue is not empty.\nTest failed...\n";
	} else {
		result = "Test passed...\n";
	}
	
	
	// clean up
	mutex_destroy (&threads_running_mtx);
	
	sem_destroy (&queue_full);
	sem_destroy (&queue_free);
	
	// print the result
	printk (result);
}
