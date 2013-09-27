/*
 * Motor_driver.c
 *
 * Created: 9/22/2013 6:37:11 PM
 *  Author: Cornelius Fudge
 */ 

# define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define led1_on()  PORTC |= _BV(4)
#define led1_off()  PORTC &= ~_BV(4)
#define led2_on()  PORTC |= _BV(5)
#define led2_off()  PORTC &= ~_BV(5)

int main(void)
{
	
	DDRC  = 0b00110000;   // Turn PC4 and PC5 to output (LED1 and LED2)
	TIMSK1 = _BV(OCIE1A) | _BV(OCIE1B); // Enable Interrupt Timer/Counter1, Output Compare A & B (SIG_OUTPUT_COMPARE1A/SIG_OUTPUT_COMPARE1B)
	TCCR1B = _BV(CS12) | _BV(CS10) | _BV(WGM12);    // Clock/1024, 0.001024 seconds per tick, Mode=CTC
    OCR1A = 1500;                       // 0.001024*1954 ~= 2 therefore SIG_OUTPUT_COMPARE1A will be triggered every 2 seconds
    OCR1B = 977;                       // 0.001024*977 = 1.0004480 therefore SIG_OUTPUT_COMPARE1B will be triggered every second
	sei();
	SPI_MasterInit();
	char data = 7;
    while(1)
    {
		SPI_MasterTransmit(data);
    }
}

ISR(TIMER1_COMPA_vect) //interrupt service routine for timer1 compare A flag
{
	led1_on();
	led2_off();
}

ISR(TIMER1_COMPB_vect) //interrupt service routine for timer1 compare B flag
{
	led1_off();
	led2_on();
}

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB  = 0b00101000; 
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}
void SPI_MasterTransmit(char cData)
{
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}
