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


#define PROD_CAP    10
#define RANGE       100

int main(int argc, char **argv)
{
    int roundNo;
    int produced;
    int products[PROD_CAP];
    int toInsert;
    int bufEmpty;
    int pid = getpid();
    long totalWait = 0;

    int medBlockId = getMemBlock(SHMEM_FILE, 1, sizeof(Fifo_med_t));
    Fifo_med_t *medBuffer = attachMemBlock(medBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, BLUE, BG_BLACK); printf("Producer B:\t\t%u\tAttached to shared medium buffer:\n", pid);
    textcolour(0, BLUE, BG_BLACK); printf("Producer B:\t\t%u\t", pid);
    textcolour(0, BLUE, BG_BLACK); printFifoMed(medBuffer);
        #endif
    sem_wait(&medBuffer->mutex);
    roundNo = medBuffer->capacity * ROUND_MULT;
    sem_post(&medBuffer->mutex);

    while(roundNo) {
        --roundNo;
        produced = random() % PROD_CAP;
        for (size_t i = 0; i < produced; i++) {
            products[i] = random() % RANGE;
        }
            #ifdef MP_VERBOSE
        textcolour(0, BLUE, BG_BLACK); printf("Producer B:\t\t%u\t\t\t\trun: %u, produced: %u units\n", pid, roundNo, produced);
            #endif
        while (produced > 0) {
                #ifdef MP_VERBOSE
            textcolour(0, BLUE, BG_BLACK); printf("Producer B:\t\t%u\t\t\t\trun: %u, to insert: %u units. ", pid, roundNo, produced);
                #endif
            sem_wait(&medBuffer->mutex);
            bufEmpty = medBuffer->capacity - medBuffer->size;
            if (bufEmpty > 0) {
                if (bufEmpty < medBuffer->chunk) {
                    if (produced <= bufEmpty) {
                        toInsert = produced;
                        produced = 0;
                    } else {
                        toInsert = bufEmpty;
                        produced -= toInsert;
                    }
                } else {
                    if (produced <= medBuffer->chunk) {
                        toInsert = produced;
                        produced = 0;
                    } else {
                        toInsert = medBuffer->chunk;
                        produced -= toInsert;
                    }
                }
                    #ifdef MP_VERBOSE
                textcolour(UNDERLINE, BLUE, BG_BLACK); printf("Inserting %u units\n", toInsert);
                    #endif
                for (size_t i = 0; i < toInsert; i++) {
                    sem_wait(&medBuffer->semEmpty);
                    putFifoMed(medBuffer,products[i]);
                    sem_post(&medBuffer->semFull);
                }
                sem_post(&medBuffer->mutex);
            } else {
                sem_post(&medBuffer->mutex);
                produced = 0;
                    #ifdef MP_VERBOSE
                textcolour(UNDERLINE, BLUE, BG_BLACK); printf("No space in buffer - dropping data.\n");
                    #endif
                    #ifdef DO_WAIT
                usleep(USEC);
                    #endif
                    #ifdef DO_TIMEOUT
                totalWait += USEC;
                if (totalWait > WAIT_CAP) {
                    produced = 0;
                    roundNo = 0;
                    textcolour(0, BLUE, BG_BLACK); printf("Producer B:\t\t%u\tWaiting timed-out - exiting.\n", pid);
                }
                    #endif
            } 
        }        
    }

    textcolour(0, BLUE, BG_BLACK); printf("Producer B:\t\t%u\tFinishing:\n", pid);
        #ifdef MP_V_VERBOSE
    textcolour(0, BLUE, BG_BLACK); printf("Producer B:\t\t%u\t", pid);
    textcolour(0, BLUE, BG_BLACK); printFifoMed(medBuffer);
        #endif
    shmdt(medBuffer);

    return 0;
}
