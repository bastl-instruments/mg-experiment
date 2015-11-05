/**
* @brief 
* @file reset.cpp
* @author J.H. 
* @date 2015-10-22
*/

/* module header */
#include "reset.h"

/* system includes C */
#include <avr/wdt.h>
#include <avr/interrupt.h>

/* system includes C++ */


/* local includes */

void software_reset()
{
wdt_enable(WDTO_500MS);
WDTCSR = _BV(WDIE);
cli();
while(1);

}  
