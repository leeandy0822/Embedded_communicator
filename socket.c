#include "socket.h"
#include "header.h"

// Encryption

void encode(char* message) {
    int len = strlen(message);
    for (int i = 0; i < len; i++) {
        message[i] = message[i] + 5 + i;
    }
}

void decode(char* message) {
    int len = strlen(message);
    for (int i = 0; i < len; i++) {
        message[i] = message[i] - 5 - i;
    }
}


int createClientSock(const char *host, int port, int type)
{
    int s;
    struct sockaddr_in sin;
    struct hostent *host_info;

    memset(&sin, 0, sizeof(sin)); // Init sin
    sin.sin_family = AF_INET;

    if ((host_info = gethostbyname(host)))
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

int createServerSock(int port, int type)
{
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
        if ((n = read(conn_fd, rcv, BUFSIZE)) != 0)
        {
            decode(rcv);
            lcd.msg_len++;
            fprintf(stdout, "%d receive : %s\n", lcd.msg_len, rcv);
            addQueue(&lcd.msg_queue, rcv);
            pthread_mutex_lock(&lcd_mutex);
            if (lcd.msg_len < 10)
            {
                lcdPosition(lcd.fd, 15, 1);
            }
            else
            {
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