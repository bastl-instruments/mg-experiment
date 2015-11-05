/**
* @brief 
* @file mcpPlayer.cpp
* @author J.H. RADAS 2012
* @date 2015-10-09
*/

/* module header */
#include "mcpPlayer.h"
#include <avr/interrupt.h>
#include "mcpDac.h"
#include <stddef.h>
#include <avr/pgmspace.h>

namespace mcpPlayer {
	const static uint8_t dbToMult[] PROGMEM = {
	  0, 228, 203, 181, 162, 144, 128, 114, 102, 91, 81, 72, 64, 57, 51, 46, 41,
	  36, 32, 29, 26, 23, 20, 18, 16, 14, 13, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	static volatile struct _mcpPlayer _player;
	static volatile uint8_t  _volMult = 0;
	static volatile uint16_t _volOffset = 0;
	static void setRate(uint16_t rate);

	// internal functions
	static void isrStop(void) {
//        ADCSRA &= ~(1 << ADIE);  // disable ADC interrupt
		TIMSK1 &= ~_BV(OCIE1B);  // disable DAC timer interrupt
		mcpDacSend(0);
		_player.state = mcpPlayer_t::P_STOP;
	}
	

	// public
	void initMCP(bufferSwapCB cb)
	{
		_player.cb = cb;
		_player.pIdx = 0;
		_player.pBufSize = 0;
//        _player.pBuf = 0;
		_player.state = mcpPlayer_t::P_STOP;
		_player.rate = 22050;
		mcpDacInit();
	}
	void play()
	{
		if(_player.cb == NULL) {
			_player.state = mcpPlayer_t::P_ERROR;
			return;
		}
		setRate(_player.rate);
		(*_player.cb)(&_player);
		_player.state = mcpPlayer_t::P_PLAYING;
		TIMSK1 |= _BV(OCIE1B);
	}
	void stop()
	{
		cli();
		isrStop();
		sei();
		mcpDacSend(0); 
	}
	static void setRate(uint16_t rate) {
		// no pwm
		TCCR1A = 0;
		// no clock div, CTC mode
		TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
		// set TOP for timer reset
		ICR1 = F_CPU/rate; //novinka
		// compare for SD interrupt
		OCR1A =  10;
		// compare for DAC interrupt
		OCR1B = 1;
	}

	void setSampleRate(uint16_t samplerate) { 
		_player.rate = samplerate;
		if (_player.state == mcpPlayer_t::P_PLAYING) {
			//s  while (TCNT1 != 0); // GRANNYFUCKER !!!
			ICR1 = F_CPU / samplerate;
		}
	}

	void addToSampleRate(uint16_t samplerate) { 
		ICR1 = F_CPU / (_player.rate + samplerate);
	}


	ISR(TIMER1_COMPB_vect) {
//        ADCSRA &= ~(1 << ADIE);  // disable ADC interrupt
		if (_player.state != mcpPlayer_t::P_PLAYING) {
			mcpDacSend(0); 
			TIMSK1 &= ~_BV(OCIE1B);  // disable DAC timer interrupt
			return;
		}

		// pause has not been tested
		if (_player.state == mcpPlayer_t::P_PAUSE) return;
		
		if((_player.pBuf == NULL) || (_player.pIdx >= _player.pBufSize)) {
			_player.state = mcpPlayer_t::P_ERROR;
			TIMSK1 &= ~_BV(OCIE1B);  // disable DAC timer interrupt
			mcpDacSend(0); 
			return;
		}

		uint16_t d = ((_player.pBuf[_player.pIdx+1] ^ 0x80) << 4) | (_player.pBuf[_player.pIdx] >> 4);
		_player.pIdx += 2;
		if(_volMult) {
			uint32_t tmp = _volMult * (uint32_t)d;
			d= tmp >> 8;
			d+= _volOffset;
		}
		mcpDacSend(d);
		//             callback user  if out of data
		if (_player.pIdx >= _player.pBufSize) {
			(*_player.cb)(&_player);
			_player.pIdx = 0;
		}
	}

	enum mcpPlayer_t::_mcpPlayerState state() { return _player.state; }
void setVolume(uint8_t db)
{
TIMSK1 &= ~_BV(OCIE1B);  // disable DAC timer interrupt - otherwise there will be pops and clicks 
  _volMult = db > 37 ? 1 : pgm_read_byte(&dbToMult[db]);
  _volOffset = 2048 - 8 * _volMult;
TIMSK1 |= _BV(OCIE1B);
}

	uint16_t samplerate() { return _player.rate; }
}
