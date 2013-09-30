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
#define pull_STR_high()  PORTB |= _BV(7)
#define pull_STR_low() PORTB &= ~_BV(7)
#define pull_WC_high()  PORTB |= _BV(6)
#define pull_WC_low() PORTB &= ~_BV(6)

int main(void)
{
	//Set direction registers (1 -> output, 0 -> input)
	DDRC  = 0b00110000;   // Turn PC4 and PC5 to output (LED1 and LED2)
	TIMSK1 = _BV(OCIE1A) | _BV(OCIE1B); // Enable Interrupt Timer/Counter1, Output Compare A & B (SIG_OUTPUT_COMPARE1A/SIG_OUTPUT_COMPARE1B)
	TCCR1B = _BV(CS12) | _BV(CS10) | _BV(WGM12);    // Clock/1024, 0.001024 seconds per tick, Mode=CTC
    OCR1A = 1500;                       // 0.001024*1954 ~= 2 therefore SIG_OUTPUT_COMPARE1A will be triggered every 2 seconds
    OCR1B = 977;                       // 0.001024*977 = 1.0004480 therefore SIG_OUTPUT_COMPARE1B will be triggered every second
	sei();
	
	
	char SPI_controlreg[3] = {0, 0, 0};	//Variable for holding SPI packet for initialization of A3985 chip
	char SPI_datareg[3] = {0, 0, 0};	//Variable for holding SPI packet for the direction and control of H-bridges during operation of driving the MOSFETS (SPI to A3985 chip)
    
	
	//Note that my SPI send register on the ATMEGA168PA is only 8 bits so I break the SPI packet up into 3 separate 8 bit packets and zero pad the most
	//significant 6 bits to make one 18 bit packet
	
	/* A3985 control register SPI packet
	--------------------------------------------------------------------------------------------------------------------------------------------------
	Bit #'s ----- Value ------------------- Description ----------------------------------------------------------------------------------------------
	--------------------------------------------------------------------------------------------------------------------------------------------------
	D18-------- 1'b1  ---- Idle Mode (1'b1 -> fully operational, 1'bo -> Idle mode, Vreg is off)------------------------------------------------------
	D17:D16 --- 2'b00 ---- Reserved bits, always set these to zero -----------------------------------------------------------------------------------
	D15:D14 --- 2'b10 ---- Synchronous Rectification (2'b10 -> Active mode)---------------------------------------------------------------------------
	D13:D12 --- 2'b00 ---- Clock select (2'b00 -> use internal clock)---------------------------------------------------------------------------------
	D11:D8 ---- 4'b0000 -- Fast Time Decay (4'b0000 with a 4MHz clock -> 1.75 us)---------------------------------------------------------------------
	D7:D3 ----- 5'b00000 - Set the fixed off time for the internal PWM of the A3985 (5'b00000with a 4 MHZ clock -> 1.75 us)---------------------------
	D2:D1 ----- 2'b00 ---- Set the blank time (t_dead = t_blank/2) (2'b00 -> t_blank = 1us, tdead = 500 ns)-------------------------------------------
	D0 -------- 1'b1 ----- Choose to communicate with the control register----------------------------------------------------------------------------
	--------------------------------------------------------------------------------------------------------------------------------------------------
	*/
	
	/* A3985 data register SPI packet
	--------------------------------------------------------------------------------------------------------------------------------------------------
	Bit #'s ----- Value ------------------- Description ----------------------------------------------------------------------------------------------
	--------------------------------------------------------------------------------------------------------------------------------------------------
	D18:D17---- 2'b01 ----- Set Gm the scaling factor (2'b01 -> Gm = 12)------------------------------------------------------------------------------
	D16 ------- 1'b0 ------ Bridge 1 Mode (0 -> mixed Decay, 1 -> slow decay)-------------------------------------------------------------------------
	D15 ------- 1'b0 ------ This sets the direction of H-Bridge 1, depending on wiring of motor, 1 way turns clockwise and the other counter clockwise)
	D14:D9 ---- 6'b000010 - This sets the DAC value for controlling I_trip of Bridge2 (6'b000010 -> I_trip = 0.781 Amps)-------------------------------
	D8 -------- 1'b0 ------ Bridge 1 Mode (0 -> mixed Decay, 1 -> slow decay)--------------------------------------------------------------------------
	D7 -------- 1'b0 ------ This sets the direction of H-Bridge 1, depending on wiring of motor, 1 way turns clockwise and the other counter clockwise)
	D6:D1 ----- 6'b000010 - This sets the DAC value for controlling I_trip of Bridge1 (6'b000010 -> I_trip = 0.781 Amps)-------------------------------
	D0 -------- 1'b0 ------ Choose to communicate with the data register-------------------------------------------------------------------------------
	---------------------------------------------------------------------------------------------------------------------------------------------------
	*/
	
	SPI_controlreg[2] = 2;
	SPI_controlreg[1] = 64;
	SPI_controlreg[0] = 0;
	
	SPI_datareg[2] = 1;
	SPI_datareg[1] = 2;
	SPI_datareg[0] = 0;
	
	int i;
	SPI_MasterInit();
	SPI_MasterTransmit(SPI_controlreg[2], SPI_controlreg[1], SPI_controlreg[0]);
	
	while(1)//for(i=0; i<3; i++)
    {
		SPI_MasterTransmit(SPI_datareg[2], SPI_datareg[1], SPI_datareg[0]);
		 //DDRB  = 0b11101100; 
		 //PORTB |= _BV(3);
		 //
		 //DDRD  = 0b00000010;
		 //PORTD |= _BV(6);
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
	DDRB  = 0b11101100;   // Turn PB7,PB6, PB5, PB3, and PB2 to output pins (STR, WC, SCK, MOSI and ENABLE for A3985 SPI, PB4=MISO which is set to input, PB1 and PB0 aren't used)
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	
}
void SPI_MasterTransmit(char data1, char data2, char data3)
{
	
	pull_WC_high();
	pull_STR_low();
	
	/* Start transmission */
	SPDR = data1;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));

	
	/* Start transmission */
	SPDR = data2;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));

	
	/* Start transmission */
	SPDR = data3;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	pull_WC_low();
	pull_STR_high();

	
}
