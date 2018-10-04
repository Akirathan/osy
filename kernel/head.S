/*
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 * Essential assembly routines of the kernel.
 *
 * Contains the kernel entry point, the exception handlers,
 * the temporary stack and the temporary variables. This
 * code usually only needs to be touched if multiple
 * processor support is required.
 *
 */

/*
 * The bootstrap and exception handling code requires having an absolute
 * control over the processor, we therefore include some directives that
 * prevent the compiler from being too smart with the assembly instructions:
 *
 *  - noat prevents the compiler from using the $at register in macros
 *  - noreorder prevents the compiler from reordering instructions
 *
 * Note that it is not generally safe to disable reordering within C code!
 */

.set noat
.set noreorder


#include <include/shared.h>
#include <include/asm.h>


/***************************************************************************\
| Code at hardwired addresses                                               |
\***************************************************************************/

/*
 * Put the hardwired code to a special input section ".excvec".
 * This section is then placed by the linker (instructed by the
 * kernel.lds linker script) at the very beginning of the kernel
 * image.
 */

.section .excvec, "ax"


/*
 * The exception handlers are placed at the hardwired addresses.
 * Rather than trying to fit the code in between the hardwired
 * addresses, the handlers simply jump away to the real code.
 */


/*
 * TLB Refill exception
 */

.org   ADDR_OFFSET (HANDLER_TLB_REFILL)
.globl exception_tlb_refill
.ent   exception_tlb_refill

exception_tlb_refill:
	j handle_tlb_refill
	nop

.end   exception_tlb_refill


/*
 * Cache error exception
 */

.org   ADDR_OFFSET (HANDLER_CACHE_ERROR)
.globl exception_cacheerror
.ent   exception_cacheerror

exception_cacheerror:
	j handle_cacheerror
	nop

.end   exception_cacheerror


/*
 * General exception
 */

.org   ADDR_OFFSET (HANDLER_GENERAL_EXCEPTION)
.globl exception_general
.ent   exception_general

exception_general:
	j handle_general
	nop

.end   exception_general


/***************************************************************************\
| Kernel entry point                                                        |
\***************************************************************************/


/*
 * Kernel entry point
 *
 * The bootstrap loader jumps here. The kernel initialiazes
 * the stack pointer register and the global pointer
 * register and passes control to compiled C code.
 */

.org   ADDR_OFFSET (KERNEL_ENTRY_POINT)
.globl start
.ent   start

start:
	/*
	 * Get kernel static area for the current CPU
	 * and use it as a temporal stack. Also reuse
	 * the current CPU ID in $k1.
	 */
	SETUP_STATIC_AREA $k0 $k1 $sp 0
	
	/* Set the global pointer to the kernel segment. */
	la $gp, ADDR_IN_KSEG0 (0)
	
	/* Continue either to BSP code or AP code. */
	bne $k1, $zero, ap
	nop
	
	/* Continue in compiled C BSP code. */
	jal bsp_start
	addiu $sp, -ABI_STACK_FRAME
	addiu $sp, ABI_STACK_FRAME
	
	/* Not reached. */
	halt
	
	ap:
		/* Continue in compiled C AP code. */
		jal ap_start
		addiu $sp, -ABI_STACK_FRAME
		addiu $sp, ABI_STACK_FRAME
		
		/* Not reached. */
		halt

.end start


/*
 * After the kernel entry point, a space for temporary
 * variables and temporary stacks (for each CPU) is made.
 *
 * Having the variables and the stacks allocated statically
 * makes the interrupt and exception handling simpler, but
 * remember that fully preemptive exception handling need
 * special care to make sure that any race conditions are
 * avoided.
 */

.org   ADDR_OFFSET (KERNEL_STATIC_AREA)
.globl static_area
.ent   static_area

static_area:
	.space KERNEL_STATIC_TOTAL

.end static_area


/***************************************************************************\
| Code at arbitrary addresses                                               |
\***************************************************************************/

.text


/***************************************************************************\
| Exception handlers                                                        |
\***************************************************************************/


/*
 * TLB Refill exception handler
 *
 * The handler code saves all registers and passes control to compiled C code.
 */

.ent handle_tlb_refill

