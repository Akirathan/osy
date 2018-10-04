/**
 * @file sched.c
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 * The assembler code in Kalisto (i.e. functions like
 * cpu_switch_context) uses contants for accessing the
 * members of the context_t structure.
 *
 * To make sure that the assembler code is using precise
 * offsets to the context_t structure, the offset constants
 * are generated from the C code definition of context_t.
 *
 * Let us illustrate the process on the offset of the member
 * v0 of context_t:
 *
 * 1. In C code below, the macro OFFSET_DECLARE ("V0", vo) is used.
 *
 * 2. The offset of the member v0 in context_t is computed using
 *    offset_of macro (say 8).
 *
 * 3. An inline assembler block is generated containing the line
 *
 *    #DECLARE	#define REGS_OFFSET_V0 8
 *
 * 4. The C code is not compiled into binary code, but only
 *    to assembler code. This assembler source code is then
 *    parsed by gen_offset.sh and a header file offset.h
 *    is created containing
 *
 *    #define REGS_OFFSET_V0 8
 *
 */


#include <include/shared.h>
#include <include/c.h>

/** Compute an offset of a member in a structure
 *
 * @param type   Type of the container struct a member is embedded in.
 * @param member Name of the member within the struct.
 *
 */
#define offset_of(type, member) ((size_t) &((type *) NULL)->member)

/** Create a #DECLARE inline assembler output
 *
 */
#define ASM_DECLARE(sym, val) \
	asm volatile ( \
		"\n#DECLARE#\t#define " sym " %0" \
		: \
		: "n" ((unative_t) (val)) \
	)

/** Create an offset inline assembler output for context_t
 *
 */
#define OFFSET_DECLARE(name, reg) \
	ASM_DECLARE("REGS_OFFSET_" name, offset_of (context_t, reg))

void main(int argc, char *argv[])
{
	/* Offsets of the members of context_t. */
	OFFSET_DECLARE ("ZERO", zero);
	
	OFFSET_DECLARE ("AT", at);
	OFFSET_DECLARE ("V0", v0);
	OFFSET_DECLARE ("V1", v1);
	
	OFFSET_DECLARE ("A0", a0);
	OFFSET_DECLARE ("A1", a1);
	OFFSET_DECLARE ("A2", a2);
	OFFSET_DECLARE ("A3", a3);
	
	OFFSET_DECLARE ("T0", t0);
	OFFSET_DECLARE ("T1", t1);
	OFFSET_DECLARE ("T2", t2);
	OFFSET_DECLARE ("T3", t3);
	OFFSET_DECLARE ("T4", t4);
	OFFSET_DECLARE ("T5", t5);
	OFFSET_DECLARE ("T6", t6);
	OFFSET_DECLARE ("T7", t7);
	OFFSET_DECLARE ("T8", t8);
	OFFSET_DECLARE ("T9", t9);
	
	OFFSET_DECLARE ("S0", s0);
	OFFSET_DECLARE ("S1", s1);
	OFFSET_DECLARE ("S2", s2);
	OFFSET_DECLARE ("S3", s3);
	OFFSET_DECLARE ("S4", s4);
	OFFSET_DECLARE ("S5", s5);
	OFFSET_DECLARE ("S6", s6);
	OFFSET_DECLARE ("S7", s7);
	
	OFFSET_DECLARE ("K0", k0);
	OFFSET_DECLARE ("K1", k1);
	
	OFFSET_DECLARE ("GP", gp);
	OFFSET_DECLARE ("FP", fp);
	
	OFFSET_DECLARE ("SP", sp);
	OFFSET_DECLARE ("RA", ra);
	
	OFFSET_DECLARE ("HI", hi);
	OFFSET_DECLARE ("LO", lo);
	
	OFFSET_DECLARE ("EPC", epc);
	OFFSET_DECLARE ("CAUSE", cause);
	OFFSET_DECLARE ("BADVA", badva);
	OFFSET_DECLARE ("STATUS", status);
	OFFSET_DECLARE ("ENTRYHI", entryhi);
	
	/* Size of the context_t structure. */
	ASM_DECLARE("CONTEXT_SIZE", sizeof (context_t));
}
