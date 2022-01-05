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


#define NEED_CAP    10


void consumeBig(int amount);
void consumeMed(int amount);
void consumeSmall(int amount);


Fifo_big_t      *bigBuffer;
Fifo_med_t      *medBuffer;
Lifo_small_t    *smallBuffer;
int             pid;
int             roundNo = 0;
long            totalWait = 0;


int main(int argc, char **argv)
{
    int need;
    pid = getpid();

    int bigBlockId = getMemBlock(SHMEM_FILE, 0, sizeof(Fifo_big_t));
    bigBuffer = attachMemBlock(bigBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\tAttached to shared big buffer:\n", pid);
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t", pid);
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printFifoBig(bigBuffer);
        #endif
    sem_wait(&bigBuffer->mutex);
    roundNo = bigBuffer->capacity * ROUND_MULT;
    sem_post(&bigBuffer->mutex);
    int medBlockId = getMemBlock(SHMEM_FILE, 1, sizeof(Fifo_med_t));
    medBuffer = attachMemBlock(medBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\tAttached to shared medium buffer:\n", pid);
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t", pid);
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printFifoMed(medBuffer);
        #endif
    int smallBlockId = getMemBlock(SHMEM_FILE, 2, sizeof(Lifo_small_t));
    smallBuffer = attachMemBlock(smallBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\tAttached to shared small buffer:\n", pid);
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t", pid);
    textcolour(0, BRIGHT_WHITE, BG_BLACK); printLifoSmall(smallBuffer);
        #endif

    while(roundNo)
    {
        --roundNo;
        need = random() % NEED_CAP;  // how much data needed by consumer
            #ifdef MP_VERBOSE
        textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\trun: %u, need: %u. \n", pid, roundNo, need);
            #endif
        switch (random() % 3)  // random select buffer to consume from
        {
        case 0:
            consumeBig(need);
            break;
        case 1:
            consumeMed(need);
            break;
        case 2:
            consumeSmall(need);
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

void consumeBig(int amount)
{
    int toConsume;
    while (amount > 0)
    {
            #ifdef MP_VERBOSE
        textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t[   Big Buffer  ]\trun: %u, need: %u. ", pid, roundNo, amount);
            #endif
        sem_wait(&bigBuffer->mutex);
        if (bigBuffer->size > 0)
        {
            // Checking borderline conditions:
            if (bigBuffer->size < bigBuffer->chunk)  // Buffer is less than 1 chunk full
            {
                if (amount <= bigBuffer->size)  // Buffer has enough to satisfy the need
                {
                    toConsume = amount;
                    amount = 0;
                }
                else  // Buffer hasn't got enough
                {
                    toConsume = bigBuffer->size;
                    amount -= toConsume;
                }
            }
            else  // Buffer is at least 1 chunk full
            {
                if (amount <= bigBuffer->chunk)  // Need can be satisfied from 1 chunk
                {
                    toConsume = amount;
                    amount = 0;
                }
                else  // Need can't be satisfied from 1 chunk
                {
                    toConsume = bigBuffer->chunk;
                    amount -= toConsume;
                }
            }
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, BRIGHT_WHITE, BG_BLACK); printf("Consuming %u units\n", toConsume);
                #endif
            for (size_t i = 0; i < toConsume; i++)
            {
                sem_wait(&bigBuffer->semFull);
                popFifoBig(bigBuffer);
                sem_post(&bigBuffer->semEmpty);
            }
            sem_post(&bigBuffer->mutex);
        }
        else  // buffer is empty
        {
            sem_post(&bigBuffer->mutex);
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, BRIGHT_WHITE, BG_BLACK); printf("Waiting for %u more units\n", amount);
                #endif
                #ifdef DO_WAIT
            usleep(USEC);
                #endif
                #ifdef DO_TIMEOUT
            totalWait += USEC;
            if (totalWait > WAIT_CAP)
            {
                amount = 0;  // break out of local while()
                roundNo = 0;  // break out of main while()
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t[   Big Buffer  ]\tWaiting timed-out - exiting.\n", pid);
                    #ifdef MP_V_VERBOSE
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\tFinishing:\n", pid);
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t", pid);
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printFifoBig(bigBuffer);
                    #endif
            }
                #endif
        }
    }
}

void consumeMed(int amount)
{
    int toConsume;
    while (amount > 0)
    {
            #ifdef MP_VERBOSE
        textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t[ Medium Buffer ]\trun: %u, need: %u. ", pid, roundNo, amount);
            #endif
        sem_wait(&medBuffer->mutex);
        if (medBuffer->size > 0)
        {
            if (medBuffer->size < medBuffer->chunk)
            {
                if (amount <= medBuffer->size)
                {
                    toConsume = amount;
                    amount = 0;
                }
                else
                {
                    toConsume = medBuffer->size;
                    amount -= toConsume;
                }
            }
            else
            {
                if (amount <= medBuffer->chunk)
                {
                    toConsume = amount;
                    amount = 0;
                }
                else
                {
                    toConsume = medBuffer->chunk;
                    amount -= toConsume;
                }
            }
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, BRIGHT_WHITE, BG_BLACK); printf("Consuming %u units\n", toConsume);
                #endif
            for (size_t i = 0; i < toConsume; i++)
            {
                sem_wait(&medBuffer->semFull);
                popFifoMed(medBuffer);
                sem_post(&medBuffer->semEmpty);
            }
            sem_post(&medBuffer->mutex);
        }
        else
        {
            sem_post(&medBuffer->mutex);
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, BRIGHT_WHITE, BG_BLACK); printf("Waiting for %u more units\n", amount);
                #endif
                #ifdef DO_WAIT
            usleep(USEC);
                #endif
                #ifdef DO_TIMEOUT
            totalWait += USEC;
            if (totalWait > WAIT_CAP)
            {
                amount = 0;
                roundNo = 0;
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t[ Medium Buffer ]\tWaiting timed-out - exiting.\n", pid);
                    #ifdef MP_V_VERBOSE
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\tFinishing:\n", pid);
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t", pid);
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printFifoMed(medBuffer);
                    #endif
            }
                #endif
        }
    }
}

void consumeSmall(int amount)
{
    int toConsume;
    while (amount > 0)
    {
            #ifdef MP_VERBOSE
        textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t[  Small Buffer ]\trun: %u, need: %u. ", pid, roundNo, amount);
            #endif
        sem_wait(&smallBuffer->mutex);
        if (smallBuffer->size > 0)
        {
            if (smallBuffer->size < smallBuffer->chunk)
            {
                if (amount <= smallBuffer->size)
                {
                    toConsume = amount;
                    amount = 0;
                }
                else
                {
                    toConsume = smallBuffer->size;
                    amount -= toConsume;
                }
            }
            else
            {
                if (amount <= smallBuffer->chunk)
                {
                    toConsume = amount;
                    amount = 0;
                }
                else
                {
                    toConsume = smallBuffer->chunk;
                    amount -= toConsume;
                }
            }
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, BRIGHT_WHITE, BG_BLACK); printf("Consuming %u units\n", toConsume);
                #endif
            for (size_t i = 0; i < toConsume; i++)
            {
                sem_wait(&smallBuffer->semFull);
                popLifoSmall(smallBuffer);
                sem_post(&smallBuffer->semEmpty);
            }
            sem_post(&smallBuffer->mutex);
        }
        else
        {
            sem_post(&smallBuffer->mutex);
                #ifdef MP_VERBOSE
            textcolour(UNDERLINE, BRIGHT_WHITE, BG_BLACK); printf("Waiting for %u more units\n", amount);
                #endif
                #ifdef DO_WAIT
            usleep(USEC);
                #endif
                #ifdef DO_TIMEOUT
            totalWait += USEC;
            if (totalWait > WAIT_CAP)
            {
                amount = 0;
                roundNo = 0;
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t[  Small Buffer ]\tWaiting timed-out - exiting.\n", pid);
                    #ifdef MP_V_VERBOSE
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\tFinishing:\n", pid);
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printf("Consumer Random:\t%u\t", pid);
                textcolour(0, BRIGHT_WHITE, BG_BLACK); printLifoSmall(smallBuffer);
                    #endif
            }
                #endif
        }
    }
}
