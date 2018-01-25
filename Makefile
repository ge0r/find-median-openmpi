OPT=-O3
NPROCS = 4
N=32768

all: findMedianParallel.c AlternativefindMedianParallel.c serialFindMedian.c

	mpicc $(OP) findMedianParallel.c -o findMedian.out 
	mpicc $(OP) AlternativefindMedianParallel.c -o alternativeFindMedian.out
	gcc  $(OP) serialFindMedian.c -o serialFindMedian.out

run:	findMedian.out
	mpirun -np $(NPROCS) findMedian.out $(N)
	
clean:
	rm -f *.o *.out *~
