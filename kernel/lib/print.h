/**
 * @file print.h
 *
 * Output functions.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef PRINT_H_
#define PRINT_H_


#include <include/shared.h>
#include <include/c.h>


/* Externals are commented with implementation */
extern size_t putc (const char c);
extern size_t puts (const char *str);
extern size_t printk (const char *format, ...);
extern void panic (const char *format, ...) __attribute__((noreturn));


#endif /* PRINT_H_ */
