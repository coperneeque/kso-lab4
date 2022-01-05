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

// attach to existing (hopefully]) big buffer
    int bigBlockId = getMemBlock(SHMEM_FILE, 0, sizeof(Fifo_big_t));
    Fifo_big_t *bigBuffer = attachMemBlock(bigBlockId);
        #ifdef MP_V_VERBOSE
    textcolour(0, GREEN, BG_BLACK); printf("Producer A:\t\t%u\tAttached to shared big buffer:\n", pid);
    textcolour(0, GREEN, BG_BLACK); printf("Producer A:\t\t%u\t", pid);
    textcolour(0, GREEN, BG_BLACK); printFifoBig(bigBuffer);
        #endif
    sem_wait(&bigBuffer->mutex);
    roundNo = bigBuffer->capacity * ROUND_MULT;
    sem_post(&bigBuffer->mutex);

    while(roundNo) {
        --roundNo;
        produced = random() % PROD_CAP;  // how much is produced
        for (size_t i = 0; i < produced; i++) {  // simulate production
            products[i] = random() % RANGE;
        }
        
            #ifdef MP_VERBOSE
        textcolour(0, GREEN, BG_BLACK); printf("Producer A:\t\t%u\t\t\t\trun: %u, produced: %u units\n", pid, roundNo, produced);
            #endif
        while (produced > 0) {
                #ifdef MP_VERBOSE
            textcolour(0, GREEN, BG_BLACK); printf("Producer A:\t\t%u\t\t\t\trun: %u, to insert: %u units. ", pid, roundNo, produced);
                #endif
            sem_wait(&bigBuffer->mutex);  // access the buffer
            bufEmpty = bigBuffer->capacity - bigBuffer->size;
            if (bufEmpty > 0) {  // there is space in the buffer
                /*
                 * There is data produced and there is space in the buffer.
                 * Determine how much can be inserted.
                 */
                /*
                 * Check borderline conditions:
                 */
                if (bufEmpty < bigBuffer->chunk) {  // buffer is less than 1 chunk empty
                    if (produced <= bufEmpty) {  // all produced data will fit in the buffer
                        toInsert = produced;  // all produced data can be inserted
                        produced = 0;  // producer is satisfied
                    } else {  // only some of the produced data will fit in the buffer
                        toInsert = bufEmpty;  // can insert to fill up the buffer
                        produced -= toInsert;  // producer is not satisfied
                    }
                } else {  // buffer is at least 1 chunk empty
                    if (produced <= bigBuffer->chunk) {  // all produced data will fit in 1 chunk
                        toInsert = produced;  // all produced data can be inserted
                        produced = 0;  // producer is satisfied
                    } else {  // produced data will not fit in 1 chunk
                        toInsert = bigBuffer->chunk;  // some of the data can be inserted into 1 full chunk
                        produced -= toInsert;  // producer is not satisfied
                    }
                }
                /*
                 * Execute the insertion:
                 */
                    #ifdef MP_VERBOSE
                textcolour(UNDERLINE, GREEN, BG_BLACK); printf("Inserting %u units\n", toInsert);
                    #endif
                for (size_t i = 0; i < toInsert; i++) {
                    sem_wait(&bigBuffer->semEmpty);
                    putFifoBig(bigBuffer, products[i]);
                    sem_post(&bigBuffer->semFull);
                }
                /*
                 * Open the mutex after inserting into the buffer.
                 * Possibly another consumer/producer will consume/produce data
                 */
                sem_post(&bigBuffer->mutex);
            } else {  // no space in the buffer
                sem_post(&bigBuffer->mutex);
                /*
                 * If there is no space in the buffer then the producer can't wait.
                 * In normal conditions running out of buffer space results in data loss.
                 * This is simulated here by zeroing of the 'produced' variable.
                 */
                produced = 0;
                    #ifdef MP_VERBOSE
                textcolour(UNDERLINE, GREEN, BG_BLACK); printf("No space in buffer - dropping data.\n");
                    #endif
                    #ifdef DO_WAIT
                usleep(USEC);
                    #endif
                    #ifdef DO_TIMEOUT
                totalWait += USEC;
                if (totalWait > WAIT_CAP) {
                    produced = 0;
                    roundNo = 0;
                    textcolour(0, GREEN, BG_BLACK); printf("Producer A:\t\t%u\tWaiting timed-out - exiting.\n", pid);
                }
                    #endif
            } 
        }        
    }

    textcolour(0, GREEN, BG_BLACK); printf("Producer A:\t\t%u\tFinishing:\n", pid);
        #ifdef MP_V_VERBOSE
    textcolour(0, GREEN, BG_BLACK); printf("Producer A:\t\t%u\t", pid);
    textcolour(0, GREEN, BG_BLACK); printFifoBig(bigBuffer);
        #endif
    shmdt(bigBuffer);

    return 0;
}
