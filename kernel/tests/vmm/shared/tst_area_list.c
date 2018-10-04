/***
 * Area list.
 *
 * Data structure for managing a sorted list of virtual memory areas.
 *
 * Change Log:
 * 2004/12/02 ghort   created
 * 2004/12/09 pallas  polished and debugged
 */

#include <api.h>
#include "../../include/defs.h"

#include "../../include/tst_area_list.h"


/****************************************************************************\
| PRIVATE DEFS                                                               |
\****************************************************************************/

/*
 * A structure holding arguments for match function
 * which selects a given pair of neighboring areas.
 */
struct neigh_data {
	/*
	 * Index of the pair.
	 */
	int nth_neigh;
	
	/*
	 * Index of an area.
	 */
	int nth;
}; /* struct neigh_data */


/****************************************************************************\
| PRIVATE CODE                                                               |
\****************************************************************************/

/**
 * match_succ_area
 *
 * Match function for tst_list_find, which matches a virtual
 * memory area starting beyond the specified page.
 */
static inline int
match_succ_area (tst_list_head_t item, int *page)
{
	tst_area_s * area = tst_list_entry (item, tst_area_s, lst_chain);
	return (area->start > (* page));
} /* match_succ_area */


/**
 * match_nth_hole
 *
 * A match function for tst_list_find which matches nth hole in the
 * virtual memory map.
 */
static inline int
match_nth_hole (tst_list_head_t item, int * cnt)
{
	tst_area_s * area = tst_list_entry (item, tst_area_s, lst_chain);
	tst_area_s * pred_area = tst_list_entry(item->prev, tst_area_s, lst_chain);
	assert (pred_area);
	
	if (pred_area->start + pred_area->size < area->start) {
		if ((* cnt)-- == 0) {
			return 1;
		}
	}
	return 0;
} /* match_nth_hole */


/**
 * match_nth_neigh
 *
 * Match function for tst_list_find, which matches a given
 * pair of neighboring virtual memory areas.
 */
static inline int
match_nth_neigh (tst_list_head_t item, struct neigh_data *data)
{
	tst_area_s * area = tst_list_entry (item, tst_area_s, lst_chain);
	tst_area_s * succ_area = tst_list_entry (item->next, tst_area_s, lst_chain);
	assert (succ_area);
	
	data->nth++;
	if (area->start + area->size == succ_area->start) {
		if (data->nth_neigh-- == 0) {
			return 1;
		}
	}
	
	return 0;
} /* match_nth_neigh */


/****************************************************************************\
| PUBLIC CODE (INITIALIZATION, LOCKING)                                      |
\****************************************************************************/

/**
 * tst_area_list_init
 *
 * Initializes the list of virtual memory areas.
 */
void
tst_area_list_init (tst_area_list_s * mem, int page_start, int page_count)
{
	mem->hole_count = 1;
	mem->area_count = 0;
	mem->neigh_count = 0;
	mem->page_start = page_start;
	mem->page_count = page_count;
	
	mem->front.start = page_start;
	mem->front.size = 0;
	
	mem->rear.start = page_start + page_count;
	mem->rear.size = INT_MAX;
	
	tst_list_init (&mem->front.lst_chain);
	tst_list_init (&mem->rear.lst_chain);
	
	tst_list_add_succ (&mem->front.lst_chain, &mem->rear.lst_chain);
} /* tst_area_list_init */


/****************************************************************************\
| PUBLIC CODE (QUERYING)                                                     |
\****************************************************************************/

/**
 * tst_area_list_area_count
 *
 * Returns the number of areas in the virtual memory map.
 */
inline int
tst_area_list_area_count (tst_area_list_s *mem)
{
	return mem->area_count;
} /* tst_area_list_area_count */


/**
 * tst_area_list_hole_count
 *
 * Returns the number of holes in the virtual memory map.
 */
inline int
tst_area_list_hole_count(tst_area_list_s *mem)
{
	return mem->hole_count;
} /* tst_area_list_hole_count */


/**
 * tst_area_list_neigh_count
 *
 * Returns the number of neigboring pairs of allocated areas.
 */
inline int
tst_area_list_neigh_count (tst_area_list_s *mem)
{
	return mem->neigh_count;
} /* tst_area_list_neigh_count */


/**
 * tst_area_list_get_area
 *
 * Returns the start page and the size of nth virtual memory area.
 */
