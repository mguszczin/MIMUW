CC = gcc
CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2 -g
LFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup
TESTFLAGS = -g -O0 -Wall -Wextra



SRC = ma.c memory_tests.c
OBJ = ma.o memory_tests.o
LIBRARY = libma.so
EXAMPLE_SRC = ma_example.c
EXAMPLE = ma_example

.PHONY: all clean test

all: $(LIBRARY) $(EXAMPLE)

$(LIBRARY): $(OBJ)
	$(CC) $(OBJ) -o $(LIBRARY) $(LFLAGS)

ma.o: ma.c ma.h
	$(CC) $(CFLAGS) -c ma.c

memory_tests.o: memory_tests.c ma.h
	$(CC) $(CFLAGS) -c memory_tests.c

$(EXAMPLE): $(EXAMPLE_SRC) $(LIBRARY)
	$(CC) $(TESTFLAGS) $(EXAMPLE_SRC) -o $(EXAMPLE) -L. -lma -Wl,-rpath,.

test: $(EXAMPLE)
	LD_LIBRARY_PATH=. ./$(EXAMPLE) one

clean:
	rm -f *.o $(LIBRARY) $(EXAMPLE)
