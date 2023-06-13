#include "lcd.h"
#include "msg_queue.h"
#include "header.h"


char code_content[16] = {};
char msg_content[16] = {};
int code_index = 0; 
int message_index = 0; 

unsigned long short_lastDebounceTime = 0;
unsigned long long_lastDebounceTime = 0;
unsigned long enter_lastDebounceTime = 0;
unsigned long send_lastDebounceTime = 0;
unsigned long debounceDelay = 150;

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
    lcd.fd = fd;
}

void button_short_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - short_lastDebounceTime >= debounceDelay && !lcd.record_mode)
    {
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ISR short trigger\n");
        strcat(code_content, ".");
        lcdPosition(lcd.fd, code_index , 1);
        lcdPuts(lcd.fd, ".");
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
    if (currentTime - long_lastDebounceTime >= debounceDelay && !lcd.record_mode)
    {
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ISR long trigger\n");
        strcat(code_content, "-");
        lcdPosition(lcd.fd, code_index, 1);
        lcdPuts(lcd.fd, "-");
        fprintf(stdout, "%s ", "-");
        code_index++;
        pthread_mutex_unlock(&lcd_mutex);

    }
    long_lastDebounceTime = currentTime;
}

void button_enter_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - enter_lastDebounceTime >= debounceDelay && !lcd.record_mode)
    {
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ISR enter trigger ");
        fprintf(stdout, " code message: %s\n", code_content);
        if (code_index != 0){
            lcdClear(lcd.fd);
        }
        for(int i = 0; i < 36; i++){
            if(strcmp(symbolsAlphabet[i][0], code_content) == 0){
                strcat(msg_content, symbolsAlphabet[i][1]);
                break; 
            }
        }
        lcdPosition(lcd.fd, 0, 0);
        lcdPuts(lcd.fd, msg_content);

        put_bar();
        strcpy(code_content, "");
        code_index = 0;
        pthread_mutex_unlock(&lcd_mutex);

    }
    enter_lastDebounceTime = currentTime;
}

void button_send_ISR()
{
    unsigned long currentTime = millis();
    if (currentTime - send_lastDebounceTime >= debounceDelay && !lcd.record_mode)
    {
        pthread_mutex_lock(&lcd_mutex);
        fprintf(stdout, "ISR send trigger\n");
        lcdClear(lcd.fd);
        fprintf(stdout, "content %s\n", msg_content);
        write(lcd.server_fd, msg_content, 16);
        put_bar();
        strcpy(msg_content, "");
        code_index = 0;
        message_index = 0;
        pthread_mutex_unlock(&lcd_mutex);

    }
    send_lastDebounceTime = currentTime;
}

void button_record_ISR(){
    unsigned long currentTime = millis();
    if (currentTime - send_lastDebounceTime >= 150){
        if(lcd.msg_len > 0){
            fprintf(stdout, "ISR record trigger\n");
            lcd.record_mode=1;

            // If the record button is triggered, we first clear the screen , add the content and add the msg_length
            lcdClear(lcd.fd);
            lcdPosition(lcd.fd, 0,0);
            lcdPuts(lcd.fd, "C:");
            Q_node* node_temp = deleteQueue(&lcd.msg_queue);
            lcd.msg_len--;
            lcdPuts(lcd.fd, node_temp->buffer);
            free(node_temp);
            put_bar();
            pthread_mutex_unlock(&lcd_mutex);
            fprintf(stdout, "Index: %d\n", lcd.msg_len);
        }else{
            lcd.record_mode=0;
            lcdClear(lcd.fd);
            put_bar();
        }
    }

    send_lastDebounceTime = currentTime;
}

void IO_initialization()
{
    lcd.fd = lcdInit(2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
    lcd.record_mode = 0;
    initQueue(&lcd.msg_queue);

    pinMode(BUTTON_SHORT, INPUT);
    pinMode(BUTTON_LONG, INPUT);
    pinMode(BUTTON_ENTER, INPUT);
    pinMode(BUTTON_SEND, INPUT);
    pinMode(BUTTON_RECORD, INPUT);

    if (wiringPiISR(BUTTON_SHORT, INT_EDGE_FALLING, (void*)&button_short_ISR) != 0)
    {
        fprintf(stderr, "Registering short ISR failed\n");
        return ;
    }

    if (wiringPiISR(BUTTON_LONG, INT_EDGE_FALLING, (void*)&button_long_ISR) != 0)
    {
        fprintf(stderr, "Registering long ISR failed\n");
        return ;
    }

    if (wiringPiISR(BUTTON_ENTER, INT_EDGE_FALLING, (void*)&button_enter_ISR) != 0)
    {
        fprintf(stderr, "Registering enter ISR failed\n");
        return ;
    }

    if (wiringPiISR(BUTTON_SEND, INT_EDGE_FALLING, (void*)&button_send_ISR) != 0)
    {
        fprintf(stderr, "Registering send ISR failed\n");
        return ;
    }

    if (wiringPiISR(BUTTON_RECORD, INT_EDGE_FALLING, (void*)&button_record_ISR) != 0)
    {
        fprintf(stderr, "Registering record ISR failed\n");
        return ;
    }
    pthread_mutex_lock(&lcd_mutex);
    lcdClear(lcd.fd);
    // Put the bar
    put_bar();
    pthread_mutex_unlock(&lcd_mutex);

    return;
}

void put_bar(void){
    lcdPosition(lcd.fd, 8, 1);
    lcdPuts(lcd.fd, "|");
    lcdPosition(lcd.fd, 13, 1);
    lcdPuts(lcd.fd, "|");
    if(lcd.msg_len < 10){
        lcdPosition(lcd.fd, 15, 1);
    }else{
        lcdPosition(lcd.fd, 14, 1);
    }
    char temp[2];
    sprintf(temp, "%d", lcd.msg_len);
    lcdPuts(lcd.fd, temp);
    lcdPosition(lcd.fd, 9, 1);
    char ping_temp[8];

    if(lcd.ping < 10){
        sprintf(ping_temp, "%.2f", lcd.ping);
    }else if(lcd.ping > 10 && lcd.ping < 100){
        sprintf(ping_temp, "%.1f", lcd.ping);
    }else if(lcd.ping > 100 && lcd.ping < 999){
        sprintf(ping_temp, "%d", (int)lcd.ping);
    }else{
        sprintf(ping_temp, "BAD!"); 
    }
    lcdPuts(lcd.fd, ping_temp);

}



