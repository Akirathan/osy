/**
 * @file tlb.c
 *
 * Address translation handling.
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

#include <lib/print.h>
#include <lib/debug.h>
#include <mm/vmm.h>
#include <proc/thread.h>

#include <mm/tlb.h>


/** Select the even/odd virtual page */
#define TLB_ENTRY_PAIR(vpn)  ((vpn) & 1)


/** Issue TLBR
 *
 * Read the TLB entry indexed by the Index register
 * into EntryLo0 and EntryLo1.
 *
 */
static inline void tlb_read (void)
{
	asm volatile (
		".set push\n"
		".set reorder\n"
		
		"tlbr\n"
		
		".set pop\n"
	);
}


/** Issue TLBP
 *
 * Probe the TLB and find the entry that matches
 * the current value in EntryHi. The matching entry
 * index is stored into the Index register. If no
 * TLB entry matches, it is indicated by the P bit
 * in the Index register.
 *
 */
static inline void tlb_probe (void)
{
	/*
	 * Note that there is a NOP after TLBP,
	 * since MIPS R4000 MMU state is undefined
	 * for the instruction just after TLBP.
	 */
	asm volatile (
		".set push\n"
		".set reorder\n"
		
		"tlbp\n"
		"nop\n"
		
		".set pop\n"
	);
}


/** Access to TLB entries using TLBWI
 *
 * The TLBWI instruction writes the entry from the EntryHi,
 * EntryLo0, EntryLo1 and PageMask registers into TLB. The
 * TLB position is specified by the value of the Index register.
 *
 */
static inline void tlb_write_indexed (void)
{
	asm volatile (
		".set push\n"
		".set reorder\n"
		
		"tlbwi\n"
		
		".set pop\n"
	);
}


/** Access to TLB entries using TLBWR
 *
 * The TLBWR instruction writes the entry from the EntryHi,
 * EntryLo0, EntryLo1 and PageMask registers into TLB. The
 * TLB position is specified by the value of the Random register.
 *
 */
static inline void tlb_write_random (void)
{
	asm volatile (
		".set push\n"
		".set reorder\n"
		
		"tlbwr\n"
		
		".set pop\n"
	);
}


/** Initialize address translation
 *
 * All TLB entries are cleared and invalidated, new entries
 * will be created on demand by the TLB Refill Exception
 * handler.
 *
 */
void tlb_init (void)
{
	/*
	 * The Wired register contains the number of entries that
	 * are never selected by the random TLB replacement
	 * algorithm. We need none of those now.
	 */
	write_cp0_wired (0);
	
	/*
	 * Each TLB entry is set by storing its fields
	 * in special registers and then copying the
	 * registers to the entry by the TLBWI
	 * instruction.
	 */
	
	/* The size of the page to map. */
	write_cp0_pagemask (CP0_PAGEMASK_4K);
	
	/* Physical address of the first of the pair of mapped pages. */
	write_cp0_entrylo0 (0);
	
	/* Physical address of the second of the pair of mapped pages. */
	write_cp0_entrylo1 (0);
	
	/* Unused address space identifier. */
	write_cp0_entryhi (CP0_ENTRYHI_ASID_MASK);
	
	/* Copy this to all TLB entries. */
	for (unsigned int i = 0; i < CP0_INDEX_INDEX_COUNT; i++) {
		write_cp0_index (i);
		tlb_write_indexed ();
	}
}


/** Write an indexed TLB entry
 *
 * Update the TLB entry indexed by the Index register.
 *
 * @param vpn Virtual page number to update in TLB.
 * @param pfn Physical frame number to update in TLB.
 *
 */
static void tlb_store_indexed (size_t vpn, size_t pfn)
{
	/* Read the TLB entry. */
	tlb_read ();
	
	/* The size of the page to map. */
	write_cp0_pagemask (CP0_PAGEMASK_4K);
	
	/*
	 * Check which pair of the TLB entries needs to be
	 * modified.
	 */
	if (TLB_ENTRY_PAIR (vpn) == 0)
		write_cp0_entrylo0 ((pfn << CP0_ENTRYLO_PFN_SHIFT) |
		    CP0_ENTRYLO_D_MASK | CP0_ENTRYLO_V_MASK);
	else
		write_cp0_entrylo1 ((pfn << CP0_ENTRYLO_PFN_SHIFT) |
		    CP0_ENTRYLO_D_MASK | CP0_ENTRYLO_V_MASK);
	
	/* Update the TLB entry. */
	tlb_write_indexed ();
}


