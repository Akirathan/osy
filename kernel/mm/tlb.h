/**
 * @file tlb.h
 *
 * Address translation handling.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef TLB_H_
#define TLB_H_


/* Externals are commented with implementation */
extern void tlb_init (void);
extern void tlb_invalid (context_t *registers);
extern void tlb_flush (uintptr_t addr);
extern void wrapped_tlb_refill (context_t *registers);


#endif
