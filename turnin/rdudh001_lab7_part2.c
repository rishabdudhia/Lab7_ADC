/*	Author: Rishab Dudhia
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #7  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *	Same as exercise 1 but with photoresistor instead of potentiometer
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Youtube Link: https://www.youtube.com/watch?v=3A-z3MrlQhI
 *
 *	MAX = 0x0C0 => In the video, I found when the lights were displaying the largest binary value with the bottom lights being 
 *	the most significant bits
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; // TimerISR() sets to 1

unsigned long _avr_timer_M = 1; //start count from here, down to 0. default 1 ms
unsigned long _avr_timer_cntcurr = 0; //current internal count of 1ms ticks

void TimerOn () {
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit3 = 0: CTC mode (clear timer on compare)
		       //bit2bit1bit0 = 011: pre-scaler / 64
		       //00001011: 0x0B
		       //so, 8 MHz clock or 8,000,000 / 64 = 125,000 ticks/s
		       //thus, TCNT1 register will count at 125,000 ticks/s
	//AVR output compare register OCR1A
	OCR1A = 125; //timer interrupt will be generated when TCNT1 == OCR1A
	             //we want a 1 ms tick. 0.001s * 125,000 ticks/s = 125
		     //so when TCNT1 register equals 125,
		     //1 ms has passed. thus, we compare 125.
		     
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr milliseconds
	
	//Enable global interrupts
	SREG |= 0x80; // 0x80: 10000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0 = 000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

//In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect){
	//CPU automatically calls when TCNT1 == OCR1 (every 1ms per TimerOn settings)
	_avr_timer_cntcurr--; // count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	//ADEN: setting this bit eneables analog to digital conversion
	//ADSC: setting this bit starts the first conversion
	//ADATE: setting this bit enables auto triggering. since we are in Free Running Mode, 
	//       a new conversion will trigger whenever the previous conversion completes
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
    DDRD = 0x03; PORTD = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    unsigned short tempADC;
    unsigned char temp;
    ADC_init();
    /* Insert your solution below */
    while (1) {
	temp = 0x00;
	tempADC = ADC;
	temp = (char)tempADC;
	PORTB = temp;
	temp = 0x00;
	temp = 0x300 & tempADC;
	temp = temp >> 8;
        PORTD = temp;	
    }
    return 1;
}
