
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)


#include <xc.h>

#include <stdio.h>
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include <pic16f688.h>

#define _XTAL_FREQ 4000000UL

//lcd commands

#define RS  RA5
#define E   RA4

#define D0  NULL
#define D1  NULL
#define D2  NULL
#define D3  NULL

#define D4  RC0
#define D5  RC1
#define D6  RC2
#define D7  RC3

#define A   RC4
#define B   RC5

void __lcd_en_pulse(void)
{
    __delay_us(10);
    PORTAbits.E  = 1;
    __delay_us(500);
    PORTAbits.E  = 0;
}

void __lcd_port_write(uint8_t data)
{
    if(data & 0x01)
		D4 = 1;
	else
		D4 = 0;

	if(data & 0x02)
		D5 = 1;
	else
		D5 = 0;

	if(data & 0x04)
		D6 = 1;
	else
		D6 = 0;
    
	if(data & 0x08)
		D7 = 1;
	else
		D7 = 0;
}

void __lcd_cmd(uint8_t cmd)
{
    RS = 0;
    __lcd_port_write(cmd);
    __lcd_en_pulse();
    
}
void __lcd_send_char(uint8_t ascii)
{
    RS = 1;
    
    uint8_t high = 0;
    uint8_t low  = 0;
    high = ascii >> 4;
    low  = ascii & 0x0F;    
    
    __lcd_port_write(high);
    __lcd_en_pulse();
    __lcd_port_write(low);
    __lcd_en_pulse();
}

void LCD_send_string(uint8_t string[])
{
    uint8_t i = 0;
    for(i = 0;string[i]!='\0';i++)
    {
        __lcd_send_char(string[i]);
    }
}

void LCD_display_ctrl(uint8_t display, uint8_t cursor, uint8_t blink)
{
    __lcd_cmd(0x00);
    
    uint8_t temp = 0x0C;
    if(display == 1)
    {
        if(cursor == 1)
        {
            temp |= 1 << 1;
        }
        if(blink == 1)
        {
            temp |= 1 << 0;
        }
        __lcd_cmd(temp);
    }
    else
    {
        __lcd_cmd(0x08);
    }
}

void __lcd_shift_display(uint8_t LeftRight)
{
    __lcd_cmd(0x01);
    
    uint8_t temp = 0x08;
    if(LeftRight == 1)
    {
        temp = 0x0C;
        __lcd_cmd(temp);
    }
    else
    {
        __lcd_cmd(temp);
    }
}

void LCD_set_cursor(uint8_t row, uint8_t column)
{   
    if(row == 0)
    {
        __lcd_cmd(0x08);
    }
    else if(row == 1)
    {
        __lcd_cmd(0x0C);
    }
    uint8_t temp = column;
    __lcd_cmd(temp);
}

void LCD_display_clear(void)
{
    uint8_t i = 0;
    LCD_set_cursor(0,0);
    for(i = 0;i < 80;i++)
    {
        __lcd_send_char(0x20);
    }
}



void LCD_Init()
{
    __delay_ms(50);
    //set write command
    __lcd_cmd(0x03);
    __delay_ms(5);
    
    //function set, interface 8bit
    __lcd_cmd(0x03);
    __delay_us(120);
    
    //function set, interface 8bit
    __lcd_cmd(0x03);
    
    //function set, interface 8bit, setting to 4bit
    __lcd_cmd(0x02);
    
    // After this point commands are written in 2 bursts of 1 nibble
    //function set, interface 4bit 
    __lcd_cmd(0x02);
    __lcd_cmd(0x08);
    
    //display off 
    __lcd_cmd(0x00);
    __lcd_cmd(0x08);
    
    //display clear
    __lcd_cmd(0x00);
    __lcd_cmd(0x01);
    
    //entry mode
    __lcd_cmd(0x00);
    __lcd_cmd(0x06);
}

const uint8_t l_time[] = {"Time"};
const uint8_t l_date[] = {"Date"};
const uint8_t l_temp[] = {"Temp"};
const uint8_t l_humi[] = {"Humi"};

uint8_t time[]         = {"12:00PM"};
uint8_t date[]         = {"30/07/92"};
uint8_t current_temp[] = {"21.5"};
uint8_t current_humi[] = {"38%"};
uint8_t update_temp[]  = {"23.5"};
uint8_t update_humi[]  = {"45%"};

void Draw_main_display(void)
{
    LCD_send_string(l_temp);
    LCD_set_cursor(0,7);
    LCD_send_string(current_temp);
    __lcd_send_char(0x7E);
    LCD_send_string(update_temp);
    
    LCD_set_cursor(1,0);
    LCD_send_string(l_humi);
    LCD_set_cursor(1,9);
    LCD_send_string(current_humi);
    __lcd_send_char(0x7E);
    LCD_send_string(update_humi);
}



void main(void)
{
    ANSEL  = 0x00;
    CMCON0 = 0x07;
    
    TRISA  = 0x04;
    PORTA  = 0x00;
    
    TRISC  = 0x30;
    PORTC  = 0x00;
    
    T1CONbits.TMR1ON = 0;
    TMR1L = 0x00;
    TMR1H = 0x00;
    T1CON = 0x30;
    TMR1IF = 0;
    
    LCD_Init();

    LCD_display_ctrl(1,0,0);
    uint8_t string[] = {"hello"};
    while(1)
    {
        while(RA2);
        TMR1ON = 0;
        TMR1L = 0x00;
        TMR1H = 0x00;
        TMR1IF = 0;
        Draw_main_display();
        if(RA2)
        {
            T1CONbits.TMR1ON = 1;
            while(RA2);
            if(TMR1IF)
            {
                LCD_display_clear();
                LCD_send_string(string);
                while(!RA2);
                LCD_display_clear();
            }
            else
            {
                LCD_display_clear();
                LCD_set_cursor(0,0);
                LCD_send_string(l_time);
                LCD_set_cursor(0,10);
                LCD_send_string(time);
                LCD_set_cursor(1,0);
                LCD_send_string(l_date);
                LCD_set_cursor(1,8);
                LCD_send_string(date);
                while(!RA2);
                LCD_display_clear();
            }
        }
    }
}