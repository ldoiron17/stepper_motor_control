/*
 * Motor_driver.c
 *
 * Created: 9/22/2013 6:37:11 PM
 *  Author: Cornelius Fudge
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define led1_on()  PORTC |= _BV(4)
#define led1_off()  PORTC &= ~_BV(4)
#define led2_on()  PORTC |= _BV(5)
#define led2_off()  PORTC |= _BV(5)

int main(void)
{
	
	DDRC  = 0b00110000;   // Turn PC4 and PC5 to output (LED1 and LED2)

    while(1)
    {
        led1_on()
		led2_on()
    }
}