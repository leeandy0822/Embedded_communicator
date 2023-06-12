#ifndef __LCD_H__
#define __LCD_H__

#include <wiringPi.h>
#include <lcd.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

#include "msg_queue.h"

#define LCD_RS           25
#define LCD_E            24
#define LCD_D4           23
#define LCD_D5           22
#define LCD_D6           21
#define LCD_D7           14
#define BUTTON_SHORT     8
#define BUTTON_LONG      9
#define BUTTON_ENTER     30
#define BUTTON_SEND      16
#define BUTTON_RECORD    31

struct LCD{
    int fd;
    int server_fd;
    int msg_len;
	int record_mode;
	Queue msg_queue;
};

extern struct LCD lcd;

void button_short_ISR(void);
void button_long_ISR(void);
void button_enter_ISR(void);
void button_send_ISR(void);
void button_record_ISR(void);

void IO_initialization(void);

void setup_server(int fd);

#endif