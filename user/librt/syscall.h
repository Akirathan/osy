/**
 * @file syscall.h
 *
 * User space syscall support.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */


#ifndef LIBRT_SYSCALL_H_
#define LIBRT_SYSCALL_H_


#include <types.h>


/** System call types
 *
 */
typedef enum {
	SYS_EXIT = 0,
	SYS_PUTC,
	SYS_PUTSTR,
	SYS_GETC,
	SYS_VMA_MAP,
	SYS_VMA_UNMAP,
	SYS_THREAD_CREATE,
	SYS_THREAD_SELF,
	SYS_THREAD_USLEEP,
	SYS_THREAD_JOIN,
	SYS_THREAD_FINISH,
	SYS_MUTEX_INIT,
	SYS_MUTEX_LOCK,
	SYS_MUTEX_UNLOCK,
	SYS_MUTEX_DESTROY
} syscall_t;


/*
 * Convenient macros for system calls with given
 * number of arguments.
 */

#define SYSCALL0(id) \
	syscall(id, 0, 0, 0, 0)

#define SYSCALL1(id, p1) \
	syscall(id, p1, 0, 0, 0)

#define SYSCALL2(id, p1, p2) \
	syscall(id, p1, p2, 0, 0)

#define SYSCALL3(id, p1, p2, p3) \
	syscall(id, p1, p2, p3, 0)

#define SYSCALL4(id, p1, p2, p3, p4) \
	syscall(id, p1, p2, p3, p4)


/** System call
 *
 * Run a system call by passing the system call type
 * and its arguments in registers.
 *
 * @param id System call type.
 * @param p1 First system call argument.
 * @param p2 Second system call argument.
 * @param p3 Third system call argument.
 * @param p4 Fourth system call argument.
 *
 * @return System call return/error value.
 *
 */
static inline unative_t syscall(const syscall_t id, const unative_t p1,
    const unative_t p2, const unative_t p3, const unative_t p4)
{
	/*
	 * Pass arguments directly in registers.
	 */
	register unative_t mips_reg_v0 asm("$2") = id;
	register unative_t mips_reg_a0 asm("$4") = p1;
	register unative_t mips_reg_a1 asm("$5") = p2;
	register unative_t mips_reg_a2 asm("$6") = p3;
	register unative_t mips_reg_a3 asm("$7") = p4;
	
	asm volatile (
		"syscall\n"
		: "=r" (mips_reg_v0)
		: "r" (mips_reg_v0),
		  "r" (mips_reg_a0),
		  "r" (mips_reg_a1),
		  "r" (mips_reg_a2),
		  "r" (mips_reg_a3)
		/*
		 * We are a function call, although C
		 * does not know it.
		 */
		: "%ra"
	);
	
	return mips_reg_v0;
}


#endif
