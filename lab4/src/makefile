CC=gcc
CFLAGS=-I.

all: parallel_min_max zombie process_memory

parallel_min_max : ../../lab3/src/utils.o ../../lab3/src/find_min_max.o ../../lab3/src/utils.h ../../lab3/src/find_min_max.h
	$(CC) -o parallel_min_max ../../lab3/src/utils.o ../../lab3/src/find_min_max.o ../../lab3/src/parallel_min_max.c $(CFLAGS)

psum : ../../lab3/src/utils.o sum.o
	$(CC) -pthread -o psum ../../lab3/src/utils.o sum.o parallel_sum.c $(CFLAGS)

utils.o : ../../lab3/src/utils.h
	$(CC) -o ../../lab3/src/utils.o -c ../../lab3/src/utils.c $(CFLAGS)

find_min_max.o : ../../lab3/src/utils.h ../../lab3/src/find_min_max.h
	$(CC) -o ../../lab3/src/find_min_max.o -c ../../lab3/src/find_min_max.c $(CFLAGS)

zombie : zombie.o
	$(CC) -o zombie zombie.o $(CFLAGS)
zombie.o : 
	$(CC) -o zombie.o -c zombie.c $(CFLAGS)

process_memory : process_memory.o
	$(CC) -Wno-format -o process_memory process_memory.o $(CFLAGS)
process_memory.o :
	$(CC) -Wno-format -o process_memory.o -c process_memory.c $(CFLAGS)

sum.o : sum.h
	$(CC) -o sum.o -c sum.c
clean : 
	rm ../../lab3/src/utils.o ../../lab3/src/find_min_max.o parallel_min_max zombie zombie.o process_memory.o process_memory psum sum.o

