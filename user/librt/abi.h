/**
 * @file abi.h
 *
 * Application binary interface definitions.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */


#ifndef LIBRT_ABI_H_
#define LIBRT_ABI_H_


/** Minimal stack frame size according to MIPS o32 ABI */
#define ABI_STACK_FRAME  32


/** Page size */
#define PAGE_WIDTH  12
#define PAGE_SIZE   (1 << PAGE_WIDTH)


#ifndef __ASM__

/* User and run-time main functions */
extern int main (void);
extern void __main (void);

#endif /* __ASM__ */


#endif
