/**
* @brief 
* @file main.cpp
* @author J.H. RADAS 2012
* @date 2015-10-13
*/

/* module header */


/* system includes C */

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>

/* radas includes */

/* local includes */
#include "hw.h"
#include "ui.h"
#include "PlayCtl.h"
#include "tasks.h"
#include "mgTasks.h"


static void updateDisplay(__attribute ((unused)) void *user) {
	mgHW::updateDisplay();
}
static void updateButtons(__attribute ((unused)) void *user) {
	mgHW::updateButtons();
	UI::update();
}

int main()
{
	Tasks::init();

	mgHW::init();
	UI::init();

	Tasks::setTask(TASK_UPDATEUI, 20, updateButtons, 0);

	
	Tasks::runTimer();
	PlayCtl::init();

	while(1) {
		Tasks::update();
		mgHW::updateDisplay();
	}
}
