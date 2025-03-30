CC = gcc
CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
LFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

SRC = ma.c memory_test.c
OBJ = ma.o memory_test.o
LIB = libma.so
EXAMPLE = ma_example.c 

.PHONY: all clean

all: $(LIB)

$(LIB) : $(OBJ) 
	$(CC) $(OBJ) -o $(LIBARY) $(LFLAGS)

memory_test.o: memory_test.c ma.h
	$(CC) $(CFLAGS) -c memory_test.c

ma.o: ma.c ma.h
	$(CC) $(CFLAGS) -c ma.c

ma_example: ma_example.c $(LIBRARY)
	$(CC) $(CFLAGS) ma_example.c -o ma_example -L. -lma

clean:
	rm -f *.o $(LIBRARY) ma_example
