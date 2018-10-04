/**
 * @file exc.c
 *
 * Exception handling.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <include/shared.h>
#include <include/c.h>

#include <exc/int.h>
#include <exc/syscall.h>
#include <lib/print.h>
#include <mm/tlb.h>

#include <exc/exc.h>


/** Handle an exception
 *
 * The function is called from the assembler exception handler.
 * Interrupts are disabled and the various System Control
 * Coprocessor registers identify what exception is
 * being handled.
 *
 * @param registers The registers at the time of the exception.
 *
*/
void wrapped_general (context_t *registers)
{
	/* The handling of the exception depends on its cause */
	switch (CP0_CAUSE_EXCCODE (registers->cause)) {
	case CP0_CAUSE_EXCCODE_INT:
		/*
		 * Interrupt Exception:
		 * Interrupts are handled by a separate procedure.
		 */
		interrupt (registers);
		break;
	case CP0_CAUSE_EXCCODE_SYS:
		/*
		 * System Call Exception:
		 * System calls are handled by a separate procedure.
		 */
		syscall (registers);
		break;
	case CP0_CAUSE_EXCCODE_TLBL:
	case CP0_CAUSE_EXCCODE_TLBS:
		tlb_invalid (registers);
		break;
	case CP0_CAUSE_EXCCODE_ADEL:
	case CP0_CAUSE_EXCCODE_ADES:
		/*
		 * Address Error Exceptions:
		 * Code accessed an invalid address.
		 *
		 * Something is very wrong, we basically just halt.
		 */
		panic ("Unhandled Address Error exception.");
		break;
	case CP0_CAUSE_EXCCODE_BP:
	case CP0_CAUSE_EXCCODE_TR:
	case CP0_CAUSE_EXCCODE_OV:
		/*
		 * Debugging Exceptions:
		 * Debugging exceptions are used to facilitate debugging.
		 *
		 * The Breakpoint Exception is trigged when the BREAK instruction
		 * is executed. The BREAK instruction can be inserted in code
		 * to implement location breakpoints.
		 *
		 * The Trap Exception is triggered when the condition of the
		 * TRAP instruction is met. The TRAP instruction can be
		 * used in code to implement range checking.
		 *
		 * The Overflow Exception is triggered when an arithmetic
		 * overflow occurs in signed arithmetic instructions.
		 *
		 * None of the exceptions is used here.
		 */
		panic ("Unhandled Debugging exception.");
		break;
	
	/*
	 * Some other exceptions can occur but mostly they just
	 * mean that something is terribly wrong and the kernel
	 * halts.
	 */
	
	case CP0_CAUSE_EXCCODE_CPU:
		panic ("Unhandled Coprocessor Unusable exception.");
		break;
	case CP0_CAUSE_EXCCODE_RI:
		panic ("Unhandled Reserved Instruction exception.");
		break;
	default:
		panic ("Unhandled exception %u.", CP0_CAUSE_EXCCODE (registers->cause));
	}
}
