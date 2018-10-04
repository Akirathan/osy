/**
 * @file c.h
 *
 * C definitions file.
 *
 * This file groups definitions that are required by multiple files
 * and whose syntax permits them to be used both in assembler and in C.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef DEFINES_C_H_
#define DEFINES_C_H_


/***************************************************************************\
| Error codes                                                               |
\***************************************************************************/

#define EOK          0       /* Everything's OK */
#define EAGAIN       -11     /* Try again */
#define ENOMEM       -12     /* Out of memory */
#define EINVAL       -22     /* Invalid argument */
#define ENOSYS       -38     /* Function not implemented */
#define ETIMEDOUT    -145    /* Connection timed out */
#define EKILLED      -666    /* Thread was killed */


/***************************************************************************\
| Basics                                                                    |
\***************************************************************************/

#define QUOTE_ME_(x) #x
#define QUOTE_ME(x) QUOTE_ME_(x)

/*
 * All right, not everything
 * needs to be commented :-)
 */
#define false  0
#define true   (!false)

/* An unused pointer value. */
#define NULL  0

/** Align to the nearest lower address
 *
 * @param size  Address or size to be aligned.
 * @param align Size of alignment, must be power of 2.
 *
 */
#define ALIGN_DOWN(size, align)  ((size) & ~((align) - 1))

/** Align to the nearest higher address
 *
 * @param size  Address or size to be aligned.
 * @param align Size of alignment, must be power of 2.
 *
 */
#define ALIGN_UP(size, align)  (((size) + ((align) - 1)) & ~((align) - 1))

/** Size of an array in array elements
 *
 */
#define sizeof_array(array) \
	(sizeof (array) / sizeof ((array)[0]))


/** Basic platform types */
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed long int32_t;
typedef unsigned long uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int32_t native_t;
typedef uint32_t unative_t;
typedef uint32_t uintptr_t;
typedef uint32_t off_t;
typedef uint32_t size_t;
typedef int32_t ssize_t;
typedef uint8_t bool;

typedef unative_t ipl_t;
typedef uint8_t asid_t;


/***************************************************************************\
| Externals                                                                 |
\***************************************************************************/

/** Switch processor thread context
 *
 */
extern void cpu_switch_context (void **stack_top_old, void **stack_top_new,
    asid_t asid_new);


/** Switch processor mode to user space
 *
 */
extern void cpu_uspace_jump (void *user_stack, void *data, void *user_data);


/***************************************************************************\
| Simple Random Generator                                                   |
\***************************************************************************/

/** Generate a next random number
 *
 */
static inline unsigned int random (unsigned int *seed)
{
	*seed = ((*seed) * 873511) % 22348977 + 7;
	return ((*seed) >> 8);
}


/***************************************************************************\
| Kernel Data Structures                                                    |
\***************************************************************************/


/** Exception stack frame
 *
 * During interrupt and exception handling, the handler stores the
 * content of the registers in memory. The following definition
 * describe the structure.
 *
 */

typedef struct {
	unative_t zero;
	
	unative_t at;
	unative_t v0;
	unative_t v1;
	
	unative_t a0;
	unative_t a1;
	unative_t a2;
	unative_t a3;
	
	unative_t t0;
	unative_t t1;
	unative_t t2;
	unative_t t3;
	unative_t t4;
	unative_t t5;
	unative_t t6;
	unative_t t7;
	unative_t t8;
	unative_t t9;
	
	unative_t s0;
	unative_t s1;
	unative_t s2;
	unative_t s3;
	unative_t s4;
	unative_t s5;
	unative_t s6;
	unative_t s7;
	
	unative_t k0;
	unative_t k1;
	
	unative_t gp;
	unative_t fp;
	
	unative_t sp;
	unative_t ra;
	
	unative_t lo;
	unative_t hi;
	
	unative_t epc;
	unative_t cause;
	unative_t badva;
	unative_t entryhi;
	unative_t status;
} context_t;


/***************************************************************************\
| Processor Related Definitions                                             |
\***************************************************************************/

/*
 * Special register access
 *
 * These functions are used to manipulate the special system registers
 * of the MIPS processor. These are accessed via special instructions,
 * there is no standard way of doing that in the C language.
 *
 * The syntax used to return a value is a compound expression C extension.
 * Alternatively, an inline function could also be declared by the macro.
 *
 */


/** Read a CP0 register by its number. */
#define read_cp0_register(number) \
	({ \
		unative_t __result; \
		asm volatile ( \
			".set push\n" \
			".set noreorder\n" \
			"nop\n" \
			"mfc0 %0, $"#number"\n" \
			".set pop\n" \
			: "=r" (__result) \
		); \
		__result; \
	})

#define read_cp0_index()     read_cp0_register (0)
#define read_cp0_badvaddr()  read_cp0_register (8)
#define read_cp0_count()     read_cp0_register (9)
#define read_cp0_entryhi()   read_cp0_register (10)
#define read_cp0_compare()   read_cp0_register (11)
#define read_cp0_status()    read_cp0_register (12)
#define read_cp0_cause()     read_cp0_register (13)
#define read_cp0_epc()       read_cp0_register (14)
#define read_cp0_xcontext()  read_cp0_register (20)
#define read_cp0_eepc()      read_cp0_register (30)


