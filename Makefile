
CC = gcc
CFLAGS = -fPIC -shared -pedantic

LIBFLAGS = -fPIC -shared

library: bigmath.o
	$(CC) -o libbigmath.so bigmath.o $(LIBFLAGS)

