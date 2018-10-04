/***
 * Mutex test #2
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/02 pallas  cleaned up and integrated
 * 2003/11/07 pallas  converted to robust_thread_create, cond_wait_mutex
 * 2004/10/25 pallas  updated for new assignment specification
 * 2005/01/13 pallas  updated for user space API
 */

static char * desc =
    "Spawns a number of threads each incrementing a counter.\n"
    "The atomicity of the increments is assured by a critical\n"
    "section guarded by a mutex.\n";


#include <librt.h>
#include "../../include/defs.h"


/*
 * The number of cycles we do in this test.
 */
#define CYCLE_COUNT  (TASK_SIZE)

/*
 * The number of threads to use concurrently.
 */
#define THREAD_COUNT  (TASK_SIZE * 10)

/*
 * The length of sleep in milliseconds.
 */
#define SLEEP_TIME_MS  10


/*
 * The global counter and associated mutex.
 */
static volatile int counter;
static struct mutex counter_mtx;

/*
 * Number of running threads and associated mutex.
 */
static volatile int threads_running;
static struct mutex threads_running_mtx;


static void * thread_proc (void * data)
{
	int cnt;
	int local_counter;
	
	assert (data == THREAD_MAGIC);
	
	
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		mutex_lock (&counter_mtx);
		
		/*
		 * Get a copy of the counter, wait a while and store an
		 * increment of the local counter. If the mutexes don't work,
		 * the counter will be damaged.
		 */
		local_counter = counter;
		thread_usleep (SLEEP_TIME_MS * 1000);
		counter = local_counter + 1;
		
		mutex_unlock (&counter_mtx);
	}
	
	
	dec_var_mutex (threads_running, threads_running_mtx);
	return NULL;
}

/*
 * Intermediate function to make robust_* definitions that return pointers work
 */
static char * main_thread (void)
{
	int cnt;
	thread_t threads [THREAD_COUNT];
	
	// init
	counter = 0;
	threads_running = THREAD_COUNT;
	
	mutex_init (&counter_mtx);
	mutex_init (&threads_running_mtx);
	
	
	/*
	 * Start the threads.
	 */
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		threads [cnt] = robust_thread_create (
		    thread_proc, (void *) THREAD_MAGIC);
	}
	
	/*
	 * Wait for all the threads to finish.
	 */
	printf ("Waiting for all the threads to finish.\n");
	cond_wait_mutex_verbose (
	    threads_running > 0, threads_running_mtx, threads_running);
	
	
	// clean up
	for (cnt = 0; cnt < THREAD_COUNT; cnt++) {
		robust_thread_join (threads [cnt]);
	}
	
	mutex_destroy (&threads_running_mtx);
	mutex_destroy (&counter_mtx);
	
	// print the result
	if (counter == THREAD_COUNT * CYCLE_COUNT)
		return NULL;
	
	return ("Counter value is wrong. Should be " STRING(THREAD_COUNT * CYCLE_COUNT));
}

int main (void)
{
	printf (desc);
	
	char * ret = main_thread();
	
	// print the result
	if (ret == NULL) {
		printf("\nTest passed...\n\n");
		return 0;
	} else {
		printf("\n%s\n\n", ret);
		return 1;
	}
}
