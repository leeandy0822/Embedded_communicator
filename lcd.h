#ifndef __LCD_H__
#define __LCD_H__


#define LCD_RS  25
#define LCD_E   24
#define LCD_D4  23
#define LCD_D5  22
#define LCD_D6  21
#define LCD_D7  14

#define BUTTON_SHORT     8
#define BUTTON_LONG      9
#define BUTTON_ENTER     30
#define BUTTON_SEND      16



void button_short_ISR(void);
void button_long_ISR(void);
void button_send_ISR(void);
void button_backspace_ISR(void);
int IO_initialization(void);

void setup_server(int fd);

#endif