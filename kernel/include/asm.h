/*
 * Assembly definitions file.
 *
 * This file groups definitions that are required by multiple files
 * and whose syntax permits them to be used only in assembler.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef DEFINES_ASM_H_
#define DEFINES_ASM_H_

#include <include/offset.h>


/***************************************************************************\
| Readable Register Names                                                   |
\***************************************************************************/


/*
 * Register 0 is always set to 0. It is called a wired zero.
 */
#define zero  0

/*
 * Register 1 is used by the assembler macros as a temporary register.
 * It can also be used as a normal register when its use by the
 * assembler macros is disabled.
 */
#define at  1

/*
 * General registers used by the compiled C code. The comments show
 * a calling convention that is traditionally observed by the compiler.
 */

/* Return values */
#define v0  2
#define v1  3

/* Function arguments */
#define a0  4
#define a1  5
#define a2  6
#define a3  7

/* Temporary registers */
#define t0  8
#define t1  9
#define t2  10
#define t3  11
#define t4  12
#define t5  13
#define t6  14
#define t7  15
#define s0  16
#define s1  17
#define s2  18
#define s3  19
#define s4  20
#define s5  21
#define s6  22
#define s7  23
#define t8  24
#define t9  25
#define gp  28      /* Global Pointer */
#define sp  29      /* Stack Pointer */
#define fp  30      /* Frame Pointer */

/*
 * Register 31 is used to store the return address of jump and
 * link branches. The register should not be used otherwise.
 */
#define ra  31

/*
 * Registers 26 and 27 are reserved for the operating system use.
 * Their context may be randomly overwritten and the application
 * should not use them.
 */
#define k0  26
#define k1  27


/*
 * System Control Coprocessor (CP0) Registers
 */
#define index     0   /* Index Register */
#define random    1   /* Random Register */
#define entrylo0  2   /* EntryLo0 Register */
#define entrylo1  3   /* EntryLo1 Register */
#define context   4   /* Context Register */
#define pagemask  5   /* PageMask Register */
#define wired     6   /* Wired Register */
#define badvaddr  8   /* Bad Virtual Address Register */
#define count     9   /* Count Register */
#define entryhi   10  /* EntryHi Register */
#define compare   11  /* Compare Register */
#define status    12  /* Status Register */
#define cause     13  /* Cause Register */
#define epc       14  /* Exception Program Counter Register */
#define config    16  /* Config Register */
#define lladdr    17  /* Load Linked Address Register */
#define xcontext  20  /* XContext Register */
#define eepc      30  /* Error Exception Program Counter Register */


/***************************************************************************\
| Special Instructions                                                      |
\***************************************************************************/


/*
 * The simulator introduces some new instructions that are useful for
 * debugging. These instructions use opcodes that are unused in the
 * real processor. For ease of use, macros that emit the opcodes
 * are defined here:
 *
 *  - halt makes the simulator terminate
 *  - stop interrupts the simulation temporarily
 *  - regs displays the content of the general registers
 *  - traceon turns on the trace mode
 *  - traceoff turns off the trace mode
 *
 */

.macro halt
	.insn
	.word 0x28
	
	/*
	 * This code should be unreachable since
	 * the simulator is already halted.
	 *
	 * However, be extremely paranoid here
	 * and really make sure that the CPU
	 * won't execute any random code past
	 * this point in the case the hell
	 * freezes over.
	 */
	
	0:
		j 0b
		nop
	
.endm halt

.macro stop
	.insn
	.word 0x29
.endm stop

.macro regs
	.insn
	.word 0x37
.endm regs

.macro traceon
	.insn
	.word 0x39
.endm traceon

.macro traceoff
	.insn
	.word 0x3D
.endm traceoff


/***************************************************************************\
| Context Manipulation                                                      |
\***************************************************************************/

/*
 * A pair of macros is used to save and restore the general registers
 * in several places. The offsets used in the macros are introduced
 * in the common definitions file.
 *
 * The macros do not save and restore the K0 and K1 registers, since
 * these are reserved for use by the kernel code, and therefore do
 * not contain application context.
 *
 * The macros also do not save and restore the stack pointer, which
 * is handled separately during context switching, and the LO and
 * HI registers, which would require temporary storage.
 *
 * The macros do save the ZERO register, leaving some space for
 * optimization should people become bored :-) ...
 */

/*
 * SAVE_REGISTERS
 */

