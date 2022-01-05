# Makefile
CFLAGS = -Wall -c
LDFLAGS = -lrt -lpthread

all: test_flags main.o fifo_big.o fifo_med.o lifo_small.o simple_test.o shared_mem.o consumerA.o consumerB.o consumerC.o consumerRand.o producerA.o producerB.o producerC.o producerRand.o textcolour.o
	gcc $(LDFLAGS) main.o fifo_big.o fifo_med.o lifo_small.o simple_test.o shared_mem.o textcolour.o -o main
	gcc $(LDFLAGS) consumerA.o fifo_big.o shared_mem.o textcolour.o -o consumerA
	gcc $(LDFLAGS) consumerB.o fifo_med.o shared_mem.o textcolour.o -o consumerB
	gcc $(LDFLAGS) consumerC.o lifo_small.o shared_mem.o textcolour.o -o consumerC
	gcc $(LDFLAGS) producerA.o fifo_big.o shared_mem.o textcolour.o -o producerA
	gcc $(LDFLAGS) producerB.o fifo_med.o shared_mem.o textcolour.o -o producerB
	gcc $(LDFLAGS) producerC.o lifo_small.o shared_mem.o textcolour.o -o producerC
	gcc $(LDFLAGS) producerRand.o fifo_big.o fifo_med.o lifo_small.o shared_mem.o textcolour.o -o producerRand
	gcc $(LDFLAGS) consumerRand.o fifo_big.o fifo_med.o lifo_small.o shared_mem.o textcolour.o -o consumerRand

main.o: main.c fifo_big.h fifo_med.h lifo_small.h simple_test.h test_flags.h textcolour.h
	gcc $(CFLAGS) main.c

fifo_big.o: fifo_big.h fifo_big.c simple_test.h test_flags.h textcolour.o
	gcc $(CFLAGS) fifo_big.c

fifo_med.o: fifo_med.h fifo_med.c simple_test.h test_flags.h
	gcc $(CFLAGS) fifo_med.c

lifo_small.o: lifo_small.h lifo_small.c simple_test.h test_flags.h
	gcc $(CFLAGS) lifo_small.c

shared_mem.o: shared_mem.h shared_mem.c test_flags.h
	gcc $(CFLAGS) shared_mem.c

consumerA.o: consumerA.c shared_mem.h fifo_big.h textcolour.h test_flags.h
	gcc $(CFLAGS) consumerA.c

consumerB.o: consumerB.c shared_mem.h fifo_med.h textcolour.h test_flags.h
	gcc $(CFLAGS) consumerB.c

consumerC.o: consumerC.c shared_mem.h lifo_small.h textcolour.h test_flags.h
	gcc $(CFLAGS) consumerC.c

consumerRand.o: consumerRand.c shared_mem.h fifo_big.h fifo_med.h lifo_small.h textcolour.h test_flags.h
	gcc $(CFLAGS) consumerRand.c

producerA.o: producerA.c shared_mem.h fifo_big.h textcolour.h test_flags.h
	gcc $(CFLAGS) producerA.c

producerB.o: producerB.c shared_mem.h fifo_med.h textcolour.h test_flags.h
	gcc $(CFLAGS) producerB.c

producerC.o: producerC.c shared_mem.h lifo_small.h textcolour.h test_flags.h
	gcc $(CFLAGS) producerC.c

producerRand.o: producerRand.c shared_mem.h fifo_big.h fifo_med.h lifo_small.h textcolour.h test_flags.h
	gcc $(CFLAGS) producerRand.c

textcolour.o: textcolour.h textcolour.c
	gcc $(CFLAGS) textcolour.c

simple_test.o: simple_test.h simple_test.c fifo_big.h fifo_med.h lifo_small.h
	gcc $(CFLAGS) simple_test.c

test_flags: test_flags.h

clean:
	rm -f *.o main consumer? producer? producerRand consumerRand

