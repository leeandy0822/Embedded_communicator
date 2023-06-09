#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define TRANSPORT_TYPE_TCP 0
#define TRANSPORT_TYPE_UDP 1

int createClientSock(const char *host, int port, int type);

int main(int argc, char **argv)
{
    int conn_fd, msg_cnt;
    char msg_buf[50];

    if (argc != 4)
    {
        printf("Usage: %s <host> <port> <message>\n", argv[0]);
        exit(-1);
    }
    msg_cnt = sprintf(msg_buf, "%s", argv[3]);

   
    conn_fd = createClientSock(argv[1], atoi(argv[2]), TRANSPORT_TYPE_TCP);
    write(conn_fd, msg_buf, msg_cnt);
    close(conn_fd);
    return 0;
} 

int createClientSock(const char *host, int port, int type)
{
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
