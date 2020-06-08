/*	Author: armanikorsich
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include "io.h"
#include "bit.h"
#include "scheduler.h"
#include "timer.h"

void WriteVal(int val) {
	LCD_ClearScreen();
	LCD_WriteData(val + '0');
}

enum keypadStates {start, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _pnd, _star, _err};
char key;
int keypadFct(int state) {
	switch(state) {
		case start:
			PORTC = 0xEF;
			asm("nop");
			if(GetBit(PINC,0)==0) { state = _1; break; }
			if(GetBit(PINC,1)==0) { state = _4; break; }
			if(GetBit(PINC,2)==0) { state = _7; break; }
			if(GetBit(PINC,3)==0) { state = _star; break; }
	
			PORTC = 0xDF;
			asm("nop");
			if(GetBit(PINC,0)==0) { state = _2; break; }
			if(GetBit(PINC,1)==0) { state = _5; break; }
			if(GetBit(PINC,2)==0) { state = _8; break; }
			if(GetBit(PINC,3)==0) { state = _0; break; }
	
			PORTC = 0xBF;
			asm("nop");
			if(GetBit(PINC,0)==0) { state = _3; break; }
			if(GetBit(PINC,1)==0) { state = _6; break; }
			if(GetBit(PINC,2)==0) { state = _9; break; }
			if(GetBit(PINC,3)==0) { state = _pnd; break; }
			
			state = _err;
			break;
		case _0:
		case _1:
		case _2:
		case _3:
		case _4:
		case _5:
		case _6:
		case _7:
		case _8:
		case _9:
		case _pnd:
		case _star:
		case _err:
			state = start;
			break;
		default:
			state = start;
			break;
	}
	switch(state) {
		case start:
			break;
		case _0: key = 0; break;
		case _1: key = 1; break;
		case _2: key = 2; break;
		case _3: key = 3; break;
		case _4: key = 4; break;
		case _5: key = 5; break;
		case _6: key = 6; break;
		case _7: key = 7; break;
		case _8: key = 8; break;
		case _9: key = 9; break;
		case _star: key = -6; break;
		case _pnd: key = -13; break;
		case _err: key = 47; break;
		default: break;
	}
	return state;
}


char* sentence = "                CS120B is Legend... wait for it... DARY!                  ";
unsigned char offset = 0;
unsigned char i;

enum States {Start, Screen};

int scrollFct(int state) {

	switch(state) {
		case Start:
			offset = 0;
			if (key == 1) {
				state = Screen;
			} else {
				state = Start;
			}
			break;

		case Screen:
			for (i = 0; i <= 15; ++i) {
				LCD_Cursor(i+1);
				LCD_WriteData(sentence[i + offset]);
			}

			offset++;

			if ((offset + i) == strlen(sentence) - 1) {
				state = Start;
			} else {
				state = Screen;
			}
			break;

		default:
			state = Start;
			break;
	}
			
	return state;


}
	
int main(void) {
    	/* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
    	DDRD = 0xFF; PORTD = 000;

	LCD_init();
	/* Insert your solution below */

	unsigned char i;
	static task task1, task2;
	task *tasks[] = { &task1, &task2 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;
	task1.state = start;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &keypadFct;
	
	task2.state = start;
	task2.period = 200;
	task2.elapsedTime = task2.period;
	task2.TickFct = &scrollFct;
	
	unsigned long GCD = tasks[0]->period;
	for(unsigned short j = 1; j < numTasks; j++) {
		GCD = findGCD(GCD, tasks[j]->period);
	}


	TimerSet(GCD);
	TimerOn();

    	while (1){
		for(i = 0; i < numTasks; i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]-> state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
				//WriteVal(key);
			}
			tasks[i]->elapsedTime += GCD;
		}
		
		while(!TimerFlag);
		TimerFlag = 0;


    	}
    	return 1;
}
