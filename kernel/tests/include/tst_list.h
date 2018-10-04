/**
 * Doubly-linked list.
 *
 * This is a simplified version of the Linux kernel
 * list implementation with minor tweaks.
 *
 * Change Log:
 * 03/10/23 pallas  created
 * 03/11/18 ghort   added function tst_list_get_nth, added some macros from
 *                  klib.h to satisfy dependencies on klib.h
 */

#ifndef TST_LIST_H_
#define TST_LIST_H_


/****************************************************************************\
| PUBLIC DEFS                                                                |
\****************************************************************************/

/*
 * List head/entry structure.
 */
typedef struct tst_list_head {
	struct tst_list_head *prev;
	struct tst_list_head *next;
} tst_list_head_s;

typedef tst_list_head_s *tst_list_head_t;


/*
 * Static list head/entry initializer.
 */
#define TST_LIST_HEAD_INIT(name) \
	{ \
		.prev = &(name), \
		.next = &(name) \
	}

/*
 * Various types of visitor/observer functions.
 */
typedef void (* tst_list_destroy_fn) (tst_list_head_t entry);
typedef void (* tst_list_visit_fn) (tst_list_head_t entry, void *data);
typedef int (* tst_list_match_fn) (tst_list_head_t entry, void *data);


/****************************************************************************\
| PRIVATE CODE                                                               |
\****************************************************************************/

/** tst_offset_of
 *
 * Compute an offset of a member in a struct.
 *
 * @param type   Type of the container struct a member is embedded in.
 * @param member Name of the member within the struct.
 *
 */
#define tst_offset_of(type, member) \
	((size_t) &((type *) 0)->member)

/** tst_container_of
 *
 * Cast a member of a structure out to the containing structure.
 *
 * @param ptr    Pointer to the member.
 * @param type   Type of the container struct a member is embedded in.
 * @param member Name of the member within the struct.
 *
 */
#define tst_container_of(ptr, type, member) \
	({ \
		const typeof (((type *) 0)->member) * __mptr = (ptr); \
		(type *) ((char *) __mptr - tst_offset_of (type, member)); \
	})


/** __tst_list_add
 *
 * Internal function which inserts a new entry between two consecutive
 * entries. There can be a moment when the new item is visible during
 * backward but not during forward traversal, but this should be
 * handled by the caller.
 *
 */
static inline void __tst_list_add (tst_list_head_t new, tst_list_head_t prev,
    tst_list_head_t next)
{
	new->next = next;
	new->prev = prev;
	
	next->prev = new;
	prev->next = new;
}


/** __tst_list_del
 *
 * Internal function which links together two entries, removing any
 * entries existing between the two from the list.
 *
 * Again, we link the backward pointer fist because lists are less
 * often traversed backwards. When we link the forward entry, the
 * list is completely linked. The caller should make sure this
 * is not a problem.
 *
 */
static inline void __tst_list_del (tst_list_head_t prev, tst_list_head_t next)
{
	next->prev = prev;
	prev->next = next;
}


/****************************************************************************\
| PUBLIC CODE                                                                |
\****************************************************************************/

/** tst_list_init
 *
 * Initialize the list. An empty list head
 * points back to itself for easy runtime checks.
 *
 * @param head List head to initialize.
 *
 */
static inline void tst_list_init (tst_list_head_t head)
{
	head->prev = (tst_list_head_t) head;
	head->next = (tst_list_head_t) head;
}


/** tst_list_empty
 *
 * Test whether a list is empty.
 *
 */
static inline int tst_list_empty (tst_list_head_t head)
{
	return (head->next == head);
}


/** tst_list_entry
 *
 * Get a typed structure from the given list entry.
 *
 * @param ptr    The &tst_list_head_s pointer.
 * @param type   Type of the struct tst_list_head_s is embedded in.
 * @param member Name of the tst_list_head_s field within the struct.
 *
 */
#define tst_list_entry(ptr, type, member) \
	tst_container_of (ptr, type, member)


/** tst_list_add_succ
 *
 * Adds a successor entry to the list after the specified head.
 *
 * @param head List head to add the entry after.
 * @param tail New list entry.
 *
 */
static inline tst_list_head_t tst_list_add_succ (tst_list_head_t head,
    tst_list_head_t tail)
{
	__tst_list_add (tail, head, head->next);
	return tail;
}


/** tst_list_add_pred
 *
 * Add a predecessor entry to the list before the specified head.
 *
 * @param head List head to add the entry before.
 * @param tail New list entry.
 *
 */
static inline tst_list_head_t tst_list_add_pred (tst_list_head_t head,
    tst_list_head_t tail)
{
	__tst_list_add (tail, head->prev, head);
	return tail;
}


/** tst_list_del
 *
 * Delete a specific entry from the list and reinitialize
 * the entry so that tst_list_empty () returns true on it.
 *
 */
