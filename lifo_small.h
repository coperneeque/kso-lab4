/************************************************************************
 * Mikolaj Panka                                                        *
 * KSO 2021-z                                                           *
 * lab3                                                                 *
 ************************************************************************/
#ifndef LIFO_SMALL_H
#define LIFO_SMALL_H

#define LIFO_SMALL_CAPACITY 30
#define LIFO_SMALL_CHUNK 4

#include <semaphore.h>

typedef struct
{
    sem_t       mutex;
    sem_t       semFull;
    sem_t       semEmpty;
    int         data[LIFO_SMALL_CAPACITY];
    unsigned    head_idx;
    unsigned    capacity;
    unsigned    size;
    unsigned    chunk;  // Assume chunk is much smaller than capacity
} Lifo_small_t;

/*
 * Set initial values
 */
void initLifoSmall();

void putLifoSmall(Lifo_small_t*, int);
int popLifoSmall(Lifo_small_t*);
void printLifoSmall(Lifo_small_t*);

/*
 * Empty the Lifo by resetting head
 */
void flushLifoSmall(Lifo_small_t*);

/*
 * Populate a random precentage between 30% and 70%
 * with random values
 */
void randFillLifoSmall(Lifo_small_t*);

#endif
