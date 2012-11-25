
CC = gcc
CFLAGS = -Wall

TESTFLAGS = -L./ -Wl,-rpath=./
LIBFLAGS = -fPIC -shared -lm

test: library test.c
	$(CC) -c test.c -lbigmath
	$(CC) -o tests test.o -L./ -lbigmath -Wl,-rpath=./

library: bigmath.c
	$(CC) -c bigmath.c -I -shared -fpic -lm
	$(CC) -o libbigmath.so bigmath.o -lm -shared

clean:
	rm *.o
	rm *.so

