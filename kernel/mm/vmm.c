/**
 * @file vmm.c
 *
 * Virtual memory map.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <include/shared.h>
#include <include/c.h>

#include <mm/malloc.h>
#include <lib/debug.h>
#include <proc/thread.h>
#include <lib/string.h>
#include <mm/tlb.h>

#include <mm/vmm.h>


/*
 * Warning: This is a very primitive implementation of virtual
 * to physical memory mapping that should be only considered
 * as a crude demonstration of the interface.
 *
 * The following code has many limitations and it operates with
 * many (silent) assumptions. A proper implementation should not
 * be based on the following code, but should start from scratch.
 *
 * A short list of some of the limitations:
 *
 *  * The number of address space identifiers (ASIDs) that can
 *    ever operate in the system is limited to 254. There is no
 *    LRU mechanism for reclaiming ASIDs.
 *
 *  * The virtual memory areas map only to continuous physical
 *    memory areas. This is rather inefficient due to the potential
 *    fragmentation of the physical address space.
 *
 *  * The number of virtual memory areas in any virtual memory
 *    map is limited by a constant. The data structure for the VMA
 *    bookkeeping is a primitive static array which only provides
 *    for a linear lookup times (determined by the number of VMAs).
 *
 *  * The mechanism for searching for unused virtual memory areas
 *    is broken and can only work reliably for small address spaces.
 *
 *  * Most corner cases (e.g. creating a new virtual memory area that
 *    overlaps with an existing area) are not handled properly.
 *
 */


/** Primitive global counter for assinging new ASIDs */
static asid_t last_asid = 0;


/** Create a virtual memory area
 *
 * Create a virtual memory area in the current virtual memory
 * map. Only a small subset of the flags and functionality
 * is actually implemented and the implementation is broken.
 *
 * @param from  Place to store the starting virtual memory address
 *              of the new virtual memory area.
 * @param size  Size of the virtual memory area in bytes.
 * @param flags Flags of the virtual memory area.
 *
 * @return EOK if the virtual memory area was created.
 * @return Error code otherwise.
 *
 */
int vma_map (void **from, const size_t size, const vm_flags_t flags)
{
	bool flag_auto = ((flags & VF_VA_AUTO) == VF_VA_AUTO);
	bool flag_user = ((flags & VF_VA_USER) == VF_VA_USER);
	bool flag_kseg0 = ((flags & VF_AT_KSEG0) == VF_AT_KSEG0) ||
	    ((flags & VF_AT_KSEG1) == VF_AT_KSEG1);
	
	assert (flag_auto ^ flag_user);
	
	if (ALIGN_DOWN (size, PAGE_SIZE) != size)
		return EINVAL;
	
	// TODO:
	// Only user segment addresses are currently supported.
	// The segment magic is currently completely hardwired.
	
	size_t count = size >> PAGE_WIDTH;
	int rc = ENOMEM;
	
	if (flag_kseg0) {
		panic ("Not implemented");
	} else {
		/* Disable interrupts while accessing shared structures. */
		ipl_t state = query_and_disable_interrupts ();
		
		struct vmm *vmm = thread_get_current ()->vmm;
		
		if (flag_auto) {
			// TODO:
			// This search for an unmapped area in the address space
			// is also completely hardwired and broken. It somehow
			// works only thanks to the numerous silent assumptions about
			// the address space layout of the user process and should
			// be replaced by a proper implementation.
			
			size_t vpn = 0;
			
			for (unsigned int i = 0; i < VMAS; i++) {
				if ((vmm->vma[i].valid) && (vmm->vma[i].vpn_base < 0x40000) &&
				    (vmm->vma[i].vpn_base + vmm->vma[i].count > vpn))
					vpn = vmm->vma[i].vpn_base + vmm->vma[i].count;
			}
			
			if (vpn == 0) {
				conditionally_enable_interrupts (state);
				return ENOMEM;
			}
			
			*from = (void *) (vpn << PAGE_WIDTH);
		}
		
		if (ALIGN_DOWN ((uintptr_t) *from, PAGE_SIZE) != (uintptr_t) *from) {
			conditionally_enable_interrupts (state);
			return EINVAL;
		}
		
		/*
		 * Find and empty "slot" for the new virtual memory
		 * area.
		 */
		for (unsigned int i = 0; i < VMAS; i++) {
			if (!vmm->vma[i].valid) {
				
				// TODO:
				// We can only satisfy the request with a continuous
				// physical memory area. This is rather inefficient.
				
				uintptr_t phys;
				rc = frame_alloc (&phys, count, VF_VA_AUTO | VF_AT_KSEG0);
				if (rc != EOK)
					break;
				
				vmm->vma[i].vpn_base = ((uintptr_t) *from) >> PAGE_WIDTH;
				vmm->vma[i].pfn_base = phys >> FRAME_WIDTH;
				vmm->vma[i].count = count;
				vmm->vma[i].valid = true;
				break;
			}
		}
		
		conditionally_enable_interrupts (state);
	}
	
	return rc;
}


