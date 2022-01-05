/************************************************************************
 * Mikolaj Panka                                                        *
 * KSO 2021-z                                                           *
 * lab4                                                                 *
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "fifo_big.h"
#include "fifo_med.h"
#include "lifo_small.h"
#include "shared_mem.h"
#include "textcolour.h"
 
#include "simple_test.h"
#include "test_flags.h"


Fifo_big_t *getBigBuffer(int id);
Fifo_med_t *getMedBuffer(int id);
Lifo_small_t *getSmallBuffer(int id);

int main(int argc, char **argv)
{
// create big buffer
    int bigBlockId = getMemBlock(SHMEM_FILE, 0, sizeof(Fifo_big_t));
    Fifo_big_t *bigBuffer = getBigBuffer(bigBlockId);
        #ifdef DO_PREFILL
    randFillFifoBig(bigBuffer);
        #endif
/* 
// create medium buffer
    int medBlockId = getMemBlock(SHMEM_FILE, 1, sizeof(Fifo_med_t));
    Fifo_med_t *medBuffer = getMedBuffer(medBlockId);
        #ifdef DO_PREFILL
    randFillFifoMed(medBuffer);
        #endif

// create small buffer
    int smallBlockId = getMemBlock(SHMEM_FILE, 2, sizeof(Lifo_small_t));
    Lifo_small_t *smallBuffer = getSmallBuffer(smallBlockId);
        #ifdef DO_PREFILL
    randFillLifoSmall(smallBuffer);
        #endif
 */
    pid_t parentpid = getpid();
    srandom(time(NULL));

    if (fork() == 0) {  // parent spawning 1st child:
            #ifdef MP_V_VERBOSE
        printf("Child:\t\tParent executed fork(), child pid: %u. Executing consumer process - execv(\"./consumerA\", (char**){NULL})\n", getpid());
            #endif
        execv("./consumerA", (char**){NULL});
    }
    else if (fork() == 0) {// parent spawning 2nd child:
            #ifdef MP_V_VERBOSE
        textcolour(0, WHITE, BG_BLACK);
        printf("Child:\t\tParent executed fork(), child pid: %u. Executing producer process - execv(\"./producerA\", (char**){NULL})\n", getpid());
            #endif
        execv("./producerA", (char**){NULL});
    }
   /*  else if (fork() == 0)  // parent spawning 3rd child:
    {
            #ifdef MP_V_VERBOSE
        textcolour(0, WHITE, BG_BLACK);
        printf("Child:\t\tParent executed fork(), child pid: %u. Executing producer process - execv(\"./producerB\", (char**){NULL})\n", getpid());
            #endif
        execv("./producerB", (char**){NULL});
    }
    else if (fork() == 0)  // parent spawning 4th child:
    {
            #ifdef MP_V_VERBOSE
        textcolour(0, WHITE, BG_BLACK);
        printf("Child:\t\tParent executed fork(), child pid: %u. Executing consumer process - execv(\"./consumerB\", (char**){NULL})\n", getpid());
            #endif
        execv("./consumerB", (char**){NULL});
    }
    else if (fork() == 0)  // parent spawning 5th child:
    {
            #ifdef MP_V_VERBOSE
        textcolour(0, WHITE, BG_BLACK);
        printf("Child:\t\tParent executed fork(), child pid: %u. Executing producer process - execv(\"./producerC\", (char**){NULL})\n", getpid());
            #endif
        execv("./producerC", (char**){NULL});
    }
    else if (fork() == 0)  // parent spawning 6th child:
    {
            #ifdef MP_V_VERBOSE
        textcolour(0, WHITE, BG_BLACK);
        printf("Child:\t\tParent executed fork(), child pid: %u. Executing consumer process - execv(\"./consumerC\", (char**){NULL})\n", getpid());
            #endif
        execv("./consumerC", (char**){NULL});
    }
    else if (fork() == 0)  // parent spawning 7th child:
    {
            #ifdef MP_V_VERBOSE
        textcolour(0, WHITE, BG_BLACK);
        printf("Child:\t\tParent executed fork(), child pid: %u. Executing producer process - execv(\"./producerRand\", (char**){NULL})\n", getpid());
            #endif
        execv("./producerRand", (char**){NULL});
    }
    else if (fork() == 0)  // parent spawning 8th child:
    {
            #ifdef MP_V_VERBOSE
        textcolour(0, WHITE, BG_BLACK);
        printf("Child:\t\tParent executed fork(), child pid: %u. Executing consumer process - execv(\"./consumerRand\", (char**){NULL})\n", getpid());
            #endif
        execv("./consumerRand", (char**){NULL});
    }
 */
    if (getpid() == parentpid) {
        while(wait(NULL) > 0);  // wait for all consumers/producers to finish
            // #ifdef MP_V_VERBOSE
        textcolour(0, WHITE, BG_BLACK);
        printf("Parent:\t\tExiting:\n");
        printf("Parent:\t\t");
        printFifoBig(bigBuffer);
/*         printf("Parent:\t\t");
        printFifoMed(medBuffer);
        printf("Parent:\t\t");
        printLifoSmall(smallBuffer);
*/
            // #endif
        // cleanup shared memory
        shmdt(bigBuffer);
        // shmdt(medBuffer);
        // shmdt(smallBuffer);
        shmctl(bigBlockId, IPC_RMID, NULL);
        // shmctl(medBlockId, IPC_RMID, NULL);
        // shmctl(smallBlockId, IPC_RMID, NULL);
    }

    return 0;
}

Fifo_big_t *getBigBuffer(int id)
{
    Fifo_big_t *buf = attachMemBlock(id);
#ifdef TEST_FIFO_BIG
    test_FifoBig(buf);
#endif
#ifndef TEST_FIFO_BIG
    initFifoBig(buf);
#endif
    return buf;
}

Fifo_med_t *getMedBuffer(int id)
{
    Fifo_med_t *buf = attachMemBlock(id);
#ifdef TEST_FIFO_MED
    test_FifoMed(buf);
#endif
#ifndef TEST_FIFO_MED
    initFifoMed(buf);
#endif
    return buf;
}

Lifo_small_t *getSmallBuffer(int id)
{
    Lifo_small_t *buf = attachMemBlock(id);
#ifdef TEST_LIFO_SMALL
    test_LifoSmall(buf);
#endif
#ifndef TEST_LIFO_SMALL
    initLifoSmall(buf);
#endif
    return buf;
}