/** Write a random TLB entry
 *
 * Update a random TLB entry.
 *
 * @param vpn Virtual page number to update in TLB.
 * @param pfn Physical frame number to update in TLB.
 *
 */
static void tlb_store_random (size_t vpn, size_t pfn)
{
	/* The size of the page to map. */
	write_cp0_pagemask (CP0_PAGEMASK_4K);
	
	/*
	 * Check which pair of the TLB entries needs to be
	 * modified.
	 *
	 * Since we are updating a random TLB entry, the other
	 * entry in the pair needs to invalidated for consistency.
	 */
	if (TLB_ENTRY_PAIR (vpn) == 0) {
		write_cp0_entrylo0 ((pfn << CP0_ENTRYLO_PFN_SHIFT) |
		    CP0_ENTRYLO_D_MASK | CP0_ENTRYLO_V_MASK);
		write_cp0_entrylo1 (0);
	} else {
		write_cp0_entrylo0 (0);
		write_cp0_entrylo1 ((pfn << CP0_ENTRYLO_PFN_SHIFT) |
		    CP0_ENTRYLO_D_MASK | CP0_ENTRYLO_V_MASK);
	}
	
	/* Fill a random TLB entry. */
	tlb_write_random ();
}


/** Flush a page from TLB.
 *
 * Remove any mapping of the given virtual page from TLB.
 * This operation is needed when a virtual memory mapping
 * for the given page is canceled in order to keep the TLB
 * consistent with the kernel view.
 *
 * @param addr Virtual address be flushed from TLB.
 *
 */
void tlb_flush (uintptr_t addr)
{
	/* Disable interrupts while manipulating the TLB. */
	ipl_t state = query_and_disable_interrupts ();
	
	/* Save the original EntryHi */
	unative_t entryhi = read_cp0_entryhi ();
	unative_t probe = entryhi;
	
	/*
	 * Probe for the TLB entry matching the virtual
	 * page number.
	 */
	probe &= ~CP0_ENTRYHI_VPN2_MASK;
	probe |= (addr >> PAGE_WIDTH >> 1) << CP0_ENTRYHI_VPN2_SHIFT;
	
	write_cp0_entryhi (probe);
	tlb_probe ();
	unative_t index = read_cp0_index ();
	
	if (!CP0_INDEX_P (index)) {
		/*
		 * The probe was successful, replace the TLB
		 * entry with an invalid entry.
		 */
		write_cp0_pagemask (CP0_PAGEMASK_4K);
		write_cp0_entrylo0 (0);
		write_cp0_entrylo1 (0);
		tlb_write_indexed ();
	}
	
	/* Restore the original EntryHi */
	write_cp0_entryhi (entryhi);
	
	conditionally_enable_interrupts (state);
}


/** TLB Invalid Exception handler
 *
 * Handle the TLB Invalid Exception.
 *
 * @param regisisters Interrupted context.
 *
 */
void tlb_invalid (context_t *registers)
{
	uintptr_t virt = registers->badva;
	uintptr_t phys;
	
	/*
	 * Find the virtual memory mapping.
	 */
	int rc = vmm_mapping_find(virt, &phys);
	if (rc != EOK) {
		printk ("Thread %x (pc=%x) caused invalid memory access at address %x\n",
		    thread_get_current (), registers->epc, virt);
		thread_finish (NULL);
	}
	
	uintptr_t vpn = virt >> PAGE_WIDTH;
	uintptr_t pfn = phys >> FRAME_WIDTH;
	
	/*
	 * Find the corresponding faulting TLB index.
	 */
	write_cp0_entryhi (registers->entryhi);
	tlb_probe ();
	unative_t index = read_cp0_index ();
	
	/* Put the mapping into TLB */
	if (CP0_INDEX_P (index))
		tlb_store_random (vpn, pfn);
	else
		tlb_store_indexed (vpn, pfn);
}


/** TLB Refill Exception handler
 *
 * Handle the TLB Refill Exception.
 *
 * @param regisisters Interrupted context.
 *
 */
void wrapped_tlb_refill (context_t *registers)
{
	uintptr_t virt = registers->badva;
	uintptr_t phys;
	
	/*
	 * Find the virtual memory mapping.
	 */
	int rc = vmm_mapping_find(virt, &phys);
	if (rc != EOK) {
		printk ("Thread %x (pc=%x) caused invalid memory access at address %x\n",
		    thread_get_current (), registers->epc, virt);
		thread_finish (NULL);
	}
	
	uintptr_t vpn = virt >> PAGE_WIDTH;
	uintptr_t pfn = phys >> FRAME_WIDTH;
	
	/* Put the mapping into TLB */
	tlb_store_random (vpn, pfn);
}
