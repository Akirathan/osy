/**
 * @file disk.c
 *
 * Disk.
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

#include <adt/list.h>
#include <proc/thread.h>

#include <drivers/disk.h>


int disk_init (void)
{
	// TODO
	return EOK;
}


int disk_get_nblocks (size_t *nblocks)
{
	// TODO
	return EOK;
}


int disk_read (size_t block, void *data)
{
	// TODO
	return EOK;
}


int disk_write (size_t block, void *data)
{
	// TODO
	return EOK;
}