/** Write a CP0 register by its number. */
#define write_cp0_register(number, value) \
	asm volatile ( \
		".set push\n" \
		".set noreorder\n" \
		"nop\n" \
		"mtc0 %0, $"#number"\n" \
		".set pop\n" \
		: \
		: "r" (value) \
	)

#define write_cp0_index(val)     write_cp0_register (0, val)
#define write_cp0_entrylo0(val)  write_cp0_register (2, val)
#define write_cp0_entrylo1(val)  write_cp0_register (3, val)
#define write_cp0_pagemask(val)  write_cp0_register (5, val)
#define write_cp0_wired(val)     write_cp0_register (6, val)
#define write_cp0_count(val)     write_cp0_register (9, val)
#define write_cp0_entryhi(val)   write_cp0_register (10, val)
#define write_cp0_compare(val)   write_cp0_register (11, val)
#define write_cp0_status(val)    write_cp0_register (12, val)
#define write_cp0_cause(val)     write_cp0_register (13, val)
#define write_cp0_epc(val)       write_cp0_register (14, val)
#define write_cp0_eepc(val)      write_cp0_register (30, val)


/***************************************************************************\
| Processor Related Functions                                               |
\***************************************************************************/

/*
 * Note that the meaning of static differs between C and C++ !
 */

/** Query and disable processor interrupts
 *
 * A special bit in the CP0 Status Register is cleared to disable interrupts.
 * The access to the Status Register is not atomic, but that should not
 * be a problem since the register is part of our context and thus
 * no other activity should modify it.
 *
 * The function is meant to be used in pair with conditionally_enable_interrupts:
 *
 * ipl_t state = query_and_disable_interrupts ();
 * ...
 * conditionally_enable_interrupts (state);
 *
 * @return Previous interrupt state.
 *
 */
static inline ipl_t query_and_disable_interrupts (void)
{
	ipl_t status = read_cp0_status ();
	write_cp0_status (status & ~CP0_STATUS_IE_MASK);
	return (status & CP0_STATUS_IE_MASK);
}


/** Disable processor interrupts
 *
 * A special bit in the CP0 Status Register is cleared to disable interrupts.
 * The access to the Status Register is not atomic, but that should not
 * be a problem since the register is part of our context and thus
 * no other activity should modify it.
 *
 */
static inline void disable_interrupts (void)
{
	write_cp0_status (read_cp0_status () & ~CP0_STATUS_IE_MASK);
}


/** Conditionally enable processor interrupts
 *
 * A special bit in the CP0 Status Register is set to enable interrupts.
 * The access to the Status Register is not atomic, but that should not
 * be a problem since the register is part of our context and thus
 * no other activity should modify it.
 *
 * The function is meant to be used in pair with query_and_disable_interrupts:
 *
 * ipl_t state = query_and_disable_interrupts ();
 * ...
 * conditionally_enable_interrupts (state);
 *
 * @param state Previous interrupt state.
 *
 */
static inline void conditionally_enable_interrupts (ipl_t state)
{
	if (state)
		write_cp0_status (read_cp0_status () | CP0_STATUS_IE_MASK);
}


/** Enable processor interrupts
 *
 * A special bit in the CP0 Status Register is set to enable interrupts.
 * The access to the Status Register is not atomic, but that should not
 * be a problem since the register is part of our context and thus
 * no other activity should modify it.
 *
 */
static inline void enable_interrupts (void)
{
	write_cp0_status (read_cp0_status () | CP0_STATUS_IE_MASK);
}


/** Enable simulation tracing
 *
 * With tracing enabled, the simulator will print
 * each instruction as it is executed.
 *
 */
static inline void msim_trace_on (void)
{
	asm volatile (
		".insn\n"
		".word 0x39\n"
	);
}


/** Disable simulation tracing
 *
 */
static inline void msim_trace_off (void)
{
	asm volatile (
		".insn\n"
		".word 0x3d\n"
	);
}


/** Display register content
 *
 * Instructs the simulator to dump the contents of
 * the general registers on the console.
 *
 */
static inline void msim_reg_dump (void)
{
	asm volatile (
		".insn\n"
		".word 0x37\n"
	);
}


/** Halt the simulation
 *
 * Instructs the simulator to terminate.
 *
 */
static inline __attribute__((noreturn)) void msim_halt (void)
{
	asm volatile (
		".insn\n"
		".word 0x28\n"
	);
	
	/* This code should be unreachable since
	   the simulator is already halted.
	   
	   However, be extremely paranoid here
	   and really make sure that the CPU
	   won't execute any random code past
	   this point in the case the hell
	   freezes over. */
	while (true);
}


/** Pause the simulation
 *
 * Instructs the simulator to enter interactive mode.
 *
 */
static inline void msim_stop (void)
{
	asm volatile (
		".insn\n"
		".word 0x29\n"
	);
}


#endif
