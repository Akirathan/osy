/**
 * @file kbd.h
 *
 * Keyboard.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef KBD_H_
#define KBD_H_


#include <include/shared.h>
#include <include/c.h>

#include <adt/list.h>


/** Base address of the keyboard hardware */
#define KBD_ADDRESS  (ADDR_IN_KSEG0 (0x10000008))


/** Read a character
 *
 * @return Read character.
 *
 */
static inline char kbd_getchar (void)
{
	return (*(volatile char *) KBD_ADDRESS);
}


/* Externals are commented with implementation */
extern void kbd_handle (void);
extern char getc (void);
extern int getc_try (void);


#endif
