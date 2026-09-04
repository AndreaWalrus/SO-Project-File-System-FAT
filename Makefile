CC=gcc
LIBS=
CFLAGS=-g -O0

all: main


FAT_structures.o: FAT_structures.c FAT_structures.h
	$(CC) $(CFLAGS) -c -o FAT_structures.o FAT_structures.c

test1.o: test1.c FAT_structures.h
	$(CC) $(CFLAGS) -c -o test1.o test1.c

main.o: main.c FAT_structures.h
	$(CC) $(CFLAGS) -c -o main.o main.c

main: main.o FAT_structures.o test1.o
	$(CC) $(CFLAGS) -o main main.o FAT_structures.o test1.o $(LIBS)
	rm -f *.o

clean:
	rm -f *.o main FileSystem.img