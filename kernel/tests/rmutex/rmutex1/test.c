/***
 * Recursive mutex test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/02 pallas  cleaned up and integrated
 * 2004/10/25 pallas  updated for new assignment specification
 * 2015/10/26 md      update for recursive mutex
 */

static char * desc =
    "Recursive mutex test #1\n"
    "Creates a recursive mutex, which is then repeatedly locked and\n"
    "unlocked by a single thread.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of cycles we do in this test.
 */
#define CYCLE_COUNT  (TASK_SIZE * 1000)


/*
 * The number of lock operations in one cycle.
 */
#define LOCK_COUNT  10


void
test_run (void)
{
	int cnt;
	int lock;
	struct rmutex mtx;
	
	printk (desc);
	
	// initialize
	rmutex_init (&mtx);
	
	
	/*
	 * Play with the mutex...
	 */
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		for (lock = 0; lock < LOCK_COUNT; lock++) {
			rmutex_lock (&mtx);
		}
		
		for (lock = 0; lock < LOCK_COUNT; lock++) {
			rmutex_unlock (&mtx);
		}
	}
	
	
	// clean up
	rmutex_destroy (&mtx);
	
	// print the result
	printk ("Test passed...\n");
}
