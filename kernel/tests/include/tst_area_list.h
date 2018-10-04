/***
 * Area list.
 *
 * Data structure for managing a sorted list of virtual memory areas.
 *
 * Change Log:
 * 2004/12/02 ghort   created
 * 2004/12/09 pallas  polished and debugged
 */

#ifndef _TST_AREA_LIST_H_
#define _TST_AREA_LIST_H_

#include <api.h>
#include "defs.h"

#include "tst_list.h"


/****************************************************************************\
| PUBLIC DEFS                                                                |
\****************************************************************************/

/*
 * A structure describing a virtual memory area.
 */
typedef struct tst_area {
	/*
	 * Starting page of the virtual memory area.
	 */
	int start;
	
	/*
	 * Size of the virtual memory area in pages.
	 */
	int size;
	
	/*
	 * Link in the list of virtual memory areas.
	 */
	tst_list_head_s lst_chain;
} tst_area_s;


/*
 * A structure describing the memory map. The individual areas of the
 * virtual memory are kept in an ordered list.
 */
typedef struct tst_area_list {
	/*
	 * Number of contiguous free areas.
	 */
	int hole_count;
	
	/*
	 * Number of contiguous allocated areas.
	 */
	int area_count;
	
	/*
	 * Number of neighboring allocated areas (each sequence of length 2),
	 * i.e. AFAAFAAAFA has 3 neighboring areas.
	 */
	int neigh_count;
	
	/*
	 * Start page of the tested segment of virtual memory.
	 */
	int page_start;
	
	/*
	 * Size of the segment of virtual memory, in pages.
	 */
	int page_count;
	
	/*
	 * Front and rear bumpers (sentinels) of the the virtual
	 * memory area list.
	 */
	tst_area_s front;
	tst_area_s rear;
} tst_area_list_s;


/****************************************************************************\
| PUBLIC FUNCTIONS                                                           |
\****************************************************************************/

/*
 * Initialization and locking.
 */
extern void tst_area_list_init (tst_area_list_s * mem, int page_start, int page_count);

/*
 * Querying the list of areas.
 */
extern int tst_area_list_area_count (tst_area_list_s *mem);
extern int tst_area_list_hole_count (tst_area_list_s *mem);
extern int tst_area_list_neigh_count (tst_area_list_s *mem);

extern void tst_area_list_get_area (tst_area_list_s *mem, int nth, int *start_page,
    int *page_count);
extern void tst_area_list_get_hole (tst_area_list_s *mem, int nth, int *start_page,
    int *page_count);
extern void tst_area_list_get_neigh (tst_area_list_s *mem, int nth_neigh, int *nth,
    int *start_page, int *page_count);

/*
 * Modifying the areas in the list.
 */
extern int tst_area_list_add (tst_area_list_s *mem, int start_page, int page_count);
extern void tst_area_list_del (tst_area_list_s *mem, int nth);
extern int tst_area_list_resize (tst_area_list_s *mem, int nth, int page_count);
extern int tst_area_list_split (tst_area_list_s *mem, int split_page);
extern int tst_area_list_remap (tst_area_list_s *mem, int nth, int to_page);
extern void tst_area_list_merge (tst_area_list_s *mem, int nth);

#endif /* _TST_AREA_LIST_H_ */
