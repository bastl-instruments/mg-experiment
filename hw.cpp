/**
* @brief 
* @file hw.cpp
* @author J.H. RADAS 2012
* @date 2015-10-13
*/

/* module header */
#include "hw.h"

/* system includes C */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

/* local includes */
#define SHIFTREGISTER_SER  C,5
#define SHIFTREGISTER_RCK  B,1
#define SHIFTREGISTER_SRCK B,0
#include "shiftRegisterFast.h"
#include "macros.h"
#include "tasks.h"
#include "mgTasks.h"

// row 4: big button leds
#define BIGLED_1_POS	1<<0
#define BIGLED_2_POS	1<<1

// button digital pins
#define BUT_PIN_BIG 	D,6
#define BUT_PIN_PAGE 	D,7

#define MAPBIT(digit, from, to) ((((digit >> from) & 1 ))  << to)
#define MAPDIGIT(dig) (MAPBIT(dig, 7, 0)| MAPBIT(dig, 6, 4)| MAPBIT(dig, 5, 6)| MAPBIT(dig, 4, 2)| MAPBIT(dig, 3, 1) | MAPBIT(dig, 2, 3) | MAPBIT(dig, 1, 7) | MAPBIT(dig, 0, 5))


namespace mgHW {
	static hw_t _hw;

	static void updateKnobs(__attribute ((unused)) void *user) {
		ADCSRA |= _BV(ADSC);
	}

	static void __initADCIRQ()
	{

		ADMUX = (1 << REFS0) | _BV(MUX0); // Set ADC reference to AVCC, channel 1

		ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz
		ADCSRA |= (1 << ADEN);  // Enable ADC
		ADCSRA |= (1 << ADIE);  // Enable ADC interrupt
		ADCSRA |= (1 << ADSC);  // Start A2D Conversions
//        ADCSRA |= (1 << ADATE);  // auto trigger enable
		sei();
	}

	ISR(ADC_vect) {
		uint8_t in;
		switch(ADMUX & 0xf) {
			case 1: _hw.knob4 = ADCL | (ADCH << 8); in = 2;  break;
			case 2: _hw.knob3 = ADCL | (ADCH << 8); in = 3; break;
			case 3: _hw.knob2 = ADCL | (ADCH << 8); in = 4; break;
			case 4: _hw.knob1 = ADCL | (ADCH << 8); in = 1; break;
			default:
				in = 1;
				break;
		}
		ADMUX = _BV(REFS0) | in;
	}

	static const PROGMEM prog_uchar _segm_typo[40]= {
	  MAPDIGIT(B(00111111)), //0
	  MAPDIGIT(B(00000110)), //1
	  MAPDIGIT(B(01011011)), //2
	  MAPDIGIT(B(01001111)), //3
	  MAPDIGIT(B(01100110)), //4
	  MAPDIGIT(B(01101101)), //5
	  MAPDIGIT(B(01111101)), //6
	  MAPDIGIT(B(00000111)), //7
	  MAPDIGIT(B(01111111)), //8
	  MAPDIGIT(B(01101111)), //9
	  MAPDIGIT(B(01110111)), //A 10
	  MAPDIGIT(B(01111100)), //b
	  MAPDIGIT(B(01011000)), //c
	  MAPDIGIT(B(01011110)), //d
	  MAPDIGIT(B(01111001)), //e
	  MAPDIGIT(B(01110001)), //f
	  MAPDIGIT(B(00111101)), //g
	  MAPDIGIT(B(01110100)), //h
	  MAPDIGIT(B(00000100)), //i
	  MAPDIGIT(B(00011110)), //j 
	  MAPDIGIT(B(01110000)), //k 20
	  MAPDIGIT(B(00111000)), //l
	  MAPDIGIT(B(01010101)), //m
	  MAPDIGIT(B(01010100)), //n
	  MAPDIGIT(B(01011100)), //o 
	  MAPDIGIT(B(01110011)), //p 25
	  MAPDIGIT(B(01100111)), //q
	  MAPDIGIT(B(01010000)), //r
	  MAPDIGIT(B(01101101)), //s //tu memit 
	  MAPDIGIT(B(01111000)), //t 
	  MAPDIGIT(B(00011100)), //u 30
	  MAPDIGIT(B(00001100)), //v 31
	  MAPDIGIT(B(01101010)), //w
	  MAPDIGIT(B(01001001)), //x 
	  MAPDIGIT(B(01110010)), //y 
	  MAPDIGIT(B(01011011)), //z tu menit 35
	  MAPDIGIT(B(00000000)), // void 36
	  MAPDIGIT(B(01000000)), //37
	  MAPDIGIT(B(01001001)), //38
	  MAPDIGIT(B(01010010)), //39 slash
	};


	hw_t *state() { return &_hw; }
	void init() {
		shiftRegFast::setup();
		memset(&_hw, 0, sizeof(hw_t));
		// init ADC interrupts 
		__initADCIRQ();
		bit_dir_inp(BUT_PIN_BIG);	
		bit_dir_inp(BUT_PIN_PAGE);	
		bit_set(BUT_PIN_BIG);
		bit_set(BUT_PIN_PAGE);
		Tasks::setTask(TASK_UPDATE_KNOBS,5, updateKnobs, 0);
	}

	void demo()
	{
		bool b = false;
		while(1) {
			setLed(hw_t::LED_BIG1, b);
			setLed(hw_t::LED_BIG2, !b);
			b = !b;

		}
	}

