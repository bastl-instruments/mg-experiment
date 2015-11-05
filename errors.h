#ifndef _ERRORS_H_
#define _ERRORS_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

//void error(const char* str) ;
void error(uint16_t n);

#define ERROR_UNKNOWN_UI_STATE	100
#define ERROR_CARD_INIT			200
#define ERROR_VOL_INIT			201
#define ERROR_ROOT_INIT			202

#define ERROR_FILE_OPEN			203
#define ERROR_FILE_READ			204
#define ERROR_FILE_CLOSE		205

#define ERROR_UPDATING_SDBUFF	206
#define ERROR_INITIAL_READBLOCK	207

#define ERROR_UNEVEN_FILE		500
#define ERROR_BUFFER_UNDERRUN	501

#ifdef __cplusplus
}
#endif

#endif /* _ERRORS_H_ */

