/**
 * @file entry.S
 *
 * User space run-time library entry point.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <abi.h>

.set noreorder

/*
 * Put the hardwired entry code to a special input section ".entry".
 * This section is then placed by the linker (instructed by the
 * process.lds linker script) at the very beginning of the process
 * image.
 */

.section .entry, "ax"
.global __entry


/*
 * Process entry point
 */
.ent __entry
__entry:
	/*
	 * Setup support for position independent
	 * code (not strictly necessary, used as
	 * a convention).
	 */
	.frame $sp, ABI_STACK_FRAME, $ra
	.cpload $t9
	
	/*
	 * Setup ABI-compliant stack frame.
	 */
	addiu $sp, -ABI_STACK_FRAME
	
	/* Allow PIC code */
	.cprestore
	
	/* Jump to the C run-time initialization */
	jal __main
	nop
	
	/* Unreachable */
	addiu $sp, ABI_STACK_FRAME
.end __entry
