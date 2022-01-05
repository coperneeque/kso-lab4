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

#include "lifo_small.h"
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

    int smallBlockId = getMemBlock(SHMEM_FILE, 2, sizeof(Lifo_small_t));
    Lifo_small_t *smallBuffer = attachMemBlock(smallBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, MAGENTA, BG_BLACK); printf("Consumer C:\t\t%u\tAttached to shared small buffer:\n", pid);
    textcolour(0, MAGENTA, BG_BLACK); printf("Consumer C:\t\t%u\t", pid);
    textcolour(0, MAGENTA, BG_BLACK); printLifoSmall(smallBuffer);
        #endif
    sem_wait(&smallBuffer->mutex);
    roundNo = smallBuffer->capacity * ROUND_MULT;
    sem_post(&smallBuffer->mutex);

    while(roundNo)
    {
        --roundNo;
        need = random() % NEED_CAP;
            #ifdef MP_VERBOSE
        textcolour(0, MAGENTA, BG_BLACK); printf("Consumer C:\t\t%u\t\t\t\trun: %u, need: %u. \n", pid, roundNo, need);
            #endif
        while (need > 0)
        {
                #ifdef MP_VERBOSE
            textcolour(0, MAGENTA, BG_BLACK); printf("Consumer C:\t\t%u\t\t\t\trun: %u, need: %u. ", pid, roundNo, need);
                #endif
            sem_wait(&smallBuffer->mutex);
            if (smallBuffer->size > 0)
            {
                if (smallBuffer->size < smallBuffer->chunk)
                {
                    if (need <= smallBuffer->size)
                    {
                        toConsume = need;
                        need = 0;
                    }
                    else
                    {
                        toConsume = smallBuffer->size;
                        need -= toConsume;
                    }
                }
                else
                {
                    if (need <= smallBuffer->chunk)
                    {
                        toConsume = need;
                        need = 0;
                    }
                    else
                    {
                        toConsume = smallBuffer->chunk;
                        need -= toConsume;
                    }
                }
                    #ifdef MP_VERBOSE
                textcolour(UNDERLINE, MAGENTA, BG_BLACK); printf("Consuming %u units\n", toConsume);
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
                textcolour(UNDERLINE, MAGENTA, BG_BLACK); printf("Waiting for %u more units\n", need);
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
                    textcolour(0, MAGENTA, BG_BLACK); printf("Consumer C:\t\t%u\tWaiting timed-out - exiting.\n", pid);
                }
                    #endif
            }
        }        
    }

    textcolour(0, MAGENTA, BG_BLACK); printf("Consumer C:\t\t%u\tFinishing:\n", pid);
        #ifdef MP_V_VERBOSE
    textcolour(0, MAGENTA, BG_BLACK); printf("Consumer C:\t\t%u\t", pid);
    textcolour(0, MAGENTA, BG_BLACK); printLifoSmall(smallBuffer);
        #endif
    shmdt(smallBuffer);

    return 0;
}