.macro SAVE_REGISTERS base
	sw $zero, REGS_OFFSET_ZERO(\base)
	
	sw $at,   REGS_OFFSET_AT(\base)
	
	sw $v0,   REGS_OFFSET_V0(\base)
	sw $v1,   REGS_OFFSET_V1(\base)
	
	sw $a0,   REGS_OFFSET_A0(\base)
	sw $a1,   REGS_OFFSET_A1(\base)
	sw $a2,   REGS_OFFSET_A2(\base)
	sw $a3,   REGS_OFFSET_A3(\base)
	
	sw $t0,   REGS_OFFSET_T0(\base)
	sw $t1,   REGS_OFFSET_T1(\base)
	sw $t2,   REGS_OFFSET_T2(\base)
	sw $t3,   REGS_OFFSET_T3(\base)
	sw $t4,   REGS_OFFSET_T4(\base)
	sw $t5,   REGS_OFFSET_T5(\base)
	sw $t6,   REGS_OFFSET_T6(\base)
	sw $t7,   REGS_OFFSET_T7(\base)
	sw $t8,   REGS_OFFSET_T8(\base)
	sw $t9,   REGS_OFFSET_T9(\base)
	
	sw $s0,   REGS_OFFSET_S0(\base)
	sw $s1,   REGS_OFFSET_S1(\base)
	sw $s2,   REGS_OFFSET_S2(\base)
	sw $s3,   REGS_OFFSET_S3(\base)
	sw $s4,   REGS_OFFSET_S4(\base)
	sw $s5,   REGS_OFFSET_S5(\base)
	sw $s6,   REGS_OFFSET_S6(\base)
	sw $s7,   REGS_OFFSET_S7(\base)
	
	sw $gp,   REGS_OFFSET_GP(\base)
	sw $fp,   REGS_OFFSET_FP(\base)
	sw $ra,   REGS_OFFSET_RA(\base)
.endm SAVE_REGISTERS

/*
 * LOAD_REGISTERS
 */

.macro LOAD_REGISTERS base
	lw $ra,   REGS_OFFSET_RA(\base)
	lw $fp,   REGS_OFFSET_FP(\base)
	lw $gp,   REGS_OFFSET_GP(\base)
	
	lw $s7,   REGS_OFFSET_S7(\base)
	lw $s6,   REGS_OFFSET_S6(\base)
	lw $s5,   REGS_OFFSET_S5(\base)
	lw $s4,   REGS_OFFSET_S4(\base)
	lw $s3,   REGS_OFFSET_S3(\base)
	lw $s2,   REGS_OFFSET_S2(\base)
	lw $s1,   REGS_OFFSET_S1(\base)
	lw $s0,   REGS_OFFSET_S0(\base)
	
	lw $t9,   REGS_OFFSET_T9(\base)
	lw $t8,   REGS_OFFSET_T8(\base)
	lw $t7,   REGS_OFFSET_T7(\base)
	lw $t6,   REGS_OFFSET_T6(\base)
	lw $t5,   REGS_OFFSET_T5(\base)
	lw $t4,   REGS_OFFSET_T4(\base)
	lw $t3,   REGS_OFFSET_T3(\base)
	lw $t2,   REGS_OFFSET_T2(\base)
	lw $t1,   REGS_OFFSET_T1(\base)
	lw $t0,   REGS_OFFSET_T0(\base)
	
	lw $a3,   REGS_OFFSET_A3(\base)
	lw $a2,   REGS_OFFSET_A2(\base)
	lw $a1,   REGS_OFFSET_A1(\base)
	lw $a0,   REGS_OFFSET_A0(\base)
	
	lw $v1,   REGS_OFFSET_V1(\base)
	lw $v0,   REGS_OFFSET_V0(\base)
	
	lw $at,   REGS_OFFSET_AT(\base)
	
	lw $zero, REGS_OFFSET_ZERO(\base)
.endm LOAD_REGISTERS


/***************************************************************************\
| SMP Support                                                               |
\***************************************************************************/

/*
 * A macro is used to get the address to the top of the static kernel
 * area which is valid for the current CPU. The value (minus \displacement)
 * is then stored into register \ptr.
 *
 * First the value from dorder device is read (which is the CPU ID ranging
 * from 0 to 31). Then this value is multiplied (by bitwise shifting) by
 * the size of the static area for a single CPU. This offset is added
 * to the address of the beginning of the static area (minus any possible
 * displacement). Finally, because stacks grow to lower addresses in memory,
 * the top of the area is calculated by adding the size again.
 *
 * The macro will clobber two registers (\temp0 and \temp1) and store
 * the calculated pointer to register \ptr, which might be the same
 * as \temp1. If it is different, \temp1 will hold the value read from the
 * dorder device.
 *
 * The \displacement has to be a constant.
 *
 */

/*
 * SETUP_STATIC_AREA
 */

.macro SETUP_STATIC_AREA temp0 temp1 ptr displacement
	/* Read the dorder value. */
	la \temp0, ADDR_IN_KSEG1 (DEVICE_DORDER_ADDR)
	lw \temp1, (\temp0)
	
	/*
	 * Calculate offset in the statically allocated
	 * area according to the CPU number.
	 */
	sll \temp0, \temp1, KERNEL_STATIC_SHIFT
	
	/* Set the statically allocated area. */
	la \ptr, ADDR_IN_KSEG0 (KERNEL_STATIC_ADDR) - \displacement
	addu \ptr, \ptr, \temp0
	
	/*
	 * Stack grows to lower addresses, thus
	 * we need to add stack size to get stack top.
	 */
	la \temp0, KERNEL_STATIC_SIZE
	addu \ptr, \ptr, \temp0
.endm SET_STATIC_STACK


#endif
