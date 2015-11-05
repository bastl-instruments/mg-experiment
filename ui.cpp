/**
* @brief 
* @file ui.cpp
* @author J.H. 
* @date 2015-10-14
*/

/* module header */
#include "ui.h"

/* system includes C */
#include <stddef.h>

/* system includes C++ */


/* local includes */
#include "hw.h"
#include "PlayCtl.h"
#include "errors.h"
#include "mgTasks.h"
#include "tasks.h"
#include "reset.h"

namespace UI {

	typedef struct {
		enum Page  {
			PAGE_GREEN,
			PAGE_BLUE,
			PAGE_SHIFT
		} page;
		enum Page prevstate;

		// grain repeat
		uint8_t k_gr_old:4;
		uint8_t k_gr_new:4;
		uint8_t k_gr_lock:1;

		// grain size
		uint8_t k_gs_old:4;
		uint8_t k_gs_new:4;
		uint8_t k_gs_lock:1;

		// sample rate
		uint16_t k_sr_old:10; // use 1024
		uint16_t k_sr_lock:1;

		// lfo depth
		uint8_t k_lfoD_old:6;
		uint8_t k_lfoD_new:6;
		uint8_t k_lfoD_lock:1;

		// lfo freq
		uint8_t k_lfoF_old:8;
		uint8_t k_lfoF_new:8;
		uint8_t k_lfoF_lock:1;

		// lfo user = ufo
		uint8_t k_ufoF_old:8;
		uint8_t k_ufoF_new:8;
		uint8_t k_ufoF_lock:1;

		// lfo user = ufo. mod
		uint8_t k_ufoM_old:8;
		uint8_t k_ufoM_new:8;
		uint8_t k_ufoM_lock:1;

		void lockGreen() { k_gr_lock = k_gs_lock = k_sr_lock = 1; }
		void lockBlue() { k_lfoD_lock = k_lfoF_lock = k_ufoF_lock = k_ufoM_lock = 1; }

	} ui_t;
	static struct {
		uint8_t led1:2;
		uint8_t rec:2;
		uint8_t lol:2;
		uint8_t led2:2;
		uint8_t led3:2;
		uint8_t led4:2;
		uint8_t led5:2;
		uint8_t led6:2;
	} blinker;

	static ui_t _ui;
	static mgHW::hw_t *_hw = NULL;
	static void blink(__attribute ((unused)) void *user) ;

#define SWITCHSTATE(nstate) { _ui.prevstate = _ui.page; _ui.page = nstate; }
#define PREVSTATE()			{ _ui.page = _ui.prevstate; }

	void init() {
		_hw = mgHW::state();
		_ui.page = ui_t::PAGE_GREEN;
		_ui.k_gs_lock = _ui.k_gr_lock = 0;
		Tasks::setTask(TASK_BLINK, 25, blink, NULL);
	}

