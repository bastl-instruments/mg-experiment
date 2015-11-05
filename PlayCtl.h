#ifndef _PLAYCTL_H_
#define _PLAYCTL_H_
/* system includes */
/* radas includes */
/* local includes */
#include <stdint.h>


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#define FILE_SLOTS	1

namespace PlayCtl
{

	// all info needed to play the wav file
	typedef struct {
		uint32_t startBlock; 	// starting block of a file
		uint32_t length; 		// length of audio data
		uint32_t audioDataStart; // begin of the audio data
	} wavNfo_t;

	typedef struct {
		wavNfo_t wav;
		uint32_t currentPos;	// current playback position
		uint32_t currentBlock;	// current playback position

		uint8_t grainSize;		// length of a grain, actual size it this * 512
		uint8_t curGrain;

		uint8_t grainRepeat;	// how many times the grain sequence is repeated
		uint8_t curGrainRepeat;
	} slotNfo_t;

	// call this to initialize  SD card and the DAC
	int init();

	// load a file into selected slot. 
	// returns 0 on success
	int loadFile(const char *f, uint8_t slot);

	// play, stop, pause.. you guess it
	void stop();
	void pause();
	int play(uint8_t slot);
	void setSampleRate(uint16_t samplerate);
	void setGrainRepeat(uint16_t s);
	void setGrainSize(uint16_t s);

	void setLFOFreq(uint8_t f);
void setLFODepth(uint8_t d);
	void setUFOFreq(uint8_t f);
	void setUFOMod(uint8_t f);

	uint8_t currentSlot();
	const slotNfo_t *slotNfo(uint8_t slot);

	// state of the mcp basically
	bool isPlaying();
	bool isPaused();
	bool isError();


	int adcMax();

}

#endif

#endif /* _PLAYCTL_H_ */