static inline tst_list_head_t tst_list_del (tst_list_head_t entry)
{
	__tst_list_del (entry->prev, entry->next);
	
	entry->next = entry;
	entry->prev = entry;
	return entry;
}


/** tst_list_del_succ
 *
 * Remove the successor of the specified list head from the list
 * and return it to the caller.
 *
 * @param head List head to delete from.
 *
 */
static inline tst_list_head_t tst_list_del_succ (tst_list_head_t head)
{
	tst_list_head_t entry = head->next;
	tst_list_del (entry);
	return entry;
}


/** tst_list_del_pred
 *
 * Remove the predecessor of the specified list head from the
 * list and return it to the caller.
 *
 * @param head List head to delete from.
 *
 */
static inline tst_list_head_t tst_list_del_pred (tst_list_head_t head)
{
	tst_list_head_t entry = head->prev;
	tst_list_del (entry);
	return entry;
}


/** tst_list_for_each
 *
 * Iterate over a list.
 *
 * @param entry: The &tst_list_head_s to use as a loop counter.
 * @param head:  Head of the list.
 *
 */
#define tst_list_for_each(entry, head) \
	for (entry = (head)->next; entry != (head); entry = entry->next)


/** tst_list_for_each_reverse
 *
 * Iterate backwards over a list.
 *
 * @param entry The &tst_list_head_s to use as a loop counter.
 * @param head  Head of the list.
 *
 */
#define tst_list_for_each_reverse(entry, head) \
	for (entry = (head)->prev; entry != (head); entry = entry->prev)


/** tst_list_for_each_entry
 *
 * Iterate over a list of typed entries.
 *
 * @param entry: The type * to use as a loop counter.
 * @param head   Head of the list.
 * @param member Name of the tst_list_head_s within the struct.
 *
 */
#define tst_list_for_each_entry(entry, head, member) \
	for (entry = tst_list_entry ((head)->next, \
			typeof (* entry), member); \
		&entry->member != (head); \
		entry = tst_list_entry (entry->member.next, \
			typeof (* entry), member))


/** tst_list_for_each_entry_reverse
 *
 * Iterates backwards over a list of typed entries.
 *
 * @param entry: The type * to use as a loop counter.
 * @param head:  Head of the list.
 * @param member Name of the tst_list_head_s within the struct.
 *
 */
#define tst_list_for_each_entry_reverse(entry, head, member) \
	for (entry = tst_list_entry ((head)->prev, \
			typeof (* entry), member); \
		&entry->member != (head); \
		entry = tst_list_entry (entry->member.prev, \
			typeof (* entry), member))


/** tst_list_destroy
 *
 * Walk the list, remove all the entries from
 * the list and call a destructor on them.
 *
 * @param head    List to destroy.
 * @param destroy Destructor function to call on every entry.
 *
 */
static inline void tst_list_destroy (tst_list_head_t head,
    tst_list_destroy_fn destroy)
{
	while (!tst_list_empty (head)) {
		tst_list_head_t entry = tst_list_del_succ (head);
		destroy (entry);
	}
}


/** tst_list_walk
 *
 * Walk the list and call an visitor function on every entry.
 *
 * @param head  List to walk.
 * @param visit Function to call on every entry.
 * @param data  Data for the visit function.
 *
 */
static inline void tst_list_walk (tst_list_head_t head,
    tst_list_visit_fn visit, void *data)
{
	if (!tst_list_empty (head)) {
		tst_list_head_t entry;
		
		tst_list_for_each (entry, head) {
			visit (entry, data);
		}
	}
}


/** tst_list_find
 *
 * Walk the list and call a match function on every entry.
 * If a match is found, the matching entry is returned to the
 * caller, otherwise NULL is returned.
 *
 * @param head  List to walk.
 * @param match Function to call on every entry.
 * @param data  Data for the match function.
 *
 */
static inline tst_list_head_t tst_list_find (tst_list_head_t head,
    tst_list_match_fn match, void *data)
{
	if (!tst_list_empty (head)) {
		tst_list_head_t entry;
		
		tst_list_for_each (entry, head) {
			if (match (entry, data))
				return entry;
		}
	}
	
	return NULL;
}


/** __tst_list_get_nth_match
 *
 * Auxiliary function for tst_list_get_nth.
 *
 */
static inline int __tst_list_get_nth_match (tst_list_head_t item, int *cnt)
{
	if ((*cnt)-- == 0)
		return 1;
	
	return 0;
}

/*** tst_list_get_nth
 *
 * Return nth entry in the list. The entries are indexed from 0.
 * If the list is not so long NULL is returned.
 *
 * @param head: List.
 * @param idx   Index of the entry.
 *
 */
static inline tst_list_head_t tst_list_get_nth (tst_list_head_t head, int idx)
{
	return tst_list_find (head,
	    (tst_list_match_fn) __tst_list_get_nth_match, &idx);
}

#endif /* TST_LIST_H_ */
