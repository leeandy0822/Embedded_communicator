CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-lwiringPi -lwiringPiDev -lpthread
SOURCES=test.c client.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=test client

all: $(EXECUTABLE)

test: test.o
	$(CC) $^ -o $@ $(LDFLAGS)

client: client.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)