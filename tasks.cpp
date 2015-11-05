/**
* @brief 
* @file tasks.cpp
* @author J.H. 
* @date 2015-10-19
*/

/* module header */
#include "tasks.h"

/* system includes C */
#include <avr/interrupt.h>
#include <string.h>

/* system includes C++ */


/* local includes */
#include "hw.h"

namespace Tasks {

typedef struct {
	uint8_t mod;
	taskCB cb;
	void *user;
} taskNfo_t;


static taskNfo_t _tasks_a[TASKS_MAX] = {
};
static volatile uint16_t	_tasks_vect = 0;
static uint8_t cnt = 0;

// if mod of a task equals current iteration, set run flag for task
ISR(TIMER0_COMPA_vect) {
	cnt = (cnt+1) % 200;
	for(uint8_t i = 0; i < TASKS_MAX; i++) {
		if(_tasks_a[i].mod == 0) continue;
		if((cnt % _tasks_a[i].mod) == 0) _tasks_vect |= _BV(i);
	}
}

void init()
{
	memset(&_tasks_a, 0, TASKS_MAX * sizeof(taskNfo_t));
}
void runTimer()
{

// set timer0 counter initial value to 0
    TCNT0=0x00;
// period is ~1 ms for 16 MHz
    OCR0A=0x4e;

    // start timer0 with /1024 prescaler
    TCCR0A = _BV(WGM01); // CTC
    TCCR0B = (1<<CS02) | (1<<CS00);
	TIMSK0=(1<<OCIE0A);
}

int8_t setTask(uint8_t taskno, uint8_t period, taskCB cb, void *user)
{
	if(taskno >= TASKS_MAX) return -1;
	_tasks_a[taskno].mod = period;
	_tasks_a[taskno].cb		= cb;
	_tasks_a[taskno].user	= user;
	_tasks_vect &= ~_BV(taskno);
	return 0;
}

int8_t setTaskMod(uint8_t taskno, uint8_t period)
{
	if(taskno >= TASKS_MAX) return -1;
	_tasks_a[taskno].mod = period;
	return 0;
}

void runTask(uint8_t taskno)
{
	if(taskno >= TASKS_MAX) return;
	_tasks_vect |= _BV(taskno);
}

void update()
{
	if(_tasks_vect == 0) return;
	for(uint8_t i = 0; i < TASKS_MAX; i++) {
		if(_tasks_vect & _BV(i)) {
			_tasks_a[i].cb(_tasks_a[i].user);
			_tasks_vect &= ~_BV(i);
		}
	}
}

}
