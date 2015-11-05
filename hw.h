#ifndef _HW_H_
#define _HW_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

namespace mgHW {

#define DISPLAY_ROWS 6


typedef struct {
	volatile uint16_t knob1:10;
	volatile uint16_t knob2:10;
	volatile uint16_t knob3:10;
	volatile uint16_t knob4:10;

	uint8_t btn_rec:2;
	uint8_t btn_hold:2;
	uint8_t btn_page:2;
	uint8_t btn_shift:2;

	uint8_t btn_up:2;
	uint8_t btn_down:2;


	uint8_t btn_big1:2;
	uint8_t btn_big2:2;
	uint8_t btn_big3:2;
	uint8_t btn_big4:2;
	uint8_t btn_big5:2;
	uint8_t btn_big6:2;

	struct 
	{
		uint8_t led1:1;
		uint8_t rec:1;
		uint8_t lol:1;
		uint8_t led2:1;
		uint8_t led3:1;
		uint8_t led4:1;
		uint8_t led5:1;
		uint8_t led6:1;
	} bigled;

	struct {
		uint8_t green:1;
		uint8_t red:1;
		uint8_t blue:1;
		uint8_t unused:1;
		uint8_t kled4:1;
		uint8_t kled3:1;
		uint8_t kled2:1;
		uint8_t kled1:1;
	} rgbled;

	uint8_t segmBuf[4];

	enum Led{
		LED_RED,
		LED_GREEN,
		LED_BLUE,
		LED_BIG1,
		LED_BIG2,
		LED_BIG3,
		LED_BIG4,
		LED_BIG5,
		LED_BIG6,
		LED_REC,
		LED_LOL,
		LED_KNOB1,
		LED_KNOB2,
		LED_KNOB3,
		LED_KNOB4,
	};

	enum ButtonState {
		BUTTON_NULL		=	0,
		BUTTON_PRESSED	=	1,
		BUTTON_RELEASED	=	2,
		BUTTON_HOLD		=	3
	};

} hw_t;


typedef enum {
	ROW_SEGM_1	= 5,
	ROW_SEGM_2	= 2,
	ROW_SEGM_3	= 3,
	ROW_SEGM_4	= 4,
	ROW_BIGLED = 6,
	ROW_RGBLED = 7
} DisplayRow_e;

hw_t *state();
void init();
void demo();

void displayHex(uint16_t nbr);
void displayHex(uint8_t nbr, uint8_t pos);
void setLed(hw_t::Led l, bool s);

void updateButtons();

void updateDisplay();
void updateDisplay(uint8_t row);

}



#endif

#endif /* _HW_H_ */

