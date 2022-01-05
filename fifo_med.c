/************************************************************************
 * Mikolaj Panka                                                        *
 * KSO 2021-z                                                           *
 * lab3                                                                 *
 ************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>

#include "fifo_med.h"

#include "simple_test.h"
#include "test_flags.h"
#include "textcolour.h"


extern int errno;

void initFifoMed(Fifo_med_t *f)
{
    errno = 0;

    if (f == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] initFifoMed(): null pointer");
            #endif
        return;
    }

    f->capacity = FIFO_MED_CAPACITY;
    f->size     = 0;
    f->head_idx = 0;  // next empty index - will wrap around when buffer is full
    f->tail_idx = -1;  // index of data to be extracted. can be -1 if buffer empty
    f->chunk    = FIFO_MED_CHUNK;
    sem_init(&f->mutex, 1, 1);
    sem_init(&f->semEmpty, 1, FIFO_MED_CAPACITY);
    sem_init(&f->semFull, 1, 0);
}

void putFifoMed(Fifo_med_t *f, int k)
{
    errno = 0;

    if (f == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG        
        perror("[ error ] putFifoMed(): NULL pointer");
            #endif
        return;
    }

    if (f->size == f->capacity)
    {
        errno = ENOBUFS;
            #ifdef MP_DEBUG        
        perror("[ error ] putFifoMed(): Buffer is full");
            #endif
        return;
    }

    if (f->size == 0)
    {
        f->data[f->head_idx] = k;
        ++(f->size);
        f->head_idx = (f->head_idx + 1 == f->capacity ? 0 : f->head_idx + 1);  // head_idx = head_idx+1 mod 60
            #ifdef MP_DEBUG
        textcolour(0, WHITE, BG_BLACK);
        printf("putFifoMed(): head_idx: %u, k: %d, size: %u\n", f->head_idx, k, f->size);        
            #endif
        return;
    }
        
    // size > 0    
    f->head_idx = (f->head_idx + 1 == f->capacity ? 0 : f->head_idx + 1);  // head_idx = head_idx+1 mod 60
    f->data[f->head_idx] = k;
    ++(f->size);
        #ifdef MP_DEBUG
    textcolour(0, WHITE, BG_BLACK);
    printf("putFifoMed(): head_idx: %u, k: %d, size: %u\n", f->head_idx, k, f->size);
        #endif
}

int popFifoMed(Fifo_med_t* f)
{
    errno = 0;

    if(f == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] popFifoMed(): null pointer.");
            #endif
        return -1;
    }

    if (f->size == 0)
    {
        errno = ENODATA;
            #ifdef MP_DEBUG        
        perror("[ error ] popFifoMed(): size is 0.");
            #endif
        return -1;
    }

    if (f->size == 1)
    {
        --f->size;
        int ret = f->data[f->tail_idx];
        f->tail_idx = (f->tail_idx + 1 == f->capacity ? 0 : f->tail_idx + 1);  // tail_idx = tail_idx+1 mod 60
            #ifdef MP_DEBUG
        textcolour(0, WHITE, BG_BLACK);
        printf("popFifoMed(): tail_idx: %u, ret: %d, size: %u\n", f->tail_idx, ret, f->size);
            #endif
        return ret;
    }
    
    f->tail_idx = (f->tail_idx + 1 == f->capacity ? 0 : f->tail_idx + 1);  // tail_idx = tail_idx+1 mod 60
    int ret = f->data[f->tail_idx];
    --f->size;
        #ifdef MP_DEBUG
    textcolour(0, WHITE, BG_BLACK);
    printf("popFifoMed(): tail_idx: %u, ret: %d, size: %u\n", f->tail_idx, ret, f->size);
        #endif

    return ret;
}

void printFifoMed(Fifo_med_t* f)
{
     errno = 0;

    if (f == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] printFifoMed(): null pointer");
            #endif
        return;
    }

    int sev, sfv;
    sem_getvalue(&f->semEmpty, &sev);
    sem_getvalue(&f->semFull, &sfv);
    // sem_wait(&f->mutex);
    // textcolour(0, WHITE, BG_BLACK);
    printf("Medium FIFO buffer. Capacity: %u, size: %u, tail_idx: %d, head_idx: %u, semEmpty: %u, semFull: %u\n", f->capacity, f->size, f->tail_idx, f->head_idx, sev, sfv);
    // sem_post(&f->mutex);
}

void flushFifoMed(Fifo_med_t *f)
{
    errno = 0;

    if (f == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] flushFifoMed(): null pointer");
            #endif
        return;
    }

    sem_wait(&f->mutex);
    f->size     = 0;
    f->head_idx = 0;
    f->tail_idx = -1;
    for (size_t i = 0; i < f->size; i++)  // adjust semaphores
    {
        sem_wait(&f->semFull);
        sem_post(&f->semEmpty);
    }
    sem_post(&f->mutex);
}

void randFillFifoMed(Fifo_med_t* f)
{
#define LOWER 30
#define UPPER 70
#define RANGE 100
        
    errno = 0;

    if (f == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] randFillFifoMed(): null pointer");
            #endif
        return;
    }

    flushFifoMed(f);

    srandom(time(NULL));
    unsigned percentage = LOWER + random() % (UPPER - LOWER);
    float bound = (float)percentage / 100 * f->capacity;
        #ifdef MP_DEBUG
    textcolour(0, WHITE, BG_BLACK);
    printf("randFillFifoMed(): Random filling %u elements\n", (int)bound);
    printFifoMed(f);
        #endif
    sem_wait(&f->mutex);
    for (size_t i = 0; i < (size_t)bound; i++)
    {
        sem_wait(&f->semEmpty);
        putFifoMed(f, random() % RANGE);
        sem_post(&f->semFull);
    }
    sem_post(&f->mutex);
}
