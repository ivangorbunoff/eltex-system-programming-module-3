CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

TARGET = file_copy
OBJECTS = main.o channel.o transfer.o ui.o

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c channel.h transfer.h ui.h
	$(CC) $(CFLAGS) -c main.c

channel.o: channel.c channel.h
	$(CC) $(CFLAGS) -c channel.c

transfer.o: transfer.c transfer.h ui.h
	$(CC) $(CFLAGS) -c transfer.c

ui.o: ui.c ui.h
	$(CC) $(CFLAGS) -c ui.c

test: $(TARGET)
	./test.sh

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf demo
