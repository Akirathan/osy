/**
 * @file main.c
 *
 * User space mainline.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <librt.h>


#ifndef USER_TEST


int main (void)
{
	printf ("\nUser space: Hello world!\n");
	
	return 0;
}


#endif /* USER_TEST */
