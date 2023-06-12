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
#include <stdint.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include "lcd.h"
#include "msg_queue.h"

#define BUFSIZE 16
#define TRANSPORT_TYPE_TCP 0
#define TRANSPORT_TYPE_UDP 1


int conn_fd, msg_cnt;
int sock_fd;
int server_fd;

 
extern pthread_mutex_t lcd_mutex;

void sigint_handler(int signum) {
    fprintf(stdout, "Terminating client\n");
    close(conn_fd);
    close(sock_fd);
    pthread_mutex_destroy(&lcd_mutex);
    exit(0);
}

int createClientSock(const char *host, int port, int type);
int createServerSock(int port, int type);
void connectCallback(int conn_fd);

int main(int argc, char **argv)
{

    pthread_mutex_t lcd_mutex;
    pthread_mutex_init(&lcd_mutex, NULL);
    pthread_t thread;

    struct sockaddr_in cln_addr;
    socklen_t sLen = sizeof(cln_addr);

    if (argc != 3)
    {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(-1);
    }

    if (wiringPiSetup() == -1)
    {
        fprintf(stderr, "setup wiringPi failed!\n");
        return -1;
    }
    
    signal(SIGINT, sigint_handler);

    IO_initialization();
    
    // Enable sending to server:1111
    lcd.server_fd = createClientSock(argv[1], atoi(argv[2]), TRANSPORT_TYPE_TCP);
    lcdClear(lcd.fd);
    lcdPosition(lcd.fd, 15, 1);      
    char temp[2];
    sprintf(temp, "%d", 0);
    lcdPuts(lcd.fd, temp);
    
    sleep(1);

    // Enable recieving on port:1112
    sock_fd = createServerSock(1112, TRANSPORT_TYPE_TCP);
    if (sock_fd < 0)
    {
        perror("Error create socket\n");
        exit(-1);
    }

    while (1)
    {   
        conn_fd = accept(sock_fd, (struct sockaddr *)&cln_addr, &sLen);
        if (conn_fd == -1)
        {
            fprintf(stdout, "Error: accept()");
            continue;
        }

        if (pthread_create(&thread, NULL, (void *(*)(void *)) & connectCallback, (void*)(intptr_t)(conn_fd)))
            fprintf(stdout, "Error: pthread_create()\n");

    }
    return 0;
}


int createClientSock(const char *host, int port, int type){
    int s;
    struct sockaddr_in sin;
    struct hostent *host_info;

    memset(&sin, 0, sizeof(sin)); // Init sin
    sin.sin_family = AF_INET;

    if((host_info = gethostbyname(host)))
        memcpy(&sin.sin_addr, host_info->h_addr_list[0], host_info->h_length);
    else
        sin.sin_addr.s_addr = inet_addr(host);

    sin.sin_port = htons((unsigned short)port);

    if (type == TRANSPORT_TYPE_TCP)
        s = socket(AF_INET, SOCK_STREAM, 0);
    else if (type == TRANSPORT_TYPE_UDP)
        s = socket(AF_INET, SOCK_DGRAM, 0);
    else
    {
        perror("Wrong transport type. Must be \"udp\" or \"tcp\"\n");
        return -1;
    }

    if (type == TRANSPORT_TYPE_TCP)
    {
        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        {
            char msg_buf[30];
            sprintf(msg_buf, "Can't connect to %s:%d\n", host, port);
            perror(msg_buf);
            return -1;
        }
    }
    return s;
}

int createServerSock(int port, int type){
    int s, yes = 1;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin)); // Init sin
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons((unsigned short)port);

    if (type == TRANSPORT_TYPE_TCP)
        s = socket(PF_INET, SOCK_STREAM, 0);
    else if (type == TRANSPORT_TYPE_UDP)
        s = socket(PF_INET, SOCK_DGRAM, 0);
    else
    {
        fprintf(stdout, "Wrong transport type. Must be \"udp\" or \"tcp\"\n");
        return -1;
    }

    if (s < 0)
    {
        fprintf(stdout, "Can't create socket\n");
        return -1;
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        fprintf(stdout, "Can't bind to port\n");
        return -1;
    }

    if (type == TRANSPORT_TYPE_TCP)
    {
        if (listen(s, 10) < 0)
        {
            fprintf(stdout, "Can'n listen on port\n");
            return -1;
        }
    }

    return s;
}

void connectCallback(int conn_fd){
    char rcv[BUFSIZE];
    int n;

    while (1)
    {
        if((n = read(conn_fd, rcv, BUFSIZE)) != 0){
            lcd.msg_len++;
            fprintf(stdout, "%d receive : %s\n", lcd.msg_len, rcv);
            addQueue(&lcd.msg_queue, rcv);
            pthread_mutex_lock(&lcd_mutex);
            if(lcd.msg_len < 10){
                lcdPosition(lcd.fd, 15, 1);
            }else{
                lcdPosition(lcd.fd, 14, 1);
            }            
            char temp[2];
            sprintf(temp, "%d", lcd.msg_len);
            lcdPuts(lcd.fd, temp);
            pthread_mutex_unlock(&lcd_mutex);
        }
    }

    close(conn_fd);
    pthread_exit(NULL);
    return;
}