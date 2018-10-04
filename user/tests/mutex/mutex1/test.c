/***
 * Mutex test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/02 pallas  cleaned up and integrated
 * 2004/10/25 pallas  updated for new assignment specification
 * 2005/01/13 pallas  updated for user space API
 */

static char * desc =
    "Creates a mutex, which is then repeatedly locked and\n"
    "unlocked by a single thread.\n";


#include <librt.h>
#include "../../include/defs.h"


/*
 * The number of cycles we do in this test.
 */
#define CYCLE_COUNT  (TASK_SIZE * 1000)


int main (void)
{
	int cnt;
	struct mutex mtx;
	
	printf (desc);
	
	// initialize
	mutex_init (&mtx);
	
	
	/*
	 * Play with the mutex...
	 */
	for (cnt = 0; cnt < CYCLE_COUNT; cnt++) {
		mutex_lock (&mtx);
		mutex_unlock (&mtx);
	}
	
	
	// clean up
	mutex_destroy (&mtx);
	
	printf("\nTest passed...\n\n");
	return 0;
}
