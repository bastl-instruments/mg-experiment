/**
* @brief 
* @file PlayCtl.cpp
* @author J.H. RADAS 2012
* @date 2015-08-12
*/

/* module header */
#include "PlayCtl.h"

/* system includes C */

/* system includes C++ */

/* radas includes */

/* local includes */
#include "errors.h"
#include "mcpPlayer.h"
#include <SdFat.h>
#include "WaveStructs.h"

#include "tasks.h"
#include "mgTasks.h"
#include "hw.h"

#define SD_CS_PIN SS

namespace PlayCtl {


#define SDBUF_SIZE	512




static uint8_t 			_buf[2][SDBUF_SIZE]; 
static SdFat sd;

static struct _pctl {
	_pctl() : activeBuf(0), currentSlot(0), lock(0) {}
	uint8_t 	activeBuf:1;
	uint8_t		currentSlot:1;
	uint8_t		lock:1;
} _playCtl;


static struct {
	uint8_t f;
	uint8_t i;
	uint8_t to;
	uint8_t depth;
} _lfo;

static struct {
	uint8_t i:8;
	uint8_t mod;
	enum {
		DEST_SR
	} dest;
} _ufo;

static slotNfo_t _slots[FILE_SLOTS];
static void update(void *user);
static void task_ufo(__attribute__((unused)) void *user);
static void initLFO()
{
    TCNT2=0x00;
    OCR2A=0xff;

    TCCR2A = _BV(WGM21); // CTC
    TCCR2B =  3; // prescaler 64
	TIMSK2 = _BV(OCIE2A);

	_lfo.depth = 36;
}

#if 0
ISR(TIMER2_COMPA_vect)
{
	if(_lfo.i <= 36) {
		mcpPlayer::setVolume(36-_lfo.i);
	} else {
		mcpPlayer::setVolume(_lfo.i-36);
	}
	_lfo.i = (_lfo.i + 1 ) % (72);
	if(_lfo.i == 0) TIMSK2 &= ~_BV(OCIE2A);
}
#endif
ISR(TIMER2_COMPA_vect)
{
	if(_lfo.i <= 36) {
		mcpPlayer::setVolume(36 - _lfo.i); // ramp up
	} else
	if(_lfo.i <= 72) {
		mcpPlayer::setVolume(_lfo.i-36);	// ramp down
	}
	_lfo.i++;
//    if(_lfo.i == 36) TIMSK2 &= ~_BV(OCIE2A);
	if(_lfo.i == _lfo.to) TIMSK2 &= ~_BV(OCIE2A);
//    if(_lfo.i == 0) TIMSK2 &= ~_BV(OCIE2A);
}

void setLFO(uint16_t f)
{
TIMSK2 &= ~_BV(OCIE2A);
// for frequencies 1000 - 6000 we use 64 prescaler
if((f >= 1000) && (f <= 10000)) {
	TCCR2B = _BV(CS22);
	OCR2A = F_CPU / 64 / f;
} else {
	TCCR2B = _BV(CS20) | _BV(CS21) | _BV(CS22);
	OCR2A = F_CPU / 64 / f;
}
TIMSK2 |= _BV(OCIE2A);
}

void runLFO(uint8_t par, uint8_t to)
{
	if(_lfo.depth == 0) return;
	_lfo.i = par;
	_lfo.to = to;
	TIMSK2 |= _BV(OCIE2A);
}
void setLFOFreq(uint8_t d)
{
	OCR2A = d;
}

void setLFODepth(uint8_t d)
{
	if(d > 36) d = 36;
	_lfo.depth = d;
	if(d == 0) {
		TIMSK2 &= ~_BV(OCIE2A);
		mcpPlayer::setVolume(0);
		return;
	}

	// grain_f = samplerate / grain_len_in_samples
	// lfo_f = grain_f * depth
	// ocr2A = F_CPU / 64 / lfo_f

//    OCR2A = (F_CPU / 64) / ((mcpPlayer::samplerate() / (_slots[playCtl.currentSlot].grainSize * (SDBUF_SIZE / 2))) * _lfo.depth);
}

// read info of a WAV file. SD buffer is used to this.
static bool wavLoad(SdFile *f, wavNfo_t &out)
{
	uint32_t sdEndBlock;
	if (!f->contiguousRange(&out.startBlock, &sdEndBlock)) {
		return -2;
	}

	// read header into buffer
	WaveHeaderExtra *header = reinterpret_cast<WaveHeaderExtra *>(_buf[0]);
	// compiler will optimize these out - makes program more readable
	WaveRiff *riff = &header->riff;
	WaveFmtExtra *fmt = &header->fmt;
	WaveData *data = &header->data;
	f->rewind();

	// file must start with RIFF chunck
	if (f->read(riff, 12) != 12
			|| strncmp_P(riff->id, PSTR("RIFF"), 4)
			|| strncmp_P(riff->type, PSTR("WAVE"), 4)) {
		return false;
	}
	// fmt chunck must be next
	if (f->read(fmt, 8) != 8
			|| strncmp_P(fmt->id, PSTR("fmt "), 4)) {
		return false;
	}
	uint16_t size = fmt->size;
	if (size == 16 || size == 18) {
		if (f->read(&fmt->formatTag, size) != (int16_t)size) {
			return false;
		}
	} else {
		// don't read fmt chunk - must not be compressed so cause an error
		fmt->formatTag = WAVE_FORMAT_UNKNOWN;
	}
	if (fmt->formatTag != WAVE_FORMAT_PCM) {
		// PgmPrintln("Compression not supported");
		return false;
	}
	if (fmt->channels > 2) {
		//  PgmPrintln("Not mono/stereo!");
		return false;
	}

	while (1) {
		// read chunk ID
		if (f->read(data, 8) != 8) return false;
		if (strncmp_P(data->id, PSTR("data"), 4) == 0) {
			out.length = data->size;
			break;
		}
		// if not "data" then skip it!
		if (!f->seekCur(data->size)) return false;
	}


	out.audioDataStart  =  (f->curPosition() / SDBUF_SIZE) * SDBUF_SIZE; //((f->curPosition() / SDBUF_SIZE)+1) * SDBUF_SIZE; // align to the start of a block
	return 0;
}


// read from the file up to size of a block
// return: pointer to the beginning of the audio data
static uint16_t wavReadBlock(slotNfo_t *nfo, uint8_t *buf)
{
	uint32_t block;
	_playCtl.lock = 1;

	block = nfo->wav.startBlock + nfo->currentBlock;


// lfo stuff

	if(nfo->curGrain == 0) {
		runLFO(36,72-_lfo.depth); // rampdown, last grain is playing
	}
	if(nfo->curGrain == 1) {
		runLFO(_lfo.depth, 36); // rampup, first grain is playing
	}

	if(nfo->grainSize >= 1) {
		block += nfo->curGrain; // block to read is base + curGrain
		nfo->curGrain = (nfo->curGrain +1) % nfo->grainSize;
		if(nfo->curGrain == 0) {
			// at the end of the sequence: iterate repeat counter
			if(nfo->grainRepeat >= 1) {
				nfo->curGrainRepeat = (nfo->curGrainRepeat +1 ) % nfo->grainRepeat;
			}
			else nfo->curGrainRepeat = 0;
			if(nfo->curGrainRepeat == 0) { nfo->currentBlock += nfo->grainSize; 
			}
		} 
	} else nfo->currentBlock++;
	

    if (!sd.card()->readBlock(block, buf)) { return -1; }

//    nfo->currentPos += SDBUF_SIZE;
	
	_playCtl.lock = 0;
	return 1;
}


/*
* callback from the mcpPlayer
* TODO: make the buffer not load if grainSize = 1 and repeat > 1, as we are using the same data
*/
static void bufCB(volatile mcpPlayer::mcpPlayer_t *p)
{
	if(_playCtl.lock == 0) _playCtl.activeBuf = (_playCtl.activeBuf + 1) % 2;
	Tasks::runTask(TASK_UPDATEBUF);
	p->pBufSize = SDBUF_SIZE;
	p->pBuf = _buf[_playCtl.activeBuf];
	p->pIdx = 0;
}

int init()
{
	if(sd.begin(SD_CS_PIN, SPI_FULL_SPEED) == 0) { error(ERROR_CARD_INIT); return -1; }
	mcpPlayer::initMCP(bufCB);
	Tasks::setTask(TASK_UPDATEBUF, 0, update, NULL);
	Tasks::setTask(TASK_UFO, 0, task_ufo, NULL);
	initLFO();
	return 0;
}

int loadFile(const char *path, uint8_t slot)
{
	SdFile f;
	if (slot >= FILE_SLOTS) return -1;
	if(!f.open(&sd,path, O_READ)) { error(ERROR_FILE_OPEN); return -1; }
	if(wavLoad(&f, _slots[slot].wav) != 0) { f.close(); return -1; }
	_slots[slot].currentPos = _slots[slot].wav.audioDataStart;
	_slots[slot].currentBlock = _slots[slot].wav.audioDataStart  / SDBUF_SIZE;
	_slots[slot].curGrainRepeat = 0;
	_playCtl.activeBuf = 1;
	update(NULL); // update buffer 0
	f.close();
	return 0;
}
void stop()
{
	mcpPlayer::stop();
}
void pause()
{

}
int play(uint8_t slot)
{
	_playCtl.currentSlot = slot;
	_slots[slot].currentPos = _slots[slot].wav.audioDataStart;
	_slots[slot].currentBlock = _slots[slot].wav.audioDataStart / SDBUF_SIZE;
	_slots[slot].curGrainRepeat = 0;
	mcpPlayer::play();
	return 0;
}


bool isPlaying()
{ return (mcpPlayer::state() == mcpPlayer::mcpPlayer_t::P_PLAYING); }
bool isPaused()
{ return (mcpPlayer::state() == mcpPlayer::mcpPlayer_t::P_PAUSE); }
bool isError()
{ return (mcpPlayer::state() == mcpPlayer::mcpPlayer_t::P_ERROR); }
uint8_t currentSlot() { return _playCtl.currentSlot; }
const slotNfo_t *slotNfo(uint8_t slot)
{
	if (slot >= FILE_SLOTS) return NULL;
	return &_slots[slot];
}

// simple envelope for the grain now: triangle with attack time equal to release
// y = (A/P) * (P - abs(x % (2*P) - P) )

static void update(__attribute__((unused)) void *user)
{
	int err = wavReadBlock(&_slots[_playCtl.currentSlot], _buf[(_playCtl.activeBuf+1) % 2]);
	if(err == 0) { mcpPlayer::stop(); return; }
	if(err == -1) error(ERROR_UPDATING_SDBUFF);
}



static void task_ufo(__attribute__((unused)) void *user)
{
	_ufo.i = (_ufo.i+1) % _ufo.mod;
	mcpPlayer::addToSampleRate(_ufo.i * 100);
}

	

void setSampleRate(uint16_t sr) { mcpPlayer::setSampleRate(sr);  }

void setGrainRepeat(uint16_t s) { _slots[_playCtl.currentSlot].grainRepeat = s;  }
void setGrainSize(uint16_t s) { if(_slots[_playCtl.currentSlot].grainSize == s) return;  _slots[_playCtl.currentSlot].grainSize = s; _slots[_playCtl.currentSlot].curGrain = 0; }

void setUFOFreq(uint8_t f)
{
	Tasks::setTaskMod(TASK_UFO, f);
}
void setUFOMod(uint8_t m)
{
	_ufo.mod = m;
}

}


