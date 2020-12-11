/*
**Name: Travis Chamness
**Project: 2 Task 2
**Date: 12/9/2020
*/

//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2020 Prof. AJ Bieszczad. All rights reserved.
///
//////////////////////////////////////////////////////////////////////////

#include "disk.h"

#ifdef __DEBUG_DISK

int static numberOfTrials; // global to allow passing it to the timeout handler

/**
 * Conditional compilation for testing the functionality of the simulated disk
 */

/***
 *
 * Call the address translation functions and prints LBA with its CHS equivalent
 *
 */
void printTransl(lba_t lba)
{
   chs_t chs;
   lba_t computedLba;

   printf("\nLogical sector number: %ld\n", lba);
   if (lba2chs(lba, &chs) == 0)
   {
      printf("Physical address: (%d, %d, %d)\n", chs.cyl, chs.head, chs.sect);

      chs2lba(&chs, &computedLba);
      if (computedLba != lba)
         printf("ERROR: given logical address %ld does no match the computed value %ld!\n", lba, computedLba);
   } else
      printf("ERROR: invalid logical address!\n");
}

/***
*
* Generates random readable/printable content for testing
*
*/
char *generatePrintableContent(int size)
{
   char *content = malloc(size + 1);

   int firstPrintable = ' ';
   int lastPrintable = '~';
   int len = lastPrintable - firstPrintable;

   for (int i = 0; i < size; i++)
      *(content + i) = firstPrintable + rand() % len;

   content[size] = '\0';
   return content;
}

/***
 *
 * This function verifies matches in writing and reading same part of disks.
 *
 * The locations and the chunk size are generated randomly.
 *
 */
void testReadWrite()
{
   lba_t lba;

   CIDEV_RET_CODE errorCode;

   char *readBuffer = NULL;
   char *writeBuffer = NULL;

   for (lba_t blockNum = 0; blockNum < MAX_LOGICAL_BLOCK; blockNum++)
      clearBlock(blockNum);

   int size; // of the read or written chunk

   for (int i = 0; i < numberOfTrials; i++)
   {
      size = (((float) rand()) / RAND_MAX) * (MAX_LOGICAL_BLOCK * SECT_SIZE) + 1;
      lba = (((float) rand()) / RAND_MAX) * MAX_LOGICAL_BLOCK;

       printf("\nSize in TestDisk: %d\n", size);

       writeBuffer = generatePrintableContent(size);
      if ((errorCode = writeDisk(lba, writeBuffer)) != CIDEV_SUCCESS)
      {
         printf("\n*** ERROR WRITING %d BYTES: %d\n\n", size, errorCode);
         free(writeBuffer);
         continue;
      }
      else
         printf("WROTE %d bytes:\n%s\n", size, writeBuffer);


      if ((errorCode = readDisk(lba, size, &readBuffer)) != CIDEV_SUCCESS)
      {
         printf("\n*** ERROR READING %d BYTES: %d\n\n", size, errorCode);
         continue;
      }
      else
         printf("READ %ld bytes:\n%s\n", strlen(readBuffer), readBuffer);

      if (strcmp(writeBuffer, readBuffer) == 0)
         printf("\nSUCCESS: READ AND WRITE MATCH.\n\n");
      else
         printf("\n*** FAILURE IN TEST 1: READ AND WRITE DO NOT MATCH.\n\n");

      free(writeBuffer);
      free(readBuffer);
   }
}

/***
 *
 * The following code depends on an optional POSIX extension real-time timers.
 * The functions require linking with the "rt" library.
 *
 */
#if defined( __DEBUG_TIMER ) && defined( __linux__ )

#define TIMEOUT_DELAY_SEC 1
#define TIMEOUT_DELAY_NANO 0

timer_t gTimerid;

void start_timer(void) {
    struct itimerspec value;

    value.it_value.tv_sec = TIMEOUT_DELAY_SEC;
    value.it_value.tv_nsec = TIMEOUT_DELAY_NANO;

    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = rand() % 100000000; // up to 100 ms (==> 10 or more per second)

    timer_create(CLOCK_REALTIME, NULL, &gTimerid);

    timer_settime(gTimerid, 0, &value, NULL);
}

void stop_timer(void) {
    struct itimerspec value;

    value.it_value.tv_sec = 0;
    value.it_value.tv_nsec = 0;

    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = 0;

    timer_settime(gTimerid, 0, &value, NULL);
}

