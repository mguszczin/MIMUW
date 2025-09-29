CC = gcc
CFLAGS = -Wall -g -no-pie -fno-pie -mcmodel=large
TARGET = sig_handler
SRC = make_sig_handler.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o