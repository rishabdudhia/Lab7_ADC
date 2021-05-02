/*	Author: Rishab Dudhia
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #7  Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *	LED on if ADC is >= MAX / 2
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Youtube link: https://www.youtube.com/watch?v=AgdpuM-vhYU
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

enum smstates {smstart, on, off} state;
unsigned short MAX; 

void Tick() {
	unsigned short tempADC = ADC;
	MAX = (0x0C0) / 2; 
	switch (state) {
		case smstart:
			state = off;
			break;
		case on:
			if (tempADC >= MAX)
				state = on;
			else 
				state = off;
			break;
		case off:
			if (tempADC >= MAX)
				state = on;
			else
				state = off;
			break;
		default:
			state = off;
			break;
	}

	switch (state) {
		case smstart:
			PORTB = 0x00;
			break;
		case on:
			PORTB = 0x01;
			break;
		case off: 
			PORTB = 0x00;
			break;
		default:
			break;
	}
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
    //DDRD = 0x03; PORTD = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    //unsigned short tempADC;
    //unsigned char temp;
    ADC_init();
    TimerSet(100);
    TimerOn();
    state = smstart;
    /* Insert your solution below */
    while (1) {
		Tick();
		while (!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}
