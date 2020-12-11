/*
**Name: Travis Chamness
**Project: 2 Task 1
**Date: 12/4/2020
*/

//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2020 Prof. AJ Bieszczad. All rights reserved.
///
//////////////////////////////////////////////////////////////////////////

#ifndef __DISK_H
#define __DISK_H

#ifdef __DEBUG_DISK

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#if defined( __DEBUG_TIMER ) && defined ( __linux__ )

#include <signal.h>
#include <time.h>
#include <sys/time.h>

#endif

#else
// adapt to kernel

// see https://www.kernel.org/doc/htmldocs/kernel-api/

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#endif

#define NUM_OF_CYLS 8
#define NUM_OF_HEADS 10
#define NUM_OF_SECTS 16
#define SECT_SIZE 16

#define MAX_LOGICAL_BLOCK (NUM_OF_CYLS * NUM_OF_HEADS * NUM_OF_SECTS)

typedef char sect_t[SECT_SIZE];
typedef sect_t head_t[NUM_OF_SECTS];
typedef head_t cyl_t[NUM_OF_HEADS];
typedef cyl_t disk_t[NUM_OF_CYLS];

typedef unsigned long lba_t;

typedef struct
{
  unsigned int cyl;
  unsigned int head;
  unsigned int sect;
} chs_t;

typedef enum {
    CIDEV_SUCCESS = 0,
    CIDEV_ADDRESS_ERROR,
    CIDEV_SPACE_ERROR,
//    CIDEV_ANOTHER_ERROR // as needed
} CIDEV_RET_CODE;

/**
 * This function translates a logical to a physical address.
 */
CIDEV_RET_CODE lba2chs(lba_t lba, chs_t *chs);

/**
 * This function translates a physical to a logical address.
 */
CIDEV_RET_CODE chs2lba(chs_t *chs, lba_t *lba);

/***
 * This function copies num_of_sectors sectors from the disk to the buffer starting with sector pointed
 * to by logical_block_num. You should validate the parameters before serving the request.
 */
CIDEV_RET_CODE readDisk(lba_t lba, unsigned int size, char **buffer);

/**
This functions "clears" the content of a sector by overwriting it with "."
 */
CIDEV_RET_CODE clearBlock(lba_t lba);

/**
 * This function copies the content of the buffer to the disk starting at sector pointed
 * to by logical block address. Here, you also should validate the parameters.
 */
CIDEV_RET_CODE writeDisk(lba_t lba, char *buffer);

#endif