/***
 
 * This function stress-tests the disk operation.
 *
 * It uses a timer to perform writing or reading randomly with random locations and chunk sizes.
 *
 * It works only on systems that implement the POSIX real-time extension to timers; e.g., Linux, but not macOS.
 *
 */
void testWithTime(int sig) {
    struct timeval ts;
    time_t tm;

    time(&tm); // man 3 time
    printf("\n\nTime: %s", ctime(&tm)); // man ctime ; Time Printed

    gettimeofday(&ts, NULL); // man gettimeofday
    printf("Time: %ld.%06ld secs.\n", (long) ts.tv_sec, (long) ts.tv_usec);

    lba_t lba;

    CIDEV_RET_CODE errorCode;

    char *readBuffer = NULL;
    char *writeBuffer = NULL;

    for (lba_t blockNum = 0; blockNum < MAX_LOGICAL_BLOCK; blockNum++)
        clearBlock(blockNum);

    unsigned int size; // of the read or written chunk

    lba = rand() % MAX_LOGICAL_BLOCK + 1;
    //Print lba & chs done
    printTransl(lba);

    size = (((float) rand() / RAND_MAX)) * (MAX_LOGICAL_BLOCK * SECT_SIZE) + 1;

    switch (rand() % 2) {
        case 0:
	        printf("\nREAD TYPE\n");
            if ((errorCode = readDisk(lba, size, (void *) &readBuffer)) == CIDEV_SUCCESS) {
                printf("READ %d bytes:\n%s\n", size, readBuffer);
                free(readBuffer);
            } else
                printf("\n*** ERROR READING %d BYTES: %d\n\n", size, errorCode);

            break;

        case 1:
            writeBuffer = generatePrintableContent(size);
    	    printf("\nWRITE TYPE\n\n");
            if ((errorCode = writeDisk(lba, writeBuffer)) == CIDEV_SUCCESS){
                printf("WROTE %d bytes:\n%s\n", size, writeBuffer);
//		printf("CHS: Cyl %d, Head %d, Sect %d\n", chs->cyl, chs->head, chs->sect);

	    }
	    else
                printf("\n*** ERROR WRITING %d BYTES: %d\n\n", size, errorCode);

            free(writeBuffer);

            break;
    }

    if (numberOfTrials-- <= 0)
    {
        stop_timer();
        exit(EXIT_SUCCESS);
    }
}

#  endif

int main(int argc, char *argv[])
{
   srand((unsigned int) time(NULL));

   chs_t chs;
   lba_t lba;

   if (argc < 2)
   {
      printf("\nUSAGE: test_disk [ l <lba> | p <cyl> <head> <sect> | <num of trials> ]\n");
      exit(EXIT_FAILURE);
   }

   ///////////////////////////////////////////////////////////////
   ///
   /// test a single address translation
   ///
   /// either 'l' for lba -> chs or 'p' for chs -> lba
   ///
   ///////////////////////////////////////////////////////////////
   switch (argv[1][0]) // "l" for logical "p" for physical
   {
      case 'l': // logical
         if (argv[2] == NULL)
            break;
         lba = strtol(argv[2], NULL, 10) % MAX_LOGICAL_BLOCK;
         printTransl(lba);
         exit(EXIT_SUCCESS);

      case 'p': // physical
         if (argv[2] == NULL || argv[3] == NULL || argv[4] == NULL)
            break;
         chs.cyl = strtol(argv[2], NULL, 10);
         chs.head = strtol(argv[3], NULL, 10);
         chs.sect = strtol(argv[4], NULL, 10);
         chs2lba(&chs, &lba);
         printTransl(lba);
         exit(EXIT_SUCCESS);
   }

   ///////////////////////////////////////////////////////////////
   ///
   /// if neither "l" nor "p" than must be the number of trials
   ///

   numberOfTrials = strtol(argv[1], NULL, 10); // set the global to pass the value to the timeout handler

   ///////////////////////////////////////////////////////////////
   ///
   /// Test whether random writes and reads from the same locations match
   ///

   testReadWrite();

   ///////////////////////////////////////////////////////////////
   ///
   /// Stress-test writing and reading with a timer
   ///

#if defined( __DEBUG_TIMER ) && defined ( __linux__ )

   srand(time(NULL));
   printf("\n======= Entering Timer=========\n");
   signal(SIGALRM, testWithTime);
   start_timer();

   while (true); // let the timeout handler deal with exiting

   printf("\n======== Exiting Timer =========\n");
#  endif
}

#endif
