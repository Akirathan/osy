/*
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 * System bootstrap code
 *
 * This is the module that gets executed as the processor powers up. The
 * processor is hardwired to start executing at the virtual address
 * 0xBFC00000, which maps to the physical address 0x1FC00000.
 *
 * The bootstrap code resides in segment KSEG1, which has both address
 * translation and memory caching disabled. This makes it possible to
 * prepare the address translation structures before address
 * translation is enabled.
 *
 * The only thing that the module does is jumping to the real kernel code.
 * The real kernel occupies segment KSEG0 from address 0x8000000, where
 * the hardwired addresses of the exception handlers are. Theoretically,
 * the real kernel code could be here, but having everything in one
 * place makes things a tiny bit simpler.
 *
 * When the processor powers up, the content of all general registers is
 * undefined. The BEV and EXL bits in the Status Register are set to 1,
 * which means that the processor operates in kernel mode with
 * interrupts disabled.
 *
 */

.text

/*
 * The bootstrap code requires having an absolute control over the processor,
 * we therefore include some directives that prevent the compiler from being
 * too smart with the assembly instructions:
 *
 *  - noat prevents the compiler from using the $at register in macros
 *  - noreorder prevents the compiler from reordering instructions
 *
 */

.set noat
.set noreorder


#include <include/shared.h>
#include <include/asm.h>


.ent __start

__start:
	
	/*
	 * Just jump to the real kernel code straight away.
	 *
	 * If you are tracing this code, you might be surprised to see
	 * that the LA instruction is not a machine code instruction.
	 * The processor does not implement instructions that load
	 * a 32 bit constant to a register, a pair of instructions
	 * that load the constant in two 16 bit parts is used
	 * instead.
	 *
	 */
	
	la $ra, KERNEL_ENTRY_POINT
	j $ra
	nop
	
	/*
	 * The NOP is attached to the code sequence to make sure that
	 * no other instruction is executed in the branch delay slot
	 * of the jump.
	 *
	 */

.end __start
