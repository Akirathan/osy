/**
 * @file printer.h
 *
 * Simple console support.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef PRINTER_H_
#define PRINTER_H_

#include <include/shared.h>


/** Base address of the console hardware */
#define PRINTER_ADDRESS  (ADDR_IN_KSEG0 (0x10000000))


/** Display a character
 *
 * @param c The character to display.
 *
 */
static inline void printer_putchar (const char c)
{
	(*(volatile char *) PRINTER_ADDRESS) = c;
}


#endif
