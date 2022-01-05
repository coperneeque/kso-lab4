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


#define PROD_CAP    10
#define RANGE       100

int main(int argc, char **argv)
{
    int roundNo = 0;
    int produced;
    int products[PROD_CAP];
    int toInsert;
    int bufEmpty;
    int pid = getpid();
    long totalWait = 0;

    int smallBlockId = getMemBlock(SHMEM_FILE, 2, sizeof(Lifo_small_t));
    Lifo_small_t *smallBuffer = attachMemBlock(smallBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, CYAN, BG_BLACK); printf("Producer C:\t\t%u\tAttached to shared small buffer:\n", pid);
    textcolour(0, CYAN, BG_BLACK); printf("Producer C:\t\t%u\t", pid);
    textcolour(0, CYAN, BG_BLACK); printLifoSmall(smallBuffer);
        #endif
    sem_wait(&smallBuffer->mutex);
    roundNo = smallBuffer->capacity * ROUND_MULT;
    sem_post(&smallBuffer->mutex);

    while(roundNo) {
        --roundNo;
        produced = random() % PROD_CAP;
        for (size_t i = 0; i < produced; i++) {
            products[i] = random() % RANGE;
        }
            #ifdef MP_VERBOSE
        textcolour(0, CYAN, BG_BLACK); printf("Producer C:\t\t%u\t\t\t\trun: %u, produced: %u units\n", pid, roundNo, produced);
            #endif
        while (produced > 0) {
                #ifdef MP_VERBOSE
            textcolour(0, CYAN, BG_BLACK); printf("Producer C:\t\t%u\t\t\t\trun: %u, to insert: %u units. ", pid, roundNo, produced);
                #endif
            sem_wait(&smallBuffer->mutex);
            bufEmpty = smallBuffer->capacity - smallBuffer->size;
            if (bufEmpty > 0) {
                if (bufEmpty < smallBuffer->chunk) {
                    if (produced <= bufEmpty) {
                        toInsert = produced;
                        produced = 0;
                    } else {
                        toInsert = bufEmpty;
                        produced -= toInsert;
                    }
                } else {
                    if (produced <= smallBuffer->chunk) {
                        toInsert = produced;
                        produced = 0;
                    } else {
                        toInsert = smallBuffer->chunk;
                        produced -= toInsert;
                    }
                }
                    #ifdef MP_VERBOSE
                textcolour(UNDERLINE, CYAN, BG_BLACK); printf("Inserting %u units\n", toInsert);
                    #endif
                for (size_t i = 0; i < toInsert; i++) {
                    sem_wait(&smallBuffer->semEmpty);
                    putLifoSmall(smallBuffer, products[i]);
                    sem_post(&smallBuffer->semFull);
                }
                sem_post(&smallBuffer->mutex);
            } else {
                sem_post(&smallBuffer->mutex);
                produced = 0;
                    #ifdef MP_VERBOSE
                textcolour(UNDERLINE, CYAN, BG_BLACK); printf("No space in buffer - dropping data.\n");
                    #endif
                    #ifdef DO_WAIT
                usleep(USEC);
                    #endif
                    #ifdef DO_TIMEOUT
                totalWait += USEC;
                if (totalWait > WAIT_CAP) {
                    produced = 0;
                    roundNo = 0;
                    textcolour(0, CYAN, BG_BLACK); printf("Producer C:\t\t%u\tWaiting timed-out - exiting.\n", pid);
                }
                    #endif
            } 
        }        
    }

    textcolour(0, CYAN, BG_BLACK); printf("Producer C:\t\t%u\tFinishing:\n", pid);
        #ifdef MP_V_VERBOSE
    textcolour(0, CYAN, BG_BLACK); printf("Producer C:\t\t%u\t", pid);
    textcolour(0, CYAN, BG_BLACK); printLifoSmall(smallBuffer);
        #endif
    shmdt(smallBuffer);

    return 0;
}