handle_tlb_refill:
	
	/*
	 * To avoid writing to user stack, we use the static kernel
	 * area initialized by the SETUP_STATIC_AREA macro to store
	 * the context and as a temporary stack. Using the $k1 register
	 * as a pointer allows us to save the $sp register first.
	 *
	 * Note that using $k0 and $k1 here means $k0 and $k1
	 * must not be used anywhere else where there is a
	 * chance of the TLB Refill Exception occuring!
	 *
	 * We allocate a frame on the temporary stack and store the
	 * current stack pointer on the stack. After that, we
	 * can use $sp as the stack pointer.
	 *
	 * It is also important to realize that the TLB Refill
	 * exception handler is not reentrant (because it
	 * always uses the same portion of the temporary
	 * kernel area for the given CPU). This is not an issue
	 * since nested TLB Refill handing would be a bad idea
	 * anyway.
	 *
	 * On the other hand, TLB Refill exception handler
	 * might preempt the General exception handler. Thus,
	 * we must be sure that both handlers always use
	 * different parts of the kernel static space to avoid
	 * any collisions and that the TLB Refill exception
	 * cannot occur during the time when the General
	 * exception handler uses $k0 and $k1.
	 */
	
	SETUP_STATIC_AREA $k0 $k1 $k1 CONTEXT_SIZE
	
	sw $sp, REGS_OFFSET_SP($k1)
	or $sp, $0, $k1
	
	/* Save the general registers and the $lo and $hi registers. */
	
	SAVE_REGISTERS $sp
	mflo $t0
	mfhi $t1
	sw $t0, REGS_OFFSET_LO($sp)
	sw $t1, REGS_OFFSET_HI($sp)
	
	/*
	 * Save the contents of the EPC, Status, BadVAddr, Cause and EntryHi
	 * registers of the System Control Coprocessor. The C code could
	 * read the values of these registers directly, but we do it here
	 * in order to achieve symmetry between the TLB Refill exception
	 * handler the General Exception handler.
	 */
	
	mfc0 $k0, $epc
	sw $k0, REGS_OFFSET_EPC($sp)
	mfc0 $k0, $cause
	sw $k0, REGS_OFFSET_CAUSE($sp)
	mfc0 $k0, $badvaddr
	sw $k0, REGS_OFFSET_BADVA($sp)
	mfc0 $k0, $entryhi
	sw $k0, REGS_OFFSET_ENTRYHI($sp)
	mfc0 $k0, $status
	sw $k0, REGS_OFFSET_STATUS($sp)
	
	/*
	 * Call the exception handler in compiled C code, passing
	 * the pointer to the stack frame as an argument.
	 */
	
	or $a0, $0, $sp
	jal wrapped_tlb_refill
	addiu $sp, -ABI_STACK_FRAME
	addiu $sp, ABI_STACK_FRAME
	
	/* Restore the $lo and $hi registers and the general registers. */
	
	lw $t0, REGS_OFFSET_LO($sp)
	lw $t1, REGS_OFFSET_HI($sp)
	mtlo $t0
	mthi $t1
	LOAD_REGISTERS $sp
	
	/* Restore the stack pointer last and return from exception. */
	
	lw $sp, REGS_OFFSET_SP($sp)
	eret

.end handle_tlb_refill


/*
 * Cache Error exception handler
 *
 * The handler halts since this exception should not occur.
 */

.ent handle_cacheerror

handle_cacheerror:
	halt

.end handle_cacheerror


/*
 * General Exception handler
 *
 * The handler code saves all registers and passes control to compiled C code.
 */

.ent handle_general

