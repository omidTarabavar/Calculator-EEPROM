/*
 * calculator.c
 *
 * Created: 1/27/2025 3:31:19 AM
 * Author: omidt
 */

#include <mega32.h>
#include <alcd.h>
#include <delay.h>
#include <stdio.h>
#include <eeprom.h>

unsigned char scan[4] = {0xFE, 0xFD, 0xFB, 0xF7};
char Buf[16];
char pressedKey = 0, operand = '';
int num1 = 0, num2 = 0, result = 0, state = 0;
int shown = 0;
int showHistoryState = 0;

char arrkey[16] = {
    '7', '8', '9', '/',
    '4', '5', '6', '*',
    '1', '2', '3', '-',
    'C', '0', '=', '+'
};
#define c1 PINB.4
#define c2 PINB.5
#define c3 PINB.6
#define c4 PINB.7
#define keypad_port PORTB

void keypad()
{
    unsigned char r,c;
    delay_us(10);
    for (r=0; r<4; r++){
            c=255;
            keypad_port=scan[r];
            delay_us(10);
            if(c1==0) pressedKey=arrkey[(r*4)+0];
            if(c2==0) pressedKey=arrkey[(r*4)+1];
            if(c3==0) pressedKey=arrkey[(r*4)+2];
            if(c4==0) pressedKey=arrkey[(r*4)+3];   
    }
}

interrupt [2] void my_int()
{
    #asm("cli")
    keypad();
    DDRB=0X0F;
    keypad_port=0XF0;
    if(pressedKey != 0)
    {
        if (pressedKey == 'C') {num1 = 0; num2 = 0; result = 0; lcd_clear(); showHistoryState = 0; num1 = 0; num2 = 0; state = 0;}
        else if(pressedKey == '/'){
            int historyCount = eeprom_read_byte(0);
            if (historyCount != 0){            
                shown = 0;
                showHistoryState = 1;
                lcd_clear();
                lcd_puts("* = next page");
            }
            else{
                lcd_clear();
                lcd_puts("No history");
            }
        }
        else if(pressedKey == '*'){
            if(showHistoryState == 1){
                int historyCount = eeprom_read_byte(0);
                if(shown < historyCount){
                    int h_num1 = eeprom_read_byte(shown * 4 + 1);
                    char h_op = eeprom_read_byte(shown * 4 + 2);
                    int h_num2 = eeprom_read_byte(shown * 4 + 3);
                    int h_res = eeprom_read_byte(shown * 4 + 4);
                    shown += 1;
                    lcd_clear();
                    sprintf(Buf,"%d",h_num1); lcd_puts(Buf);
                    sprintf(Buf,"%c",h_op); lcd_puts(Buf);
                    sprintf(Buf,"%d",h_num2); lcd_puts(Buf);
                    sprintf(Buf,"%c",'='); lcd_puts(Buf);
                    sprintf(Buf,"%d",h_res); lcd_puts(Buf);
                }
                else { shown = 0; }
            }
        
        }
        else if((pressedKey == '+' | pressedKey == '-') & showHistoryState == 0) { operand = pressedKey; state = 1; lcd_putchar(pressedKey);}
        
        else if(pressedKey == '=' & showHistoryState == 0) {
            int historyCount = eeprom_read_byte(0);
            if(historyCount == 10){
                int j;
                for(j = 0; j < 9; j++){
                    int oh_num1 = eeprom_read_byte((j+1)*4+1);
                    int oh_opr1 = eeprom_read_byte((j+1)*4+2);
                    int oh_num2 = eeprom_read_byte((j+1)*4+3);
                    int oh_res1 = eeprom_read_byte((j+1)*4+4);
                    delay_ms(10);
                    eeprom_update_byte(j*4+1,oh_num1);
                    eeprom_update_byte(j*4+2,oh_opr1);
                    eeprom_update_byte(j*4+3,oh_num2);
                    eeprom_update_byte(j*4+4,oh_res1);
                    delay_ms(10);
                }
                eeprom_update_byte(0,9);
                historyCount -= 1;
            }
            state = 0;
            lcd_putchar(pressedKey);
            if(operand == '+') { result = num1 + num2; }
            else if(operand == '-') { result = num1 - num2; }
            sprintf(Buf,"%d",result); lcd_puts(Buf);

            eeprom_update_byte(historyCount * 4 + 1, num1);
            eeprom_update_byte(historyCount * 4 + 2, operand);
            eeprom_update_byte(historyCount * 4 + 3, num2);
            eeprom_update_byte(historyCount * 4 + 4, result);
            eeprom_update_byte(0, (historyCount + 1));
            
        }
        else if(showHistoryState == 1){
            lcd_clear();
            lcd_puts("* = next page");
        }
        else{
            if(state == 0){ num1 = num1 * 10 + pressedKey - 48; }
            else{ num2 = num2 * 10 + pressedKey - 48; }
            lcd_putchar(pressedKey);
        }
        pressedKey = 0;
    }
    
    #asm ("sei")
}

void main(void)
{
    //eeprom_write_byte(0, 0);
    lcd_init(16);
    lcd_clear();
    lcd_puts("Let's calculate!");
    delay_ms(1000);
    lcd_clear();
    
    #asm("sei")
    GICR |= (1 << INT0);
    MCUCR |= (1 << ISC01) | (0 << ISC00);
    DDRB=0X0F;
    keypad_port=0XF0;
    while (1) {
           
    }
}