void tst_area_list_get_area (tst_area_list_s * mem, int nth, int * start_page,
    int * page_count)
{
	tst_list_head_t nth_item = tst_list_get_nth (&mem->front.lst_chain, nth);
	tst_area_s * nth_area = tst_list_entry (nth_item, tst_area_s, lst_chain);
	
	assert (nth_area);
	assert (nth_area != &mem->front);
	assert (nth_area != &mem->rear);
	
	(* start_page) = nth_area->start;
	(* page_count) = nth_area->size;
} /* tst_area_list_get_area */


/**
 * tst_area_list_get_hole
 *
 * Returns the start and size of nth hole in the virtual memory map.
 */
void tst_area_list_get_hole (tst_area_list_s * mem, int nth, int * start_page,
    int * page_count)
{
	tst_list_head_t succ_item = tst_list_find (&mem->front.lst_chain,
	    (tst_list_match_fn) match_nth_hole, &nth);
	assert (succ_item);
	
	tst_area_s * succ_area = tst_list_entry (succ_item, tst_area_s, lst_chain);
	assert (succ_area);
	
	tst_area_s * pred_area = tst_list_entry (succ_item->prev, tst_area_s, lst_chain);
	assert (pred_area);
	
	(* start_page) = pred_area->start + pred_area->size;
	(* page_count) = succ_area->start - (* start_page);
} /* tst_area_list_get_hole */


/**
 * tst_area_list_get_neigh
 *
 * Returns the nth pair of neighboring areas.
 */
void tst_area_list_get_neigh (tst_area_list_s * mem, int nth_neigh, int * nth,
    int * start_page, int * page_count)
{
	struct neigh_data data = { .nth_neigh = nth_neigh, .nth = -1 };
	assert (nth_neigh < mem->neigh_count);
	
	tst_list_head_t neigh_item = tst_list_find (&mem->front.lst_chain,
	    (tst_list_match_fn) match_nth_neigh, &data);
	assert (neigh_item);
	
	tst_area_s * neigh_area = tst_list_entry (neigh_item, tst_area_s, lst_chain);
	assert (neigh_area);
	
	(* start_page) = neigh_area->start;
	(* page_count) = neigh_area->size;
	(* nth) = data.nth;
} /* tst_area_list_get_neigh */


/****************************************************************************\
| PUBLIC CODE (MODIFICATION)                                                 |
\****************************************************************************/

/**
 * tst_area_list_add
 *
 * Add the area to the tst_area_list and check if the area does not overlap
 * with another already allocated.
 *
 * Returns ENOMEM if there is not enough memory to allocate a new list item,
 * EINVAL if there is an overlap.
 */
int
tst_area_list_add (tst_area_list_s * mem, int start_page, int page_count)
{
	tst_list_head_t succ_item = tst_list_find (&mem->front.lst_chain,
	    (tst_list_match_fn) match_succ_area, &start_page);
	assert (succ_item);
	
	tst_area_s * succ_area = tst_list_entry (succ_item, tst_area_s, lst_chain);
	assert (succ_area);
	
	tst_area_s * pred_area = tst_list_entry (succ_item->prev, tst_area_s, lst_chain);
	assert (pred_area);
	
	if ((pred_area->start + pred_area->size > start_page) ||
	    (start_page + page_count > succ_area->start)) {
		/* The inserted area would overlap with others */
		return EINVAL;
	}
	
	tst_area_s * new_area = (tst_area_s *) malloc (sizeof (tst_area_s));
	if (new_area == NULL) {
		return ENOMEM;
	}
	
	new_area->start = start_page;
	new_area->size = page_count;
	tst_list_add_pred (succ_item, &new_area->lst_chain);
	
	/*
	 * If the area fits right between two other areas, this will add
	 * two neighbor boundaries and remove a single hole. If the area
	 * neigbors only with one other area, this will add a single neighbor
	 * boundary and leave the number of holes intact. If the area is put
	 * in a middle of a hole, this will leave the number of neighbors
	 * unchanged, but will split the hole in two.
	 */
	mem->area_count++;
	mem->hole_count--;
	
	if (pred_area->start + pred_area->size == start_page) {
		if (pred_area != &mem->front) {
			mem->neigh_count++;
		}
	} else {
		mem->hole_count++;
	}
	
	if (start_page + page_count == succ_area->start) {
		if (succ_area != &mem->rear) {
			mem->neigh_count++;
		}
	} else {
		mem->hole_count++;
	}
	
	return EOK;
} /* tst_area_list_add */


/**
 * tst_area_list_del
 *
 * Removes the nth virtual memory area from the area list.
 */
