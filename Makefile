CC=gcc
LIBS=
CFLAGS=-g -O0

all: test


FAT_structures.o: FAT_structures.c FAT_structures.h
	$(CC) $(CFLAGS) -c -o FAT_structures.o FAT_structures.c


test.o: test.c FAT_structures.h
	$(CC) $(CFLAGS) -c -o test.o test.c

test: test.o FAT_structures.o
	$(CC) $(CFLAGS) -o test test.o FAT_structures.o $(LIBS)
	rm -f *.o

clean:
	rm -f *.o test