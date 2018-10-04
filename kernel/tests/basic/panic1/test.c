/***
 * Panic test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 pallas  cleaned up and integrated
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Panic test #1\n"
    "Tests whether the panic () function works.\n\n";


#include <api.h>
#include "../../include/defs.h"


void
test_run (void)
{
	printk (desc);
	
	panic ("<%s, %d>\nTest passed...\n", "sample string", 42);
	
	printk ("This statement should not have been reached.\n"
	    "Test failed...\n");
}