void
tst_area_list_del (tst_area_list_s * mem, int nth)
{
	tst_list_head_t nth_item = tst_list_get_nth (&mem->front.lst_chain, nth);
	tst_area_s * nth_area = tst_list_entry (nth_item, tst_area_s, lst_chain);
	
	assert (nth_area);
	assert (nth_area != &mem->front);
	assert (nth_area != &mem->rear);
	
	tst_area_s * pred_area = tst_list_entry (nth_item->prev, tst_area_s, lst_chain);
	assert (pred_area);
	
	tst_area_s * succ_area = tst_list_entry (nth_item->next, tst_area_s, lst_chain);
	assert (succ_area);
	
	/*
	 * If the area was surrounded from both sides, this will remove
	 * two neighbor boundaries and create one hole. If the area was
	 * surrounded only from one side, this will remove one neighbor
	 * boundary and leave the number of holes unchanged. If the area
	 * was surrounded by holes, this will leave the neighbors unchanged,
	 * but will merge two holes into one.
	 */
	mem->area_count--;
	mem->hole_count++;
	
	if (pred_area->start + pred_area->size == nth_area->start) {
		if (pred_area != &mem->front) {
			mem->neigh_count--;
		}
	} else {
		mem->hole_count--;
	}
	
	if (nth_area->start + nth_area->size == succ_area->start) {
		if (succ_area != &mem->rear) {
			mem->neigh_count--;
		}
	} else {
		mem->hole_count--;
	}
	
	tst_list_del (nth_item);
	free (nth_area);
} /* tst_area_list_del */


/**
 * tst_area_list_resize
 *
 * Resizes nth area to a new size.
 *
 * Returns EINVAL should an overlap happen.
 */
int
tst_area_list_resize (tst_area_list_s *mem, int nth, int page_count)
{
	tst_list_head_t nth_item = tst_list_get_nth (&mem->front.lst_chain, nth);
	tst_area_s * nth_area = tst_list_entry (nth_item, tst_area_s, lst_chain);
	
	assert (nth_area);
	assert (nth_area != &mem->front);
	assert (nth_area != &mem->rear);
	
	tst_area_s * succ_area = tst_list_entry (nth_item->next, tst_area_s, lst_chain);
	assert (succ_area);
	
	if (nth_area->start + page_count > succ_area->start) {
		/* The resized area would overlap with others */
		return EINVAL;
	}
	
	/*
	 * Adjust the hole and neighbor counters. If the area neighbored with
	 * another area before resize and does not after, increase the number
	 * of holes and decrease the number of neighbors, except for the
	 * rear bumper. If it was the other way around, increase the number
	 * of neighbors and decrease the number of holes.
	 */
	if ((nth_area->start + nth_area->size == succ_area->start) &&
	    (nth_area->start + page_count < succ_area->start)) {
		if (succ_area != &mem->rear) {
			mem->neigh_count--;
		}
		
		mem->hole_count++;
	} else if ((nth_area->start + nth_area->size < succ_area->start) &&
	    (nth_area->start + page_count == succ_area->start)) {
		if (succ_area != &mem->rear) {
			mem->neigh_count++;
		}
		
		mem->hole_count--;
	}
	
	nth_area->size = page_count;
	return EOK;
} /* tst_area_list_resize */


/**
 * tst_area_list_split
 *
 * Splits the area at @split_page into two.
 *
 * Returns ENOMEM if there is not enough memory to allocate a new list item.
 */
int
tst_area_list_split (tst_area_list_s *mem, int split_page)
{
	tst_list_head_t succ_item = tst_list_find (&mem->front.lst_chain,
	    (tst_list_match_fn) match_succ_area, &split_page);
	assert (succ_item);
	
	tst_area_s * succ_area = tst_list_entry (succ_item, tst_area_s, lst_chain);
	assert (succ_area);
	
	tst_area_s * this_area = tst_list_entry (succ_item->prev, tst_area_s, lst_chain);
	
	assert (this_area);
	assert (this_area->start < split_page);
	assert (this_area->start + this_area->size > split_page);
	
	tst_area_s * new_area = (tst_area_s *) malloc (sizeof (tst_area_s));
	if (new_area == NULL) {
		return ENOMEM;
	}
	
	new_area->start = split_page;
	new_area->size = this_area->start + this_area->size - split_page;
	this_area->size -= new_area->size;
	
	/*
	 * Splitting an area increases the number of areas and of
	 * neighbor boundaries.
	 */
	mem->neigh_count++;
	mem->area_count++;
	
	tst_list_add_pred (succ_item, &new_area->lst_chain);
	return EOK;
} /* tst_area_list_split */


/**
 * tst_area_list_remap
 *
 * Remaps the nth area to to_page.
 *
 * Returns EINVAL if the remap would cause an overlap.
 */
