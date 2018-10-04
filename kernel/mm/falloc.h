/**
 * @file falloc.h
 *
 * Frame allocator.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef FALLOC_H_
#define FALLOC_H_


#include <include/c.h>


/** The size of a frame.
 *
 * The frame size is determined by the number of bits.
 * The default value of 12 results in 4096 byte frames.
 *
 */
#define FRAME_WIDTH  12
#define FRAME_SIZE   (1 << FRAME_WIDTH)


/** Allocation flags
 *
 * - VF_VA_AUTO: Use optimal virtual address
 * - VF_VA_USER: Use virtual address supplied by user
 *
 * - VF_AT_KUSEG: Prefer physical memory not accessible from KSEG0/KSEG1
 * - VF_AT_KSEG0: Allocate physical memory accessible from KSEG0
 * - VF_AT_KSEG1: Allocate physical memory accessible from KSEG1
 * - VF_AT_KSSEG: Prefer physical memory not accessible from KSEG0/KSEG1
 * - VF_AT_KSEG3: Prefer physical memory not accessible from KSEG0/KSEG1
 *
 */
typedef enum {
	VF_VA_AUTO = (1 << 0),
	VF_VA_USER = (1 << 1),
	
	VF_AT_KUSEG = (1 << 2),
	VF_AT_KSEG0 = (1 << 3),
	VF_AT_KSEG1 = (1 << 4),
	VF_AT_KSSEG = (1 << 5),
	VF_AT_KSEG3 = (1 << 6)
} vm_flags_t;


/** Address of kernel image end.
 *
 * The symbol containing the last address of the kernel image is
 * provided by the linker, see the linker script for details of how
 * this is done.
 *
 */
extern uint8_t _kernel_end;


/* Externals are commented with implementation */
extern void frame_init (void);
extern int frame_alloc (uintptr_t *phys, const size_t cnt, const vm_flags_t flags);
extern int frame_free (const uintptr_t phys, const size_t cnt);


#endif
