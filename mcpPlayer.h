#ifndef _MCPPLAYER_H_
#define _MCPPLAYER_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif
namespace mcpPlayer
{

	typedef void (*bufferSwapCB)(volatile struct _mcpPlayer *player);
	typedef struct _mcpPlayer {
		enum _mcpPlayerState {
			P_PLAYING,
			P_STOP,
			P_PAUSE,
			P_ERROR
		} state;
		uint16_t pIdx;	// play index
		volatile uint8_t *pBuf;  // play buffer, s16 LE
		uint16_t pBufSize;  // play buffer size
		bufferSwapCB cb;
		uint16_t rate;	// samplerate
	} mcpPlayer_t;

	void initMCP(bufferSwapCB cb);
	void play();
	void stop();
	void setSampleRate(uint16_t samplerate);
	void addToSampleRate(uint16_t samplerate);
	void setVolume(uint8_t db);
	mcpPlayer_t::_mcpPlayerState state();
	uint16_t samplerate();

	// this function should set a new buffer to the player. It will be called whenever pIdx >= pBuf.

}

#ifdef __cplusplus
}
#endif

#endif /* _MCPPLAYER_H_ */

