#ifndef __SOCKET_H__
#define __SOCKET_H__


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
#include "lcd.h"

#define BUFSIZE 16
#define TRANSPORT_TYPE_TCP 0
#define TRANSPORT_TYPE_UDP 1



int createClientSock(const char *host, int port, int type);
int createServerSock(int port, int type);
void connectCallback(int conn_fd);


#endif