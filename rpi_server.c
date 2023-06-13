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
#include "socket.h"
#include "header.h"

// Kernel primitives
pthread_attr_t attr;
pthread_t ping_thread;
pthread_t lcd_thread;
pthread_mutex_t lcd_mutex;
pthread_barrier_t init;


// Variables
int conn_fd, msg_cnt;
int sock_fd;
int server_fd;
int ping_flag = 1; // to close ping
struct LCD lcd;




void sigint_handler(int signum)
{
    ping_flag = 0;
    void *status;
    pthread_attr_destroy(&attr);
    if (pthread_join(ping_thread, &status))
    {
        exit(-1);
    }
    if (pthread_join(lcd_thread, &status))
    {
        exit(-1);
    }
    fprintf(stdout, "Terminating client\n");
    close(conn_fd);
    close(sock_fd);
    pthread_mutex_destroy(&lcd_mutex);
    pthread_barrier_destroy(&init);

    exit(0);
}

void *ping_func(void *ip) // deliver ip(char*) with this term
{
    FILE *p;
    float time = 0;
    char str[70], time_str[10];

    memset(str, 0, sizeof(str));
    memset(time_str, 0, sizeof(time_str));
    
    sprintf(str, "ping %s\n", (char *)ip);
    fprintf(stdout, "%s", str);
    p = popen(str, "r");
    if (!p)
    {
        fprintf(stderr, "Error opening pipe.\n");
        exit(-1);
    }
    
    pthread_barrier_wait(&init);

    while (ping_flag)
    {
        fgets(str, sizeof(str), p);
        for (int i = strlen(str); i > 0; i--)
        {
            if (str[i] == '=')
            {
                strncpy(time_str, str + i + 1, 5);
                time = atof(time_str);
                break;
            }
        }
        // fprintf(stdout, "ping speed %.3f ms\n", time);
        lcd.ping = time;
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ping : %f!\n", time);
        put_bar();
        pthread_mutex_unlock(&lcd_mutex);

        sleep(1);
    }
    /* speed section */
    if (pclose(p) == -1)
    {
        fprintf(stderr, " Error!\n");
        exit(1);
    }
    return NULL;
}

void* LCD_func(void* str) //you can use str to deliver the string into this thread
{
	/* init section */
    IO_initialization();
	pthread_barrier_wait(&init);
	/* display exec section */
    return NULL;
}

int main(int argc, char **argv)
{

    pthread_mutex_t lcd_mutex;
    pthread_mutex_init(&lcd_mutex, NULL);
    pthread_barrier_init(&init, NULL, 3);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    char ip[20];
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
	memset(ip,0,20);

    sprintf(ip,"%s",argv[1]);
    pthread_create(&ping_thread, &attr, ping_func, (void*)&ip);
	pthread_create(&lcd_thread,&attr,LCD_func,(void*)&ip);

    fprintf(stdout, "wait for initialization");
    
    // Enable sending to server:1111
    lcd.server_fd = createClientSock(argv[1], atoi(argv[2]), TRANSPORT_TYPE_TCP);

    // Enable recieving on port:1112
    sock_fd = createServerSock(1112, TRANSPORT_TYPE_TCP);
    if (sock_fd < 0)
    {
        perror("Error create socket\n");
        exit(-1);
    }

    pthread_barrier_wait(&init);

    
    while (1)
    {
        conn_fd = accept(sock_fd, (struct sockaddr *)&cln_addr, &sLen);
        if (conn_fd == -1)
        {
            fprintf(stdout, "Error: accept()");
            continue;
        }

        if (pthread_create(&thread, NULL, (void *(*)(void *)) & connectCallback, (void *)(intptr_t)(conn_fd)))
            fprintf(stdout, "Error: pthread_create()\n");
    }
    return 0;
}

