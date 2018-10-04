/***
 * Recursive mutex test #4
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/02 pallas  cleaned up and integrated
 * 2003/11/07 pallas  converted to robust_thread_create
 * 2004/10/25 pallas  updated for new assignment specification
 * 2015/10/26 md      update for recursive mutex
 */

static char * desc =
    "Recursive mutex test #5\n"
    "Creates a recursive mutex and locks it in one thread and unlocks it\n"
    "in another. The operation should result in panic.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The tested mutex.
 */
static struct rmutex	mtx;

/*
 * Global termination flag.
 */
static volatile int	finish_flag;


static void *
thread_proc (void * data)
{
	assert (data == THREAD_MAGIC);
	
	rmutex_unlock (&mtx);
	finish_flag = 1;
	
	return NULL;
}


void
test_run (void)
{
	thread_t thread;
	
	printk (desc);
	
	// init
	finish_flag = 0;
	
	rmutex_init (&mtx);
	rmutex_lock (&mtx);
	
	
	/*
	 * Start the thread and wait until the mutex is unlocked.
	 */
	thread = robust_thread_create (thread_proc, THREAD_MAGIC, 0);
	
	while (!finish_flag) {
		thread_sleep (1);
	}
	
	robust_thread_join (thread);
	
	
	// clean up
	rmutex_destroy (&mtx);
	
	// print the result
	printk ("This statement should not have been reached.\n"
	    "Test failed...\n");
}