	void setLed(hw_t::Led l, bool s)
	{
		switch(l) {
			case hw_t::LED_BIG1: _hw.bigled.led1 = s; break;
			case hw_t::LED_BIG2: _hw.bigled.led2 = s; break;
			case hw_t::LED_BIG3: _hw.bigled.led3 = s; break;
			case hw_t::LED_BIG4: _hw.bigled.led4 = s; break;
			case hw_t::LED_BIG5: _hw.bigled.led5 = s; break;
			case hw_t::LED_BIG6: _hw.bigled.led6 = s; break;
			case hw_t::LED_REC: _hw.bigled.rec = s; break;
			case hw_t::LED_LOL: _hw.bigled.lol= s; break;

			case hw_t::LED_RED: _hw.rgbled.red= s; break;
			case hw_t::LED_BLUE: _hw.rgbled.blue= s; break;
			case hw_t::LED_GREEN: _hw.rgbled.green= s; break;
			case hw_t::LED_KNOB1: _hw.rgbled.kled1= s; break;
			case hw_t::LED_KNOB2: _hw.rgbled.kled2= s; break;
			case hw_t::LED_KNOB3: _hw.rgbled.kled3= s; break;
			case hw_t::LED_KNOB4: _hw.rgbled.kled4= s; break;
			default: return;
		}
	}

	void updateDisplay() {
		mgHW::updateDisplay(mgHW::ROW_BIGLED); // update display if idle
		mgHW::updateDisplay(mgHW::ROW_RGBLED); // update display if idle
		mgHW::updateDisplay(mgHW::ROW_SEGM_1); // update display if idle
		mgHW::updateDisplay(mgHW::ROW_SEGM_2); // update display if idle
		mgHW::updateDisplay(mgHW::ROW_SEGM_3); // update display if idle
		mgHW::updateDisplay(mgHW::ROW_SEGM_4); // update display if idle
	}

	void updateDisplay(uint8_t row)
	{
		shiftRegFast::write_8bit(_BV(row));
		switch(row) {
			case ROW_BIGLED: shiftRegFast::write_8bit(~(*(uint8_t*)&_hw.bigled)); break;
			case ROW_RGBLED: shiftRegFast::write_8bit(~(*(uint8_t*)&_hw.rgbled)); break;
			case ROW_SEGM_1: shiftRegFast::write_8bit(~(*(uint8_t*)&_hw.segmBuf[0])); break;
			case ROW_SEGM_2: shiftRegFast::write_8bit(~(*(uint8_t*)&_hw.segmBuf[1])); break;
			case ROW_SEGM_3: shiftRegFast::write_8bit(~(*(uint8_t*)&_hw.segmBuf[2])); break;
			case ROW_SEGM_4: shiftRegFast::write_8bit(~(*(uint8_t*)&_hw.segmBuf[3])); break;
			default: break;
		}
		
		shiftRegFast::enableOutput();
	}

	void displayHex(uint8_t nbr, uint8_t pos)
	{
		if(pos >= 4) return;
		if(nbr > 0xf) return;
		_hw.segmBuf[pos] = pgm_read_word_near(_segm_typo+nbr);
	}

	void displayHex(uint16_t nbr)
	{
		displayHex((nbr & 0xf000) >> 12,0);
		displayHex((nbr & 0x0f00) >> 8,1);
		displayHex((nbr & 0x00f0) >> 4,2);
		displayHex(nbr & 0x000f,3);

	}

	void updateButtons()
	{
		for(uint8_t i=0;i<8;i++){ // first read the buttons and update button states
			unsigned char whichButton=1<<i;;
			shiftRegFast::write_8bit(0);
			shiftRegFast::write_8bit(~whichButton);
			shiftRegFast::enableOutput();

			_delay_us(1); // to let voltage settle a little

			switch(i) {
				case 2:	_hw.btn_rec = (_hw.btn_rec << 1) | !bit_read_in(BUT_PIN_BIG); 
						_hw.btn_up = (_hw.btn_up << 1) | !bit_read_in(BUT_PIN_PAGE);break;
				case 1:	_hw.btn_hold = (_hw.btn_hold << 1) | !bit_read_in(BUT_PIN_BIG); 
						_hw.btn_down = (_hw.btn_down << 1) | !bit_read_in(BUT_PIN_PAGE);break;

				case 0:	_hw.btn_big1 = (_hw.btn_big1 << 1) | !bit_read_in(BUT_PIN_BIG); 
						_hw.btn_page = (_hw.btn_page << 1) | !bit_read_in(BUT_PIN_PAGE); break;
				case 3:	_hw.btn_big2 = (_hw.btn_big2 << 1) | !bit_read_in(BUT_PIN_BIG); 
						_hw.btn_shift = (_hw.btn_shift << 1) | !bit_read_in(BUT_PIN_PAGE);break;
				case 4:	_hw.btn_big3 = (_hw.btn_big3 << 1) | !bit_read_in(BUT_PIN_BIG); break;
				case 5:	_hw.btn_big4 = (_hw.btn_big4 << 1) | !bit_read_in(BUT_PIN_BIG); break;
				case 6:	_hw.btn_big5 = (_hw.btn_big5 << 1) | !bit_read_in(BUT_PIN_BIG); break;
				case 7:	_hw.btn_big6 = (_hw.btn_big6 << 1) | !bit_read_in(BUT_PIN_BIG); break;
				default:
					break;
			}
		}

	}

}
