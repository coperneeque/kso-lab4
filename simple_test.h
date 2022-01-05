/************************************************************************
 * Mikolaj Panka                                                        *
 * KSO 2021-z                                                           *
 * lab3                                                                 *
 ************************************************************************/
#ifndef SIMPLE_TEST_H
#define SIMPLE_TEST_H

#include "fifo_big.h"
#include "fifo_med.h"
#include "lifo_small.h"

void test_FifoBig(Fifo_big_t* fifo);
void test_FifoMed(Fifo_med_t* fifo);
void test_LifoSmall(Lifo_small_t *lifo);

#endif
