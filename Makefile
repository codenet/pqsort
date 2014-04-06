#CCFLAGS = -g -pg
CCFLAGS = 
all: generate run

generate: generate.c
	gcc -o generate generate.c

binary.o: binary.c
	gcc -c binary.c ${CCFLAGS}

chktime.o: chktime.c
	gcc -c chktime.c ${CCFLAGS}

pqueue.o: pqueue.h pqueue.c global.h
	gcc -c pqueue.c ${CCFLAGS}

logbarrier.o: logbarrier.c global.h
	gcc -c logbarrier.c ${CCFLAGS}

prefixsum.o: prefixsum.c global.h
	gcc -c prefixsum.c ${CCFLAGS}

aligned_malloc_free.o: aligned_malloc_free.c global.h
	gcc -c aligned_malloc_free.c ${CCFLAGS}

pqsort.o: pqsort.c global.h 
	gcc -c -lm -lpthread pqsort.c  ${CCFLAGS}

run: driver.c pqsort.o aligned_malloc_free.o logbarrier.o pqueue.o chktime.o binary.o prefixsum.o
	gcc -o run driver.c pqsort.o logbarrier.o aligned_malloc_free.o pqueue.o chktime.o binary.o prefixsum.o -lm -lpthread ${CCFLAGS}

clean:
	rm generate *.o run
