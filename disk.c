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

#include "disk.h"

disk_t disk;

/***
 *
 * Verifies correctness of the logical block address, and then translates it to a cylinder-head-sector equivalent.
 *
 */
CIDEV_RET_CODE lba2chs(lba_t lba, chs_t *chs)
{
    if (lba >= MAX_LOGICAL_BLOCK)
        return CIDEV_ADDRESS_ERROR;
	
    chs->cyl = lba /(NUM_OF_HEADS*NUM_OF_SECTS);
    chs->head = (lba / NUM_OF_SECTS) % NUM_OF_HEADS;
    chs->sect = (lba % NUM_OF_SECTS);
// todo: implement

    return CIDEV_SUCCESS;
}

/***
 *
 * Verifies correctness of the cylinder-head-sector address, and then translates it to a logical block address.
 *
 */
CIDEV_RET_CODE chs2lba(chs_t *chs, lba_t *lba)
{
// todo: implement
    if((chs->cyl * NUM_OF_HEADS * NUM_OF_SECTS) + (chs->sect) >= MAX_LOGICAL_BLOCK)
        return CIDEV_ADDRESS_ERROR;

    *lba = (chs->cyl * NUM_OF_HEADS * NUM_OF_SECTS) + chs->head * NUM_OF_SECTS + (chs->sect);

    return CIDEV_SUCCESS;
}
/***
 *
 * Verifies the parameters, then allocates space for the requested data (using
 * the caller-provided pointer buffer), copies the data from the disk to the buffer,
 * and appends '\0' to terminate the string.
 *  
 */
CIDEV_RET_CODE readDisk(lba_t lba, unsigned int size, char **buffer)
{
    //Verifies the size request is not larger than available space.
    // todo: verify parameters
    if (size >= MAX_LOGICAL_BLOCK*SECT_SIZE)
        return CIDEV_SPACE_ERROR;

    CIDEV_RET_CODE errCode = CIDEV_SUCCESS;
    unsigned int numSectors = 0,
    i = 0,
    sizingVar = 0;
    //How many sectors do I need to read from?
    //Convert the lba to the correct chs data structs.
    //  utilizing an array for convenient chs loading
    if(size % SECT_SIZE == 0) {
        numSectors = size / SECT_SIZE;
        sizingVar = size;
    }
    else {
        numSectors = (size / SECT_SIZE) + 1;
        sizingVar = size + (SECT_SIZE - (size % SECT_SIZE));
    }
    chs_t diskLocations[numSectors], chs;

    for (i = 0; i < numSectors; ++i) {
        if ((errCode = lba2chs((lba + i) % MAX_LOGICAL_BLOCK, &diskLocations[i])) != CIDEV_SUCCESS)
            return errCode;
    }

    // todo: implement

    *buffer = (char*)calloc(sizeof(char*), sizingVar + 1);
    printf("\nSize Variable in Read: %d\n", size);

    for (i = 0; i < numSectors; i++) {
        chs = diskLocations[i];
        if((size / SECT_SIZE) == 0)
            sizingVar = size;
        else
            sizingVar = SECT_SIZE;
        strncat(*buffer, disk[chs.cyl][chs.head][chs.sect], sizingVar);
        size -= SECT_SIZE;
    }
    strcat(*buffer, "\0");

    return errCode;
}

/***
 *
 * An auxiliary function to fill a single disk block with '.'
 *
*/
CIDEV_RET_CODE clearBlock(lba_t lba)
{
    char *writeBuffer;
#ifdef __DEBUG_DISK
    writeBuffer = malloc(SECT_SIZE);
#else
    writeBuffer = kmalloc(SECT_SIZE, GFP_USER);
#endif
    if (writeBuffer == NULL)
        return CIDEV_SPACE_ERROR;

    for (int i = 0; i < SECT_SIZE; i++)
        writeBuffer[i] = '.';

    CIDEV_RET_CODE errCode = writeDisk(lba, writeBuffer);

#ifdef __DEBUG_DISK
    free(writeBuffer);
#else
    kfree(writeBuffer);
#endif

    return errCode;
}

/***
 *
 * Validates the parameters, and then writes the caller-provided data to the disk starting at the block pointed
 * to by the logical block address.
 *
 */
CIDEV_RET_CODE writeDisk(lba_t lba, char *buffer)
{
// todo: verify the parameters

    CIDEV_RET_CODE errCode = CIDEV_SUCCESS;
    size_t  numberSector, numberChars = strlen(buffer);
    int i, sizeRemain = numberChars;

    if (numberChars % SECT_SIZE == 0)
        numberSector = numberChars / SECT_SIZE;
    else
        numberSector = numberChars / SECT_SIZE + 1;

    if (numberSector >= MAX_LOGICAL_BLOCK)
        return CIDEV_SPACE_ERROR;
    chs_t diskLocation[numberSector], chs;

    for (i = 0; i < numberSector; ++i) {
        if((errCode = lba2chs((lba + i) % MAX_LOGICAL_BLOCK, &diskLocation[i])) != CIDEV_SUCCESS)
            return errCode;
    }


    //Tester lba
//    lba_t lba2;
//    char *buffer2;
// todo: implement
    for (i = 0; i < numberSector; i++) {
        chs = diskLocation[i];

        if ((numberChars / SECT_SIZE) == 0)
            sizeRemain = numberChars;
        else
            sizeRemain = SECT_SIZE;


        strncpy(disk[chs.cyl][chs.head][chs.sect], buffer + i*SECT_SIZE, sizeRemain);
        numberChars -= SECT_SIZE;
//        chs2lba(&diskLocation[i], &lba2);
//        readDisk(lba2, sizeRemain, &buffer2);
//        printf("%s\n", buffer2);
//        fflush(stdout);
    }

    return errCode;
}