handle_general:
	
	/*
	 * Save the contents of the EPC, Status, BadVAddr, Cause and EntryHi
	 * registers of the System Control Coprocessor to the static
	 * kernel area. These registers contain information on the
	 * exception that we are handling, and the information
	 * must be saved in case another exception comes while we are
	 * inside this handler.
	 *
	 * The code can trash the $k0 and $k1 registers, which are
	 * reserved for kernel use. All other registers are preserved.
	 * Note that using $k0 and $k1 here means $k0 and $k1 must not
	 * be used anywhere else where there is a chance of the General
	 * Exception occuring!
	 */
	
	SETUP_STATIC_AREA $k0 $k1 $k1 KERNEL_STATIC_SIZE
	
	mfc0 $k0, $epc
	sw $k0, STATIC_OFFSET_EPC($k1)
	mfc0 $k0, $cause
	sw $k0, STATIC_OFFSET_CAUSE($k1)
	mfc0 $k0, $badvaddr
	sw $k0, STATIC_OFFSET_BADVA($k1)
	mfc0 $k0, $entryhi
	sw $k0, STATIC_OFFSET_ENTRYHI($k1)
	mfc0 $k0, $status
	sw $k0, STATIC_OFFSET_STATUS($k1)
	
	/*
	 * On entering the handler, the EXL bit in the Status register of
	 * the System Control Coprocessor is set. This bit indicates that
	 * the processor is just beginning to handle an exception, and
	 * that nested exceptions require special handling.
	 *
	 * Now that we have saved the registers of the System Control
	 * Coprocessor, we can clear the EXL bit in the Status register,
	 * but to avoid nested exceptions, we also disable interrupts by
	 * masking the IE bit of the Status register, and we make sure we
	 * stay in the kernel mode by masking the KSU bits of the Status
	 * register.
	 */
	
	la $k1, ~(CP0_STATUS_KSU_MASK | CP0_STATUS_EXL_MASK | CP0_STATUS_IE_MASK)
	and $k0, $k1
	mtc0 $k0, $status
	
	/*
	 * The rest of the registers is saved on the current stack. Note
	 * that when the stack resides in virtual memory, this can cause
	 * the TLB Refill Exception, which means that we must not use
	 * the kernel scratch registers $k0 and $k1 except for
	 * calculations performed only in registers and accessing
	 * the unmapped memory regions.
	 */
	
	addiu $sp, -CONTEXT_SIZE
	
	SAVE_REGISTERS $sp
	mflo $t0
	mfhi $t1
	sw $t0, REGS_OFFSET_LO($sp)
	sw $t1, REGS_OFFSET_HI($sp)
	
	/*
	 * Also move the previously stored System Control Coprocessor
	 * registers from the statically allocated variables to the
	 * stack. Once this is done, the handler becomes reentrant
	 * and we can enable interrupts.
	 */
	
	SETUP_STATIC_AREA $k0 $k1 $t1 KERNEL_STATIC_SIZE
	
	lw $t0, STATIC_OFFSET_EPC($t1)
	sw $t0, REGS_OFFSET_EPC($sp)
	lw $t0, STATIC_OFFSET_CAUSE($t1)
	sw $t0, REGS_OFFSET_CAUSE($sp)
	lw $t0, STATIC_OFFSET_BADVA($t1)
	sw $t0, REGS_OFFSET_BADVA($sp)
	lw $t0, STATIC_OFFSET_ENTRYHI($t1)
	sw $t0, REGS_OFFSET_ENTRYHI($sp)
	lw $t0, STATIC_OFFSET_STATUS($t1)
	sw $t0, REGS_OFFSET_STATUS($sp)
	
	/*
	 * Call the exception handler in compiled C code, passing
	 * the pointer to the stack frame as an argument.
	 */
	
	or $a0, $0, $sp
	jal wrapped_general
	addiu $sp, -ABI_STACK_FRAME
	addiu $sp, ABI_STACK_FRAME
	
	/*
	 * Now the exception has hopefully been handled and all
	 * that remains is returning to the interrupted code.
	 * For that, we load the saved registers back first.
	 *
	 * Remember that here the code cannot be reentrant anymore,
	 * because we are using the kernel static area again.
	 * If interrupts were enabled previously, they need
	 * to be disabled here.
	 *
	 * The TLB Refill Exception due to accessing unmapped memory
	 * can happen here as well, which means that we must not use
	 * the kernel scratch registers $k0 and $k1 except for
	 * calculations performed only in registers and accessing
	 * the unmapped memory regions.
	 */
	
	SETUP_STATIC_AREA $k0 $k1 $t1 KERNEL_STATIC_SIZE
	
	lw $t0, REGS_OFFSET_STATUS($sp)
	sw $t0, STATIC_OFFSET_STATUS($t1)
	lw $t0, REGS_OFFSET_EPC($sp)
	sw $t0, STATIC_OFFSET_EPC($t1)
	
	lw $t0, REGS_OFFSET_LO($sp)
	lw $t1, REGS_OFFSET_HI($sp)
	mtlo $t0
	mthi $t1
	LOAD_REGISTERS $sp
	
	/*
	 * After the saved registers have been loaded, release
	 * the stack frame that has been used to preserve them.
	 */
	
	addiu $sp, CONTEXT_SIZE
	
	/*
	 * Finally, restore the System Control Coprocessor registers
	 * and return from the exception. The ERET instruction also
	 * restores the processor status.
	 */
	
	SETUP_STATIC_AREA $k0 $k1 $k1 KERNEL_STATIC_SIZE
	
	lw $k0, STATIC_OFFSET_EPC($k1)
	mtc0 $k0, $epc
	lw $k0, STATIC_OFFSET_STATUS($k1)
	mtc0 $k0, $status
	
	eret

