/************************************************************************
 * Mikolaj Panka                                                        *
 * KSO 2021-z                                                           *
 * lab3                                                                 *
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "fifo_big.h"
#include "fifo_med.h"
#include "lifo_small.h"
#include "shared_mem.h"
#include "test_flags.h"
#include "textcolour.h"


#define PROD_CAP    10
#define RANGE       100


void insertBig(int amount);
void insertMed(int amount);
void insertSmall(int amount);


Fifo_big_t      *bigBuffer;
Fifo_med_t      *medBuffer;
Lifo_small_t    *smallBuffer;
int             products[PROD_CAP];
int             pid;
int             roundNo = 0;
long            totalWait = 0;


int main(int argc, char **argv)
{
    int produced;
    pid = getpid();

// attach to existing buffers
    int bigBlockId = getMemBlock(SHMEM_FILE, 0, sizeof(Fifo_big_t));
    bigBuffer = attachMemBlock(bigBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\tAttached to shared big buffer:\n", pid);
    textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t", pid);
    textcolour(0, GRAY, BG_BLACK); printFifoBig(bigBuffer);
        #endif
    sem_wait(&bigBuffer->mutex);
    roundNo = bigBuffer->capacity * ROUND_MULT;
    sem_post(&bigBuffer->mutex);
    int medBlockId = getMemBlock(SHMEM_FILE, 1, sizeof(Fifo_med_t));
    medBuffer = attachMemBlock(medBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\tAttached to shared medium buffer:\n", pid);
    textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t", pid);
    textcolour(0, GRAY, BG_BLACK); printFifoMed(medBuffer);
        #endif
    int smallBlockId = getMemBlock(SHMEM_FILE, 2, sizeof(Lifo_small_t));
    smallBuffer = attachMemBlock(smallBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\tAttached to shared small buffer:\n", pid);
    textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t", pid);
    textcolour(0, GRAY, BG_BLACK); printLifoSmall(smallBuffer);
        #endif

    while(roundNo) {
        --roundNo;
        produced = random() % PROD_CAP;
        for (size_t i = 0; i < produced; i++) {
            products[i] = random() % RANGE;
        }
            #ifdef MP_VERBOSE
        textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\trun: %u, produced: %u units\n", pid, roundNo, produced);
            #endif
        switch (random() % 3)  // random select buffer to insert into
        {
        case 0:
            insertBig(produced);
            break;
        case 1:
            insertMed(produced);
            break;
        case 2:
            insertSmall(produced);
            break;
        default:
            break;
        }
    }
    shmdt(smallBuffer);
    shmdt(medBuffer);
    shmdt(bigBuffer);

    return 0;
}

void insertBig(int amount)
{
    int bufEmpty;
    int toInsert;

    while (amount > 0) {
            #ifdef MP_VERBOSE
        textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t[   Big Buffer  ]\trun: %u, to insert: %u units. ", pid, roundNo, amount);
            #endif
        sem_wait(&bigBuffer->mutex);
        bufEmpty = bigBuffer->capacity - bigBuffer->size;
        if (bufEmpty > 0) {
            if (bufEmpty < bigBuffer->chunk) {  // buffer is less than 1 chunk empty
                if (amount <= bufEmpty) {
                    toInsert = amount;
                    amount = 0;
                } else {
                    toInsert = bufEmpty;
                    amount -= toInsert;
                }
            } else {  // buffer is at least 1 chunk empty
                if (amount <= bigBuffer->chunk) {
                    toInsert = amount;
                    amount = 0;
                } else {  // produced data will not fit in 1 chunk
                    toInsert = bigBuffer->chunk;
                    amount -= toInsert;
                }
            }
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, GRAY, BG_BLACK); printf("Inserting %u units\n", toInsert);
                #endif
            for (size_t i = 0; i < toInsert; i++) {
                sem_wait(&bigBuffer->semEmpty);
                putFifoBig(bigBuffer, products[i]);
                sem_post(&bigBuffer->semFull);
            }
            sem_post(&bigBuffer->mutex);
        } else {  // no space in the buffer
            sem_post(&bigBuffer->mutex);
            amount = 0;
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, GRAY, BG_BLACK); printf("No space in buffer - dropping data.\n");
                #endif
                #ifdef DO_WAIT
            usleep(USEC);
                #endif
                #ifdef DO_TIMEOUT
            totalWait += USEC;
            if (totalWait > WAIT_CAP) {
                amount = 0;  // break out of local while()
                roundNo = 0;  // break out of main while()
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t[   Big Buffer  ]\tWaiting timed-out - exiting.\n", pid);
                    #ifdef MP_V_VERBOSE
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\tFinishing:\n", pid);
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t", pid);
                textcolour(0, GRAY, BG_BLACK); printFifoBig(bigBuffer);
                    #endif
            }
                #endif
        }
    }
}

void insertMed(int amount)
{
    int bufEmpty;
    int toInsert;

    while (amount > 0) {
            #ifdef MP_VERBOSE
        textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t[ Medium Buffer ]\trun: %u, to insert: %u units. ", pid, roundNo, amount);
            #endif
        sem_wait(&medBuffer->mutex);
        bufEmpty = medBuffer->capacity - medBuffer->size;
        if (bufEmpty > 0) {
            if (bufEmpty < medBuffer->chunk) {
                if (amount <= bufEmpty) {
                    toInsert = amount;
                    amount = 0;
                } else {
                    toInsert = bufEmpty;
                    amount -= toInsert;
                }
            } else {
                if (amount <= medBuffer->chunk) {
                    toInsert = amount;
                    amount = 0;
                } else {
                    toInsert = medBuffer->chunk;
                    amount -= toInsert;
                }
            }
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, GRAY, BG_BLACK); printf("Inserting %u units\n", toInsert);
                #endif
            for (size_t i = 0; i < toInsert; i++) {
                sem_wait(&medBuffer->semEmpty);
                putFifoMed(medBuffer, products[i]);
                sem_post(&medBuffer->semFull);
            }
            sem_post(&medBuffer->mutex);
        } else {
            sem_post(&medBuffer->mutex);
            amount = 0;
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, GRAY, BG_BLACK); printf("No space in buffer - dropping data.\n");
                #endif
                #ifdef DO_WAIT
            usleep(USEC);
                #endif
                #ifdef DO_TIMEOUT
            totalWait += USEC;
            if (totalWait > WAIT_CAP) {
                amount = 0;
                roundNo = 0;
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t[ Medium Buffer ]\tWaiting timed-out - exiting.\n", pid);
                    #ifdef MP_V_VERBOSE
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\tFinishing:\n", pid);
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t", pid);
                textcolour(0, GRAY, BG_BLACK); printFifoMed(medBuffer);
                    #endif
            }
                #endif
        }
    }
}

void insertSmall(int amount)
{
    int bufEmpty;
    int toInsert;

    while (amount > 0) {
            #ifdef MP_VERBOSE
        textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t[  Small Buffer ]\trun: %u, to insert: %u units. ", pid, roundNo, amount);
            #endif
        sem_wait(&smallBuffer->mutex);
        bufEmpty = smallBuffer->capacity - smallBuffer->size;
        if (bufEmpty > 0) {
            if (bufEmpty < smallBuffer->chunk) {
                if (amount <= bufEmpty) {
                    toInsert = amount;
                    amount = 0;
                } else {
                    toInsert = bufEmpty;
                    amount -= toInsert;
                }
            } else {
                if (amount <= smallBuffer->chunk) {
                    toInsert = amount;
                    amount = 0;
                } else {
                    toInsert = smallBuffer->chunk;
                    amount -= toInsert;
                }
            }
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, GRAY, BG_BLACK); printf("Inserting %u units\n", toInsert);
                #endif
            for (size_t i = 0; i < toInsert; i++) {
                sem_wait(&smallBuffer->semEmpty);
                putLifoSmall(smallBuffer,products[i]);
                sem_post(&smallBuffer->semFull);
            }
            sem_post(&smallBuffer->mutex);
        } else {
            sem_post(&smallBuffer->mutex);
            amount = 0;
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, GRAY, BG_BLACK); printf("No space in buffer - dropping data.\n");
                #endif
                #ifdef DO_WAIT
            usleep(USEC);
                #endif
                #ifdef DO_TIMEOUT
            totalWait += USEC;
            if (totalWait > WAIT_CAP) {
                amount = 0;
                roundNo = 0;
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t[  Small Buffer ]\tWaiting timed-out - exiting.\n", pid);
                    #ifdef MP_V_VERBOSE
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\tFinishing:\n", pid);
                textcolour(0, GRAY, BG_BLACK); printf("Producer Random:\t%u\t", pid);
                textcolour(0, GRAY, BG_BLACK); printLifoSmall(smallBuffer);
                    #endif
            }
                #endif
        }
    }
}
