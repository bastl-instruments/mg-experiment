#ifndef _TASKS_H_
#define _TASKS_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#define TASKS_MAX	8

namespace Tasks {
	typedef void (*taskCB)(void *user);
	void runTask(uint8_t taskno);
	int8_t setTask(uint8_t taskno, uint8_t period, taskCB cb, void *user);
	int8_t setTaskMod(uint8_t taskno, uint8_t period);

	void init();
	void runTimer();

	void update();
}

#endif

#endif /* _TASKS_H_ */