/** Remove a virtual memory area
 *
 * Remove a virtual memory area previously created by vma_map().
 *
 * @param from  Starting virtual memory address of the virtual
 *              memory area.
 *
 * @return EOK if the virtual memory area was removed.
 * @return Error code otherwise.
 *
 */
int vma_unmap (const void *from)
{
	if (ALIGN_DOWN ((uintptr_t) from, PAGE_SIZE) != (uintptr_t) from)
		return EINVAL;
	
	size_t vpn = ((uintptr_t) from) >> PAGE_WIDTH;
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	struct vmm *vmm = thread_get_current ()->vmm;
	int rc = EINVAL;
	
	/*
	 * Find the "slot" of the virtual memory area.
	 */
	for (unsigned int i = 0; i < VMAS; i++) {
		if ((vmm->vma[i].valid) && (vmm->vma[i].vpn_base == vpn)) {
			uintptr_t phys = vmm->vma[i].pfn_base << FRAME_WIDTH;
			
			rc = frame_free (phys, vmm->vma[i].count);
			if (rc != EOK)
				break;
			
			/*
			 * Flush the pages from TLB.
			 */
			for (size_t pos = 0; pos < vmm->vma[i].count; pos++)
				tlb_flush ((vpn + pos) << FRAME_WIDTH);
			
			vmm->vma[i].valid = false;
			break;
		}
	}
	
	conditionally_enable_interrupts (state);
	return rc;
}


/** Check user memory mapping
 *
 * Check whether the given memory area is correctly mapped
 * in the address space of the current user process.
 *
 * @param addr Starting virtual address of the memory area.
 * @param size Size of the memory area.
 *
 * @return True if the memory area is mapped.
 * @return False if the memory area is not mapped.
 *
 */
int vma_check_user (const void *addr, const size_t size)
{
	size_t vpn_start = ((uintptr_t) addr) >> PAGE_WIDTH;
	size_t vpn_end = (((uintptr_t) addr) + size) >> PAGE_WIDTH;
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	struct vmm *vmm = thread_get_current ()->vmm;
	int rc = false;
	
	for (unsigned int i = 0; i < VMAS; i++) {
		if ((vmm->vma[i].valid) &&
		    (vmm->vma[i].vpn_base <= vpn_start) &&
		    (vmm->vma[i].vpn_base + vmm->vma[i].count > vpn_start) &&
		    (vmm->vma[i].vpn_base <= vpn_end) &&
		    (vmm->vma[i].vpn_base + vmm->vma[i].count > vpn_end)) {
			rc = true;
			break;
		}
	}
	
	// TODO:
	// Check that the memory area is actually in KUSEG
	
	conditionally_enable_interrupts (state);
	
	return rc;
}


/** Create a new virtual memory map (address space)
 *
 * Create a new (empty) virtual memory map (address space).
 *
 * @param pvmm Place to store the virtual memory map structure.
 *
 * @return EOK if the virtual memory map was created.
 * @return ENOMEM if there is no memory for a new virtual memory map.
 *
 */
int vmm_create (vmm_t *pvmm)
{
	struct vmm *vmm = (struct vmm *) malloc (sizeof (struct vmm));
	if (!vmm)
		return ENOMEM;
	
	bzero (vmm, sizeof (struct vmm));
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	// TODO:
	// This is really broken. The address space identifiers
	// (ASIDs) should not be assigned to address spaces statically,
	// but rather dynamically on-the-fly using an LRU algorithm.
	
	vmm->asid = last_asid;
	last_asid++;
	
	conditionally_enable_interrupts (state);
	
	(* pvmm) = vmm;
	return EOK;
}


/** Translate virtual address to physical address
 *
 * Convert virtual address to physical address using the current virtual
 * memory map.
 *
 * @param virt Virtual address to convert.
 * @param phys Storage for the converted physical address. No value
 *             is stored if there is no translation found.
 *
 * @return EOK if the translation exists in the virtual memory map.
 * @return EINVAL otherwise.
 *
 */
int vmm_mapping_find (uintptr_t virt, uintptr_t *phys)
{
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	struct vmm *vmm = thread_get_current ()->vmm;
	int rc = EINVAL;
	uintptr_t vpn = virt >> PAGE_WIDTH;
	
	// TODO:
	// A more efficient data structure should be used (hash table,
	// hierarchical page tables) to provide a better time complexity
	// for the lookup.
	
	for (unsigned int i = 0; i < VMAS; i++) {
		if ((vmm->vma[i].valid) && (vmm->vma[i].vpn_base <= vpn) &&
		    (vmm->vma[i].vpn_base + vmm->vma[i].count > vpn)) {
			uintptr_t pfn = vmm->vma[i].pfn_base +
			    (vpn - vmm->vma[i].vpn_base);
			uintptr_t offset = virt & (PAGE_SIZE - 1);
			*phys = (pfn << FRAME_WIDTH) + offset;
			rc = EOK;
			break;
		}
	}
	
	conditionally_enable_interrupts (state);
	return rc;
}
