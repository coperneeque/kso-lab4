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

#include "fifo_med.h"
#include "shared_mem.h"
#include "test_flags.h"
#include "textcolour.h"


#define NEED_CAP    10

int main(int argc, char **argv)
{
    int roundNo = 0;
    int need;
    int toConsume;
    int pid = getpid();
    long totalWait = 0;

    int medBlockId = getMemBlock(SHMEM_FILE, 1, sizeof(Fifo_med_t));
    Fifo_med_t *medBuffer = attachMemBlock(medBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, YELLOW, BG_BLACK); printf("Consumer B:\t\t%u\tAttached to shared medium buffer:\n", pid);
    textcolour(0, YELLOW, BG_BLACK); printf("Consumer B:\t\t%u\t", pid);
    textcolour(0, YELLOW, BG_BLACK); printFifoMed(medBuffer);
        #endif
    sem_wait(&medBuffer->mutex);
    roundNo = medBuffer->capacity * ROUND_MULT;
    sem_post(&medBuffer->mutex);

    while(roundNo)
    {
        --roundNo;
        need = random() % NEED_CAP;
            #ifdef MP_VERBOSE
        textcolour(0, YELLOW, BG_BLACK); printf("Consumer B:\t\t%u\t\t\t\trun: %u, need: %u. \n", pid, roundNo, need);
            #endif
        while (need > 0)
        {
                #ifdef MP_VERBOSE
            textcolour(0, YELLOW, BG_BLACK); printf("Consumer B:\t\t%u\t\t\t\trun: %u, need: %u. ", pid, roundNo, need);
                #endif
            sem_wait(&medBuffer->mutex);
            if (medBuffer->size > 0)
            {
                if (medBuffer->size < medBuffer->chunk)
                {
                    if (need <= medBuffer->size)
                    {
                        toConsume = need;
                        need = 0;
                    }
                    else
                    {
                        toConsume = medBuffer->size;
                        need -= toConsume;
                    }
                }
                else
                {
                    if (need <= medBuffer->chunk)
                    {
                        toConsume = need;
                        need = 0;
                    }
                    else
                    {
                        toConsume = medBuffer->chunk;
                        need -= toConsume;
                    }
                }
                    #ifdef MP_VERBOSE
                textcolour(UNDERLINE, YELLOW, BG_BLACK); printf("Consuming %u units\n", toConsume);
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
                textcolour(UNDERLINE, YELLOW, BG_BLACK); printf("Waiting for %u more units\n", need);
                    #endif
                    #ifdef DO_WAIT
                usleep(USEC);
                    #endif
                    #ifdef DO_TIMEOUT
                totalWait += USEC;
                if (totalWait > WAIT_CAP)
                {
                    need = 0;
                    roundNo = 0;
                    textcolour(0, YELLOW, BG_BLACK); printf("Consumer B:\t\t%u\tWaiting timed-out - exiting.\n", pid);
                }
                    #endif
            }
        }        
    }

    textcolour(0, YELLOW, BG_BLACK); printf("Consumer B:\t\t%u\tFinishing:\n", pid);
        #ifdef MP_V_VERBOSE
    textcolour(0, YELLOW, BG_BLACK); printf("Consumer B:\t\t%u\t", pid);
    textcolour(0, YELLOW, BG_BLACK); printFifoMed(medBuffer);
        #endif
    shmdt(medBuffer);

    return 0;
}
