CC = gcc
CFLAGS = -c -Wall
LDFLAGS = -lwiringPi -lwiringPiDev -lpthread
SOURCES = rpi_server.c lcd.c msg_queue.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLES = rpi_server 

all: $(EXECUTABLES)

rpi_server: rpi_server.o lcd.o msg_queue.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c lcd.h
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c msg_queue.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLES)
