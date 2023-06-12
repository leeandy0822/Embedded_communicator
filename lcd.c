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
#include "lcd.h"

int lcd_send_fd;
int lcd;
char code_content[16] = {};
char msg_content[16] = {};
int code_index = 0; 
int message_index = 0; 
pthread_mutex_t lcd_mutex;

unsigned long short_lastDebounceTime = 0;
unsigned long long_lastDebounceTime = 0;
unsigned long enter_lastDebounceTime = 0;
unsigned long send_lastDebounceTime = 0;

unsigned long debounceDelay = 100;

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

void setup_server(int fd){
    lcd_send_fd = fd;
}

void button_short_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - short_lastDebounceTime >= debounceDelay)
    {
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ISR short trigger\n");
        strcat(code_content, ".");
        lcdPosition(lcd, code_index , 1);
        lcdPuts(lcd, ".");
        pthread_mutex_unlock(&lcd_mutex);
        fprintf(stdout, "%s ", ".");
        code_index++;
        pthread_mutex_unlock(&lcd_mutex);

    }
    short_lastDebounceTime = currentTime;
}

void button_long_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - long_lastDebounceTime >= debounceDelay)
    {
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ISR long trigger\n");
        strcat(code_content, "-");
        lcdPosition(lcd, code_index, 1);
        lcdPuts(lcd, "-");
        fprintf(stdout, "%s ", "-");
        code_index++;
        pthread_mutex_unlock(&lcd_mutex);

    }
    long_lastDebounceTime = currentTime;
}

void button_enter_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - enter_lastDebounceTime >= debounceDelay)
    {
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ISR enter trigger ");
        fprintf(stdout, " code message: %s\n", code_content);
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
        strcpy(code_content, "");
        code_index = 0;
        pthread_mutex_unlock(&lcd_mutex);

    }
    enter_lastDebounceTime = currentTime;
}

void button_send_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - send_lastDebounceTime >= debounceDelay)
    {
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ISR send trigger\n");
        lcdClear(lcd);
        fprintf(stdout, "content %s\n", msg_content);
        printf("%d",lcd_send_fd);
        write(lcd_send_fd, msg_content, 16);
        strcpy(msg_content, "");
        code_index = 0;
        message_index = 0;
        pthread_mutex_unlock(&lcd_mutex);

    }
    send_lastDebounceTime = currentTime;
}

int IO_initialization()
{
    lcd = lcdInit(2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);

    pinMode(BUTTON_SHORT, INPUT);
    pinMode(BUTTON_LONG, INPUT);
    pinMode(BUTTON_ENTER, INPUT);
    pinMode(BUTTON_SEND, INPUT);

    if (wiringPiISR(BUTTON_SHORT, INT_EDGE_FALLING, (void*)&button_short_ISR) != 0)
    {
        fprintf(stderr, "Registering short ISR failed\n");
        return -1;
    }

    if (wiringPiISR(BUTTON_LONG, INT_EDGE_FALLING, (void*)&button_long_ISR) != 0)
    {
        fprintf(stderr, "Registering long ISR failed\n");
        return -1;
    }

    if (wiringPiISR(BUTTON_ENTER, INT_EDGE_FALLING, (void*)&button_enter_ISR) != 0)
    {
        fprintf(stderr, "Registering enter ISR failed\n");
        return -1;
    }

    if (wiringPiISR(BUTTON_SEND, INT_EDGE_FALLING, (void*)&button_send_ISR) != 0)
    {
        fprintf(stderr, "Registering send ISR failed\n");
        return -1;
    }

    return lcd;
}



