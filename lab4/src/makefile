CC=gcc
CFLAGS=-I.

all: parallel_min_max

parallel_min_max : ../../lab3/src/utils.o ../../lab3/src/find_min_max.o ../../lab3/src/utils.h ../../lab3/src/find_min_max.h
	$(CC) -o parallel_min_max ../../lab3/src/utils.o ../../lab3/src/find_min_max.o ../../lab3/src/parallel_min_max.c $(CFLAGS)

utils.o : ../../lab3/src/utils.h
	$(CC) -o ../../lab3/src/utils.o -c ../../lab3/src/utils.c $(CFLAGS)

find_min_max.o : ../../lab3/src/utils.h ../../lab3/src/find_min_max.h
	$(CC) -o ../../lab3/src/find_min_max.o -c ../../lab3/src/find_min_max.c $(CFLAGS)

clean :
	rm ../../lab3/src/utils.o ../../lab3/src/find_min_max.o parallel_min_max 

