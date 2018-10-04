/***
 * User space memory protection check
 */

#include <librt.h>

int main (void)
{
	printf ("Trying to dereference kernel memory...\n");
	
	int volatile * kmem = (int * ) 0x80001000;
	*kmem = 10;
	
	printf("Test should not reach this point\n");
	
	return 1;
}
