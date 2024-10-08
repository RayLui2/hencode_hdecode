CFLAGS = -Wall -ansi -g

CC = gcc

all: htable hencode hdecode

hdecode: hdecode.o hufftools.o
	$(CC) $(CFLAGS) -o hdecode hdecode.o hufftools.o

hdecode.o: hdecode.c hufftools.c
	$(CC) $(CFLAGS) -c hdecode.c hufftools.c

hencode: hencode.o hufftools.o
	$(CC) $(CFLAGS) -o hencode hencode.o hufftools.o

hencode.o: hencode.c hufftools.c
	$(CC) $(CFLAGS) -c hencode.c hufftools.c

htable: htable.o hufftools.o
	$(CC) $(CFLAGS) -o htable htable.o hufftools.o

htable.o: htable.c hufftools.c
	$(CC) $(CFLAGS) -c htable.c hufftools.c

clean:
	rm *.o htable hencode hdecode

