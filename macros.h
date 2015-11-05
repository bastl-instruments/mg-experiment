#ifndef _MACROS_H_
#define _MACROS_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#define B(x) \
  ((0##x >>  0 & 0x01) | \
  (0##x >>  2 & 0x02) | \
  (0##x >>  4 & 0x04) | \
  (0##x >>  6 & 0x08) | \
  (0##x >>  8 & 0x10) | \
  (0##x >> 10 & 0x20) | \
  (0##x >> 12 & 0x40) | \
  (0##x >> 14 & 0x80))

#ifdef __cplusplus
}
#endif

#endif /* _MACROS_H_ */