int
tst_area_list_remap (tst_area_list_s *mem, int nth, int to_page)
{
	/*
	 * Find the nth virtual memory area. It should be neither of the
	 * front/rear bumpers.
	 */
	tst_list_head_t nth_item = tst_list_get_nth (&mem->front.lst_chain, nth);
	tst_area_s * nth_area = tst_list_entry (nth_item, tst_area_s, lst_chain);
	
	assert (nth_area);
	assert (nth_area != &mem->front);
	assert (nth_area != &mem->rear);
	
	/*
	 * Find the predecessor and the successor of the area. These can
	 * happen to be the front and rear bumpers.
	 */
	tst_area_s * from_succ_area = tst_list_entry (nth_item->next, tst_area_s, lst_chain);
	assert (from_succ_area);
	
	tst_area_s * from_pred_area = tst_list_entry (nth_item->prev, tst_area_s, lst_chain);
	assert (from_pred_area);
	
	/*
	 * Find between which areas the to_page points.
	 */
	tst_list_head_t to_succ_item = tst_list_find (&mem->front.lst_chain,
	    (tst_list_match_fn) match_succ_area, &to_page);
	assert (to_succ_item);
	
	tst_list_head_t to_pred_item = to_succ_item->prev;
	
	/*
	 * Update the "to" pointers so that they don't point at the
	 * nth_area to allow for overlaps with the area being moved.
	 */
	if (to_succ_item == nth_item) {
		to_succ_item = nth_item->next;
	} else if (to_pred_item == nth_item) {
		to_pred_item = nth_item->prev;
	}
	
	tst_area_s * to_succ_area = tst_list_entry (to_succ_item, tst_area_s, lst_chain);
	assert (to_succ_area);
	
	tst_area_s * to_pred_area = tst_list_entry (to_pred_item, tst_area_s, lst_chain);
	assert (to_pred_area);
	
	if ((to_page < to_pred_area->start + to_pred_area->size) ||
	    (to_page + nth_area->size > to_succ_area->start)) {
		// the remaped area would overlap with others
		return EINVAL;
	}
	
	/*
	 * Update the neighbor and hole counters with respect to the
	 * original location (corresponds to deletion of the area). Since
	 * we are remapping an area, the number of area remains unchanged.
	 */
	if (from_pred_area->start + from_pred_area->size == nth_area->start) {
		if (from_pred_area != &mem->front) {
			mem->neigh_count--;
		}
	} else {
		mem->hole_count--;
	}
	
	if (nth_area->start + nth_area->size == from_succ_area->start) {
		if (from_succ_area != &mem->rear) {
			mem->neigh_count--;
		}
	} else {
		mem->hole_count--;
	}
	
	/*
	 * Update the neighbor and hole counters with respect to the
	 * target location (corresponds to insertion of the area). Since
	 * we are remapping an area, the number of areas remains unchanged.
	 */
	if (to_pred_area->start + to_pred_area->size == to_page) {
		if (to_pred_area != &mem->front) {
			mem->neigh_count++;
		}
	} else {
		mem->hole_count++;
	}
	
	if (to_page + nth_area->size == to_succ_area->start) {
		if (to_succ_area != &mem->rear) {
			mem->neigh_count++;
		}
	} else {
		mem->hole_count++;
	}
	
	
	/*
	 * Update and move the area in the list.
	 */
	nth_area->start = to_page;
	tst_list_del (nth_item);
	tst_list_add_pred (to_succ_item, nth_item);
	return EOK;
} /* tst_area_list_remap */


/**
 * tst_area_list_merge
 *
 * Merges two neighboring areas starting from @nth.
 */
void tst_area_list_merge (tst_area_list_s *mem, int nth)
{
	tst_list_head_t nth_item = tst_list_get_nth (&mem->front.lst_chain, nth);
	tst_area_s * nth_area = tst_list_entry (nth_item, tst_area_s, lst_chain);
	
	assert(nth_area);
	assert(nth_area != &mem->front);
	assert(nth_area != &mem->rear);
	
	tst_list_head_t succ_item = nth_item->next;
	tst_area_s * succ_area = tst_list_entry (succ_item, tst_area_s, lst_chain);
	assert (succ_area);
	
	assert (nth_area->start + nth_area->size == succ_area->start);
	
	nth_area->size += succ_area->size;
	
	/*
	 * Merging two areas decreases the number of areas and the
	 * number of neighbor boundaries.
	 */
	mem->area_count--;
	mem->neigh_count--;
	
	tst_list_del (succ_item);
	free (succ_area);
} /* tst_area_list_merge */
