/**
* @brief 
* @file errors.cpp
* @author J.H. 
* @date 2015-10-19
*/

/* module header */
#include "errors.h"

/* system includes C */

/* system includes C++ */


/* local includes */

#include "hw.h"


void error(uint16_t n) {
  mgHW::displayHex(n);
  mgHW::setLed(mgHW::hw_t::LED_RED, true);
  while(1){
    mgHW::updateDisplay(); 
  }
}
