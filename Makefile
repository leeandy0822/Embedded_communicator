CC = gcc
CFLAGS = -c -Wall
LDFLAGS = -lwiringPi -lwiringPiDev -lpthread
SOURCES = test.c client.c lcd.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLES = test client

all: $(EXECUTABLES)

test: test.o lcd.o
	$(CC) $^ -o $@ $(LDFLAGS)

client: client.o lcd.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c lcd.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLES)
