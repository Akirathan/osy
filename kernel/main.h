/**
 * @file main.h
 *
 * Kernel mainline.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef MAIN_H_
#define MAIN_H_


/* Externals are commented with implementation */
extern void bsp_start (void) __attribute__((noreturn));
extern void ap_start (void) __attribute__((noreturn));


#endif