	static void pageGreen()
	{
		mgHW::setLed(mgHW::hw_t::LED_GREEN, 1);

		_ui.k_gr_new = (_hw->knob2 / 100);
		_ui.k_gs_new = (_hw->knob3 / 100) + 1;

		// switch page
		if(_hw->btn_page == mgHW::hw_t::BUTTON_RELEASED) { 
			mgHW::setLed(mgHW::hw_t::LED_GREEN, 0);
			SWITCHSTATE(ui_t::PAGE_BLUE); 
			mgHW::setLed(mgHW::hw_t::LED_KNOB1, 1);
			mgHW::setLed(mgHW::hw_t::LED_KNOB2, 1);
			mgHW::setLed(mgHW::hw_t::LED_KNOB3, 1);
			mgHW::setLed(mgHW::hw_t::LED_KNOB4, 1);
			_ui.lockGreen(); 
			return;
			}

		// grain repeat
		if(_ui.k_gr_lock) {
			if(_ui.k_gr_new == _ui.k_gr_old) {
				_ui.k_gr_lock = 0;
				mgHW::setLed(mgHW::hw_t::LED_KNOB2, 0);
			}
		} else {
			if(_ui.k_gr_new != _ui.k_gr_old) {
				PlayCtl::setGrainRepeat(_ui.k_gr_old = _ui.k_gr_new);
				mgHW::displayHex(_ui.k_gr_old);
			}
		}

		// grain size
		if(_ui.k_gs_lock) {
			if(_ui.k_gs_new == _ui.k_gs_old) {
				_ui.k_gs_lock = 0;
				mgHW::setLed(mgHW::hw_t::LED_KNOB3, 0);
			}
		} else {
			if(_ui.k_gs_new != _ui.k_gs_old) {
				PlayCtl::setGrainSize(_ui.k_gs_old = _ui.k_gs_new);
				mgHW::displayHex(_ui.k_gs_old);
			}
		}

		// sample rate
		if(_ui.k_sr_lock) {
			if((_hw->knob1 ^ _ui.k_sr_old) < 32) { // unlock if change is significant
				_ui.k_sr_lock = 0;
				mgHW::setLed(mgHW::hw_t::LED_KNOB1, 0);
			}
		} else {
			if(_hw->knob1 != _ui.k_sr_old) {
				_ui.k_sr_old = _hw->knob1;
				PlayCtl::setSampleRate(_ui.k_sr_old  * 21.554 + 512);
				mgHW::displayHex(_ui.k_sr_old * 2 + 51);
			}
		}



		if(_hw->btn_big1 == mgHW::hw_t::BUTTON_PRESSED) {
			if(PlayCtl::isPlaying()) { PlayCtl::stop(); }
			else {
				PlayCtl::setSampleRate(22050);
				// play stuff
				if(PlayCtl::loadFile("A0.wav", 0) != 0 ) error(500);
				if(PlayCtl::play(0) != 0 ) error(501);
			}
		  }
		const PlayCtl::slotNfo_t *nfo = PlayCtl::slotNfo(0);
		if(_hw->btn_big2 == mgHW::hw_t::BUTTON_RELEASED) {
			mgHW::displayHex(nfo->grainRepeat);
		}
		else if(_hw->btn_big3 == mgHW::hw_t::BUTTON_RELEASED) {
			mgHW::displayHex(nfo->grainSize);
		}
		else if(_hw->btn_big4 == mgHW::hw_t::BUTTON_RELEASED) {
			mgHW::displayHex(_hw->knob1);
		}
		else if(_hw->btn_big5 == mgHW::hw_t::BUTTON_RELEASED) {
			mgHW::displayHex(_hw->knob4);
		}
		if(PlayCtl::isPlaying()) { blinker.led1 = 2; }
		else{ mgHW::setLed(mgHW::hw_t::LED_BIG1, 0); blinker.led1 = 0; }

		if(_hw->btn_shift == mgHW::hw_t::BUTTON_PRESSED) {
			SWITCHSTATE(ui_t::PAGE_SHIFT);
		}
		
	}
	static void pageBlue()
	{
		mgHW::setLed(mgHW::hw_t::LED_BLUE, 1);
		const PlayCtl::slotNfo_t *nfo = PlayCtl::slotNfo(0);
//        _ui.k_lfoD_new = (_hw->knob1 / 50);
		_ui.k_lfoF_new = _hw->knob1 >> 2;
		_ui.k_lfoD_new = _hw->knob2 / 34;
		_ui.k_ufoF_new = _hw->knob3 >> 2;
		_ui.k_ufoM_new = _hw->knob4 >> 2;
		
		// switch page	
		if(_hw->btn_page == mgHW::hw_t::BUTTON_RELEASED) {
			mgHW::setLed(mgHW::hw_t::LED_BLUE, 0); 
			SWITCHSTATE(ui_t::PAGE_GREEN); 
			mgHW::setLed(mgHW::hw_t::LED_KNOB1, 1);
			mgHW::setLed(mgHW::hw_t::LED_KNOB2, 1);
			mgHW::setLed(mgHW::hw_t::LED_KNOB3, 1);
			mgHW::setLed(mgHW::hw_t::LED_KNOB4, 1);
			_ui.lockBlue();
			return;
		}

		// lfo depth
		if(_ui.k_lfoD_lock) {
			if(_ui.k_lfoD_new == _ui.k_lfoD_old) {
				_ui.k_lfoD_lock = 0;
				mgHW::setLed(mgHW::hw_t::LED_KNOB2, 0);
			}
		} else if(_ui.k_lfoD_old != _ui.k_lfoD_new) {
			PlayCtl::setLFODepth(_ui.k_lfoD_old = _ui.k_lfoD_new);
			mgHW::displayHex(_ui.k_lfoD_old);
		}
		// lfo freq
		if(_ui.k_lfoF_lock) {
			if(_ui.k_lfoF_new == _ui.k_lfoF_old) {
				_ui.k_lfoF_lock = 0;
				mgHW::setLed(mgHW::hw_t::LED_KNOB1, 0);
			}
		} else if(_ui.k_lfoF_old != _ui.k_lfoF_new) {
			PlayCtl::setLFOFreq(_ui.k_lfoF_old = _ui.k_lfoF_new);
			mgHW::displayHex(_ui.k_lfoF_old);
		}
		// ufo freq
		if(_ui.k_ufoF_lock) {
			if(_ui.k_ufoF_new == _ui.k_ufoF_old) {
				_ui.k_ufoF_lock = 0;
				mgHW::setLed(mgHW::hw_t::LED_KNOB3, 0);
			}
		} else if(_ui.k_ufoF_old != _ui.k_ufoF_new) {
			PlayCtl::setUFOFreq(_ui.k_ufoF_old = _ui.k_ufoF_new);
			mgHW::displayHex(_ui.k_ufoF_old);
		}
		// ufo mod
		if(_ui.k_ufoM_lock) {
			if(_ui.k_ufoM_new == _ui.k_ufoM_old) {
				_ui.k_ufoM_lock = 0;
				mgHW::setLed(mgHW::hw_t::LED_KNOB4, 0);
			}
		} else if(_ui.k_ufoM_old != _ui.k_ufoM_new) {
			PlayCtl::setUFOMod(_ui.k_ufoM_old = _ui.k_ufoM_new);
			mgHW::displayHex(_ui.k_ufoM_old);
		}

		if(_hw->btn_big1 == mgHW::hw_t::BUTTON_RELEASED) {
		}  else
		if(_hw->btn_big2 == mgHW::hw_t::BUTTON_RELEASED) {
			mgHW::displayHex(nfo->wav.startBlock);
		} else
		if(_hw->btn_big3 == mgHW::hw_t::BUTTON_RELEASED) {
//            mgHW::displayHex(nfo->wav.endBlock);
		}
		else if(_hw->btn_shift == mgHW::hw_t::BUTTON_PRESSED) {
			SWITCHSTATE(ui_t::PAGE_SHIFT);
		}
	}

void pageShift()
{
	if(_hw->btn_rec == mgHW::hw_t::BUTTON_RELEASED) {
		software_reset();
	}
		else if(_hw->btn_shift == mgHW::hw_t::BUTTON_RELEASED) {
			PREVSTATE();
		}
}

	void update()
	{
		switch(_ui.page) {
			case ui_t::PAGE_GREEN:  pageGreen(); break;
			case ui_t::PAGE_BLUE:  pageBlue(); break;
			case ui_t::PAGE_SHIFT:  pageShift(); break;
			default: break;
		}
	}



// blink with 8 Hz freq maximum
static void blink(__attribute ((unused)) void *user) {
	static uint8_t cnt = 0;
	cnt = (cnt+1) % 8;
	
	if(blinker.led1 != 0) mgHW::setLed(mgHW::hw_t::LED_BIG1, cnt & blinker.led1);
	if(blinker.led2 != 0) mgHW::setLed(mgHW::hw_t::LED_BIG2, cnt & blinker.led2);
	if(blinker.led3 != 0) mgHW::setLed(mgHW::hw_t::LED_BIG3, cnt & blinker.led3);
	if(blinker.led4 != 0) mgHW::setLed(mgHW::hw_t::LED_BIG4, cnt & blinker.led4);
	if(blinker.led5 != 0) mgHW::setLed(mgHW::hw_t::LED_BIG5, cnt & blinker.led5);
	if(blinker.led6 != 0) mgHW::setLed(mgHW::hw_t::LED_BIG6, cnt & blinker.led6);

}

}
