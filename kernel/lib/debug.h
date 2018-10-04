/**
 * @file debug.h
 *
 * Debugging functions. Contains assert and dprintk.
 * These statements are evaluated unless NDEBUG macro is declared.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef DEBUG_H_
#define DEBUG_H_


#include <lib/print.h>


/** assert
 *
 * Assert that a condition expressed by @expr holds.
 *
 * @param expr Expression to assert
 *
 */
#ifdef NDEBUG
#	define assert(expr) \
		do {} while (0)
#else
#	define assert(expr) \
		if (!(expr)) { \
			panic ("Assertion failed in %s() at %s:%d: %s\n", \
				__FUNCTION__, __FILE__, __LINE__, #expr); \
		}
#endif


/** dprintk
 *
 * Display a debug message on the console, prefixed by the function
 * name in which the macro was used.
 *
 * @param args  Arguments for printk.
 *
 */
#ifdef NDEBUG
#	define dprintk(args...) \
		do {} while (0)
#else
#	define dprintk(args...) \
		{ \
			printk ("%s:%d: ", __FUNCTION__, __LINE__); \
			printk (args); \
		}
#endif

#endif /* DEBUG_H_ */
