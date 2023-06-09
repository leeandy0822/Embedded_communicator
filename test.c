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

#define TRANSPORT_TYPE_TCP 0
#define TRANSPORT_TYPE_UDP 1

#define LCD_RS  25
#define LCD_E   24
#define LCD_D4  23
#define LCD_D5  22
#define LCD_D6  21
#define LCD_D7  14

#define BUTTON_SHORT     8
#define BUTTON_LONG      9
#define BUTTON_SEND      30
#define BUTTON_BACKSPACE 16

int lcd;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;
char code_content[16] = {};
char msg_content[16] = {};
int code_index = 0; 
int message_index = 0 ; 
int conn_fd, msg_cnt;
char msg_buf[50];
char* symbolsAlphabet[][2] =
{
	{ ".-","A" },
	{ "-...","B" },
	{ "-.-.","C" },
	{ "-..","D" },
	{ ".","E" },
	{ "..-.","F" },
	{ "--.","G" },
	{ "....","H" },
	{ "..","I" },
	{ ".---","J" },
	{ "-.-","K" },
	{ ".-..","L" },
	{ "--","M" },
	{ "-.","N" },
	{ "---","O" },
	{ ".--.","P" },
	{ "--.-","Q" },
	{ ".-.","R" },
	{ "...","S" },
	{ "-","T" },
	{ "..-","U" },
	{ "...-","V" },
	{ ".--","W" },
	{ "-..-","X" },
	{ "-.--","Y" },
	{ "--..","Z" },
	{ ".----","1" },
	{ "..---","2" },
	{ "...--","3" },
	{ "....-","4" },
	{ ".....","5" },
	{ "-....","6" },
	{ "--...","7" },
	{ "---..","8" },
	{ "----.","9" },
	{ "-----","0"}
};

int createClientSock(const char *host, int port, int type);
void IO_initialization();

void button_short_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime >= debounceDelay)
    {
        fprintf(stdout, "ISR short trigger\n");
        strcat(code_content, ".");
        lcdPosition(lcd, code_index , 1);
        lcdPuts(lcd, ".");
        fprintf(stdout, "%s\n", ".");
        code_index++;
    }
    lastDebounceTime = currentTime;
}

void button_long_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime >= debounceDelay)
    {
        fprintf(stdout, "ISR long trigger\n");
        strcat(code_content, "-");
        lcdPosition(lcd, code_index, 1);
        lcdPuts(lcd, "-");
        fprintf(stdout, "%s\n", "-");
        code_index++;
    }
    lastDebounceTime = currentTime;
}

void button_send_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime >= debounceDelay)
    {
        fprintf(stdout, "ISR send trigger\n");
        fprintf(stdout, "code message: %s\n", code_content);

        if (code_index != 0){
            lcdClear(lcd);
        }
        for(int i = 0; i <36; i++){
            if(strcmp(symbolsAlphabet[i][0], code_content) == 0){
                strcat(msg_content, symbolsAlphabet[i][1]);
                break; 
            }
        }
        lcdPosition(lcd, 0, 0);
        lcdPuts(lcd, msg_content);
        fprintf(stdout, "%s \n", msg_content);
        strcpy(code_content, "");
        code_index = 0;
    }
    lastDebounceTime = currentTime;
}

void button_backspace_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime >= debounceDelay)
    {
        fprintf(stdout, "ISR backspace trigger\n");
        lcdClear(lcd);
        fprintf(stdout, "content %s\n", msg_content);

        write(conn_fd, msg_content, 16);
        strcpy(msg_content, "");
        fprintf(stdout, "content %s\n", msg_content);

        code_index = 0;
        message_index = 0;
    }
    lastDebounceTime = currentTime;
}

void sigint_handler(int signum) {
    fprintf(stdout, "Terminating client\n");
    close(conn_fd);
    exit(0);
}


int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(-1);
    }

    if (wiringPiSetup() == -1)
    {
        fprintf(stderr, "setup wiringPi failed!\n");
        return 1;
    }
    signal(SIGINT, sigint_handler);

    IO_initialization();
    conn_fd = createClientSock(argv[1], atoi(argv[2]), TRANSPORT_TYPE_TCP);
    lcdClear(lcd);
    sleep(1);

    while (1)
    {   

    }
    return 0;
}

void IO_initialization()
{
    lcd = lcdInit(2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);

    pinMode(BUTTON_SHORT, INPUT);
    pinMode(BUTTON_LONG, INPUT);
    pinMode(BUTTON_SEND, INPUT);
    pinMode(BUTTON_BACKSPACE, INPUT);

    if (wiringPiISR(BUTTON_SHORT, INT_EDGE_FALLING, &button_short_ISR) != 0)
    {
        fprintf(stderr, "Registering short ISR failed\n");
        return;
    }

    if (wiringPiISR(BUTTON_LONG, INT_EDGE_FALLING, &button_long_ISR) != 0)
    {
        fprintf(stderr, "Registering long ISR failed\n");
        return;
    }

    if (wiringPiISR(BUTTON_SEND, INT_EDGE_FALLING, &button_send_ISR) != 0)
    {
        fprintf(stderr, "Registering send ISR failed\n");
        return;
    }

    if (wiringPiISR(BUTTON_BACKSPACE, INT_EDGE_FALLING, &button_backspace_ISR) != 0)
    {
        fprintf(stderr, "Registering backspace ISR failed\n");
        return;
    }
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
