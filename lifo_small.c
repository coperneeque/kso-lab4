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

#include "lifo_small.h"

#include "simple_test.h"
#include "test_flags.h"
#include "textcolour.h"


extern int errno;

void initLifoSmall(Lifo_small_t *l)
{
    errno = 0;

    if (l == NULL)
    {
        errno = EINVAL;
#ifdef MP_DEBUG        
        perror("[ error ] initLifoSmall(): null pointer");
#endif
        return;
    }

    l->capacity = LIFO_SMALL_CAPACITY;
    l->size     = 0;
    l->head_idx = 0;
    l->chunk    = LIFO_SMALL_CHUNK;
    sem_init(&l->mutex, 1, 1);
    sem_init(&l->semEmpty, 1, LIFO_SMALL_CAPACITY);
    sem_init(&l->semFull, 1, 0);
}

void putLifoSmall(Lifo_small_t *l, int k)
{
    errno = 0;

    if (l == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG        
        perror("[ error ] putLifoSmall(): NULL pointer");
            #endif
        return;
    }

    if (l->size == l->capacity)
    {
        errno = ENOBUFS;
            #ifdef MP_DEBUG        
        perror("[ error ] putLifoSmall(): Buffer is full");
            #endif
        return;
    }

    // size < capacity
    l->data[l->head_idx] = k;
    ++l->head_idx;
    ++l->size;
        #ifdef MP_DEBUG
    textcolour(0, WHITE, BG_BLACK);
    printf("putLifoSmall(): head_idx: %u, k: %d, size: %u\n", l->head_idx, k, l->size);
        #endif
}

int popLifoSmall(Lifo_small_t* l)
{
    errno = 0;

    if (l == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] popLifoSmall(): null pointer.");
            #endif
        return -1;
    }

    if (l->size == 0)
    {
        errno = ENODATA;
            #ifdef MP_DEBUG        
        perror("[ error ] popLifoSmall(): size is 0.");
            #endif
        return -1;
    }

    // size > 0
    --l->head_idx;
    int ret = l->data[l->head_idx];
    --l->size;
        #ifdef MP_DEBUG
    textcolour(0, WHITE, BG_BLACK);
    printf("popLifoSmall(): head_idx: %u, ret: %d, size: %u\n", l->head_idx, ret, l->size);
        #endif

    return ret;
}

void printLifoSmall(Lifo_small_t* l)
{
    errno = 0;

    if (l == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] printFifoBig(): null pointer");
            #endif
        return;
    }

    int sev, sfv;
    sem_getvalue(&l->semEmpty, &sev);
    sem_getvalue(&l->semFull, &sfv);
    // assume call is made synchronously
    sem_wait(&l->mutex);
    // textcolour(0, WHITE, BG_BLACK);
    printf("Small LIFO buffer. Capacity: %u, size: %u, head_idx: %u, semEmpty: %u, semFull: %u\n", l->capacity, l->size, l->head_idx, sev, sfv);
    sem_post(&l->mutex);
}

void flushLifoSmall(Lifo_small_t *l)
{
    errno = 0;

    if (l == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] flushLifoSmall(): null pointer");
            #endif
        return;
    }

    sem_wait(&l->mutex);
    l->size     = 0;
    l->head_idx = 0;
    for (size_t i = 0; i < l->size; i++)  // adjust semaphores
    {
        sem_wait(&l->semFull);
        sem_post(&l->semEmpty);
    }
    sem_post(&l->mutex);
}

void randFillLifoSmall(Lifo_small_t* l)
{
#define LOWER 30
#define UPPER 70
#define RANGE 100
        
    errno = 0;

    if (l == NULL)
    {
        errno = EINVAL;
            #ifdef MP_DEBUG
        perror("[ error ] randFillLifoSmall(): null pointer");
            #endif
        return;
    }

    flushLifoSmall(l);

    srandom(time(NULL));
    unsigned percentage = LOWER + random() % (UPPER - LOWER);
    float bound = (float)percentage / 100 * l->capacity;
        #ifdef MP_DEBUG
    textcolour(0, WHITE, BG_BLACK);
    printf("randFillLifoSmall(): Random filling %u elements\n", (int)bound);
    printLifoSmall(l);
        #endif
    sem_wait(&l->mutex);
    for (size_t i = 0; i < (size_t)bound; i++)  // semaphores have to track buffer size
    {
        sem_wait(&l->semEmpty);
        putLifoSmall(l, random() % RANGE);
        sem_post(&l->semFull);
    }
    sem_post(&l->mutex);
}
