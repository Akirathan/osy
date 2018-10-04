/***
 * Stack overflow test
 *
 * Change Log:
 * 2004/05/14 ghort   created
 * 2005/01/13 pallas  updated for user space API
 * 2006/12/17 md      framework update
 */

#include <librt.h>

// Infinite recursion
static void recurse (char *dummy)
{
	char spacer [2048];
	
	if (dummy != NULL)
		dummy [0] = 0x55;
	
	recurse (spacer);
}


int main (void)
{
	recurse (NULL);
	
	printf("Test should not reach this point\n");
	
	return 1;
}
