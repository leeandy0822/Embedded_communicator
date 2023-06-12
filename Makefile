CC = gcc
CFLAGS = -c -Wall
LDFLAGS = -lwiringPi -lwiringPiDev -lpthread
SOURCES = test.c client.c lcd.c msg_queue.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLES = test client

all: $(EXECUTABLES)

test: test.o lcd.o msg_queue.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c lcd.h
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c msg_queue.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLES)
