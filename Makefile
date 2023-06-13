CC = gcc
CFLAGS = -c -Wall
LDFLAGS = -lwiringPi -lwiringPiDev -lpthread
SOURCES = rpi_server.c lcd.c msg_queue.c socket.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLES = rpi_server

all: $(EXECUTABLES)

rpi_server: rpi_server.o lcd.o msg_queue.o socket.o
	$(CC) $^ -o $@ $(LDFLAGS)

rpi_server.o: rpi_server.c lcd.h msg_queue.h socket.h
	$(CC) $(CFLAGS) $< -o $@

lcd.o: lcd.c lcd.h
	$(CC) $(CFLAGS) $< -o $@

msg_queue.o: msg_queue.c msg_queue.h
	$(CC) $(CFLAGS) $< -o $@

socket.o: socket.c socket.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLES)
