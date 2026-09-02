CC=gcc
LIBS=
CFLAGS=-g -O0

all: main


FAT_structures.o: FAT_structures.c FAT_structures.h
	$(CC) $(CFLAGS) -c -o FAT_structures.o FAT_structures.c


main.o: main.c FAT_structures.h
	$(CC) $(CFLAGS) -c -o main.o main.c

main: main.o FAT_structures.o
	$(CC) $(CFLAGS) -o main main.o FAT_structures.o $(LIBS)
	rm -f *.o

clean:
	rm -f *.o test