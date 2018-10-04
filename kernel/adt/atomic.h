/**
 * @file atomic.h
 *
 * Atomic data types.
 *
 * Basic primitives for atomic variables. The implementation here uses the
 * LL (load linked) and SC (store conditional) instruction pair to achieve
 * atomicity of certain operations.
 *
 * The atomic variable data type and the associated interface for
 * manipulating it has been largely inspired by the Linux kernel.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef ATOMIC_H_
#define ATOMIC_H_


/** Atomic variable
 *
 * The atomic data type is opaque to the user to prevent
 * access by other than atomic operations. It is also
 * small enough to be passed by value.
 *
 */
typedef struct {
	volatile native_t value;
} atomic_t;


/** Macros for more elegent declaration of global atomic variables */

#define ATOMIC_EXTERN(name)          extern atomic_t name
#define ATOMIC_DECLARE(name, value)  atomic_t name = { value }


/** Get the value of an atomic variable
 *
 * @param var The variable to read.
 *
 */
static inline native_t atomic_get (const atomic_t *var)
{
	/* Reading an integer is atomic on MIPS */
	return (var->value);
}


/** Set the value of an atomic variable
 *
 * @param var The variable to write.
 * @param val The value to write.
 *
 */
static inline void atomic_set (atomic_t *var, const native_t val)
{
	/* Writing an integer is atomic on MIPS */
	var->value = val;
}


/** Set the value of an atomic variable to 1
 *
 * @param var The variable to set.
 *
 * @return The original value of the variable.
 *
 */
static inline native_t atomic_test_and_set (atomic_t *var)
{
	/*
	 * This is an optimistic algorithm that keeps trying
	 * to set until the LL and SC pair of instructions
	 * succeeds, which means there was no other
	 * access to the same variable in the
	 * meantime.
	 */
	
	native_t orig, result;
	
	asm volatile (
		".set push\n"
		".set noreorder\n"
		
		"1: ll %[orig], %[value]\n"
		"   bnez %[orig], 2f\n"
		"   li %[result], %[num]\n"
		"   sc %[result], %[value]\n"
		"   beqz %[result], 1b\n"
		"   nop\n"
		
		"2: sync\n"
		
		".set pop\n"
		: [orig] "=&r" (orig),
		  [result] "=&r" (result),
		  [value] "+m" ((var)->value)
		: [num] "Ir" (1)
		: "memory"
	);
	
	return orig;
}


/** Add to the value of an atomic variable
 *
 * @param var The variable to add to.
 * @param num The value to add.
 * @return The new value of the variable.
 *
 */
static inline native_t atomic_add (atomic_t *var, const native_t num)
{
	/*
	 * This is an optimistic algorithm that keeps trying
	 * to add until the LL and SC pair of instructions
	 * succeeds, which means there was no other
	 * access to the same variable in the
	 * meantime.
	 */
	
	native_t orig, result;
	
	asm volatile (
		".set push\n"
		".set noreorder\n"
		
		"1: ll %[orig], %[value]\n"
		"   addu %[result], %[orig], %[num]\n"
		"   sc %[result], %[value]\n"
		"   beqz %[result], 1b\n"
		"   nop\n"
		
		"   addu %[result], %[orig], %[num]\n"
		"   sync\n"
		
		".set pop\n"
		: [orig] "=&r" (orig),
		  [result] "=&r" (result),
		  [value] "+m" ((var)->value)
		: [num] "Ir" (num)
		: "memory"
	);
	
	return result;
}


/** Add to the value of an atomic variable
 *
 * @param var The variable to add to.
 * @param num The value to add.
 * @return The original value of the variable.
 *
 */
static inline native_t atomic_post_add (atomic_t *var, const native_t num)
{
	/*
	 * This is an optimistic algorithm that keeps trying
	 * to add until the LL and SC pair of instructions
	 * succeeds, which means there was no other
	 * access to the same variable in the
	 * meantime.
	 */
	
	native_t orig, result;
	
	asm volatile (
		".set push\n"
		".set noreorder\n"
		
		"1: ll %[orig], %[value]\n"
		"   addu %[result], %[orig], %[num]\n"
		"   sc %[result], %[value]\n"
		"   beqz %[result], 1b\n"
		"   nop\n"
		
		"   sync\n"
		
		".set pop\n"
		: [orig] "=&r" (orig),
		  [result] "=&r" (result),
		  [value] "+m" ((var)->value)
		: [num] "Ir" (num)
		: "memory"
	);
	
	return orig;
}


/** Subtract from the value of an atomic variable
 *
 * @param var The variable to subtract from.
 * @param num The value to subtract.
 * @return The new value of the variable.
 *
 */
static inline native_t atomic_sub (atomic_t *var, const native_t num)
{
	/*
	 * This is an optimistic algorithm that keeps trying
	 * to subtract until the LL and SC pair of instructions
	 * succeeds, which means there was no other access to the
	 * same variable in the meantime.
	 */
	
	native_t orig, result;
	
	asm volatile (
		".set push\n"
		".set noreorder\n"
		
		"1: ll %[orig], %[value]\n"
		"   subu %[result], %[orig], %[num]\n"
		"   sc %[result], %[value]\n"
		"   beqz %[result], 1b\n"
		"   nop\n"
		
		"   subu %[result], %[orig], %[num]\n"
		"   sync\n"
		
		".set pop\n"
		: [orig] "=&r" (orig),
		  [result] "=&r" (result),
		  [value] "+m" ((var)->value)
		: [num] "Ir" (num)
		: "memory"
	);
	
	return result;
}


#endif
