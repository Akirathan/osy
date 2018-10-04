/**
 * @file debug.h
 *
 * Debugging functions. Contains assert and dprintf.
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

#ifndef LIBRT_DEBUG_H_
#define LIBRT_DEBUG_H_


#include <stdio.h>
#include <thread.h>


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
			printf ("Assertion failed in %s() at %s:%d: %s\n", \
			    __FUNCTION__, __FILE__, __LINE__, #expr); \
			exit (-1); \
		}
#endif


/** dprintf
 *
 * Display a debug message on the console, prefixed by the function
 * name in which the macro was used.
 *
 * @param args  Arguments for printf.
 *
 */
#ifdef NDEBUG
#	define dprintf(args...) \
		do {} while (0)
#else
#	define dprintf(args...) \
		{ \
			printf ("%s:%d: ", __FUNCTION__, __LINE__); \
			printf (args); \
		}
#endif

#endif
