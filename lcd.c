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

int lcd_sender_fd;
int lcd;
char code_content[16] = {};
char msg_content[16] = {};
int code_index = 0; 
int message_index = 0; 

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;

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
    lcd_sender_fd = fd;
}

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
        printf("%d",lcd_sender_fd);
        write(lcd_sender_fd, msg_content, 16);
        strcpy(msg_content, "");
        code_index = 0;
        message_index = 0;
    }
    lastDebounceTime = currentTime;
}

int IO_initialization()
{
    lcd = lcdInit(2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);

    pinMode(BUTTON_SHORT, INPUT);
    pinMode(BUTTON_LONG, INPUT);
    pinMode(BUTTON_SEND, INPUT);
    pinMode(BUTTON_BACKSPACE, INPUT);

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

    if (wiringPiISR(BUTTON_SEND, INT_EDGE_FALLING, (void*)&button_send_ISR) != 0)
    {
        fprintf(stderr, "Registering send ISR failed\n");
        return -1;
    }

    if (wiringPiISR(BUTTON_BACKSPACE, INT_EDGE_FALLING, (void*)&button_backspace_ISR) != 0)
    {
        fprintf(stderr, "Registering backspace ISR failed\n");
        return -1;
    }

    return lcd;
}