.end handle_general


/***************************************************************************\
| Context switching                                                         |
\***************************************************************************/


/*
 * cpu_switch_context
 *
 * Switches processor context from one thread to another. The first argument
 * points to a pointer to the top of the stack of the old thread, the
 * second argument points to a pointer to the top of the stack of
 * the new thread.
 *
 * The pointer to the old stack is written, the pointer to the new stack
 * is read. Both are passed as pointers to handle the weird case where
 * the old thread and the new thread are identical, because then,
 * we would not know the address of the new stack before it
 * is written as the address of the old stack.
 *
 * The third argument contains the address space identifier of the thread
 * we are switching to. The ASID value is set to the EntryHi register after
 * the context of the old thread is saved, but before the context of the
 * new thread is loaded.
 */

.globl cpu_switch_context
.ent   cpu_switch_context

cpu_switch_context:

	/*
	 * Allocate a frame on the stack of the old thread and update
	 * the address of the stack top of the old thread.
	 */
	
	addiu $sp, -CONTEXT_SIZE
	sw $sp, ($a0)
	
	/*
	 * Store the general registers and the $hi, $lo and Status
	 * registers on the stack. Disable interrupts after
	 * storing the Status register.
	 */
	
	SAVE_REGISTERS $sp
	mflo $t0
	mfhi $t1
	sw $t0, REGS_OFFSET_LO($sp)
	sw $t1, REGS_OFFSET_HI($sp)
	
	mfc0 $t0, $status
	sw $t0, REGS_OFFSET_STATUS($sp)
	la $t1, ~CP0_STATUS_IE_MASK
	and $t0, $t1
	mtc0 $t0, $status
	
	/*
	 * Switch to the address space of the new thread.
	 */
	
	mfc0 $t0, $entryhi
	la $t1, ~CP0_ENTRYHI_ASID_MASK
	and $t0, $t1
	or $t0, $a2
	mtc0 $t0, $entryhi
	
	/*
	 * Switch to the stack of the new thread. The stack of the
	 * new thread should be in the same state as the stack of
	 * the old thread.
	 */
	
	lw $sp, ($a1)
	
	/* Load the general registers and the $hi and $lo registers from the stack. */
	
	lw $t0, REGS_OFFSET_LO($sp)
	lw $t1, REGS_OFFSET_HI($sp)
	mtlo $t0
	mthi $t1
	LOAD_REGISTERS $sp
	
	/*
	 * Prepare to restore the Status register. The Status register is
	 * restored in the branch delay slot of the jump that returns
	 * control to the new thread.
	 *
	 * Setting the Status register in the branch delay slot makes it
	 * possible to return from kernel mode to user mode. Setting the
	 * register sooner would mean switching from kernel mode to user
	 * mode while executing in KSEG0, which is not allowed.
	 *
	 * A somewhat cleaner alternative to this particular method of
	 * returning from kernel mode to user mode is the ERET instruction.
	 */
	
	lw $k0, REGS_OFFSET_STATUS($sp)
	addiu $sp, CONTEXT_SIZE
	
	/*
	 * Note that from now on, an interrupt or an exception here would
	 * trash the content of the $k0 and $k1 registers. This is why we
	 * have disabled interrupts earlier, and do not touch anything
	 * in memory.
	 */
	
	j $ra
	mtc0 $k0, $status

.end cpu_switch_context


/*
 * cpu_uspace_jump
 *
 * Switch to user space by the means of the ERET instruction.
 * We assume that we are running in the Exception level (thus
 * the interrupts are currently disabled) and the EPC register
 * contains the target address of the user space instruction
 * to jump to.
 *
 * The first argument contains the address to the user space
 * stack that we set as the stack pointer just before jumping
 * to the user space. The second and third arguments contain
 * the real first and second argument of the user space thread
 * entry function.
 */

.globl cpu_uspace_jump
.ent   cpu_uspace_jump

cpu_uspace_jump:

	move $sp, $a0
	move $a0, $a1
	move $a1, $a2
	eret

.end cpu_uspace_jump
