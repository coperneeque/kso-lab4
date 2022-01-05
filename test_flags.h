/************************************************************************
 * Mikolaj Panka                                                        *
 * KSO 2021-z                                                           *
 * lab4                                                                 *
 ************************************************************************/

// The following flags can be set:

// Consumer/producer rounds per buffer capacity multiplier
// eg. if buffer capacity is 30 then multiplier of 5
// yields 5*30 rounds of consumption/production
#define ROUND_MULT 1

// Producer/Consumer wait time after buffer is full/empty:
// #define DO_WAIT
#define USEC        1000  // microseconds waiting time
#define DO_TIMEOUT
#define WAIT_CAP    10000  // microseconds total waiting time

// to be verbose
#define MP_VERBOSE

// to be very verbose
// #define MP_V_VERBOSE

// to print more debug info:
// #define MP_DEBUG

// to prefill buffers before consumers/producers are spawned:
// #define DO_PREFILL

// to run test on the bigest buffer FIFO:
// #define TEST_FIFO_BIG

// to run test on the medium buffer FIFO:
// #define TEST_FIFO_MED

// to run test on the smallest buffer LIFO (stack):
// #define TEST_LIFO_SMALL
