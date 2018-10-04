/***
 * User space protection check
 */

#include <librt.h>

int main (void)
{
	printf ("Trying to use CP0 intruction...\n");
	
	asm volatile (
		"mfc0 $26, $12"  /* addiu $k0, $status -> coprocessor unusable exception */
	);
	
	printf("Test should not reach this point\n");
	
	return 1;
}
