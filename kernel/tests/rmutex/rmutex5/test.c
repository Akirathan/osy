/***
 * Recursive mutex test #5
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
    "Creates a recursive mutex, locks it twice, but unlocks it only once.\n"
    "The unbalanced operations should result in panic.\n\n";


#include <api.h>
#include "../../include/defs.h"


void
test_run (void)
{
	struct rmutex mtx;
	
	printk (desc);
	
	// initialize
	rmutex_init (&mtx);
	
	// lock twice
	rmutex_lock (&mtx);
	rmutex_lock (&mtx);
	
	// unlock once
	rmutex_unlock (&mtx);
	
	// clean up (this should result in a panic)
	rmutex_destroy (&mtx);
	
	// print the result
	printk ("This statement should not have been reached.\n"
	    "Test failed...\n");
}
