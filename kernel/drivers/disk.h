/**
 * @file disk.h
 *
 * Disk block device support.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef DISK_H_
#define DISK_H_

#include <include/shared.h>
#include <include/c.h>


#define DDISK_ADDRESS  (ADDR_IN_KSEG1 (DEVICE_DDISK_ADDR))

#define DISK_BLOCK_SIZE  512


/* Externals are commented with implementation */
extern int disk_init (void);
extern void disk_handle (void);
extern int disk_get_nblocks (size_t *nblocks);
extern int disk_read (size_t block, void *data);
extern int disk_write (size_t block, void *data);


#endif
