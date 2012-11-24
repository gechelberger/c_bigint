
CC = gcc
CFLAGS = -fPIC -shared -w

LIBFLAGS = -fPIC -shared

library: bigmath.o
	$(CC) -o libbigmath.so bigmath.o $(LIBFLAGS)

