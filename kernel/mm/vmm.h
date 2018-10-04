/**
 * @file vmm.h
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

#ifndef VMM_H_
#define VMM_H_


#include <include/c.h>
#include <mm/falloc.h>


/** The size of a page.
 *
 * The page size is derived from the frame size.
 *
 */
#define PAGE_WIDTH  FRAME_WIDTH
#define PAGE_SIZE   FRAME_SIZE


/** Number of supported VMAs per VMM.
 *
 */
#define VMAS  100


/** Virtual memory area slot
 *
 * This is a single entry of a virtual
 * memory area.
 *
 */
struct vma {
	uintptr_t vpn_base;
	uintptr_t pfn_base;
	size_t count;
	bool valid;
};


/** Virtual memory map control structure
 *
 */
typedef struct vmm {
	asid_t asid;
	struct vma vma[VMAS];
} *vmm_t;


/* Externals are commented with implementation */
extern int vma_map (void **from, const size_t size, const vm_flags_t flags);
extern int vma_unmap (const void *from);
extern int vma_check_user (const void *addr, const size_t size);

extern int vmm_create (vmm_t *vmmp);
extern int vmm_mapping_find (uintptr_t virt, uintptr_t *phys);


#endif
