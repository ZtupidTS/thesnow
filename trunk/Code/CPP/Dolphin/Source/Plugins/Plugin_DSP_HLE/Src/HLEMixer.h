#ifndef HLEMIXER_H
#define HLEMIXER_H
#include "AudioCommon.h"
#include "Mixer.h"

class HLEMixer : public CMixer 
{
public:
	HLEMixer(unsigned int AISampleRate = 48000, unsigned int DACSampleRate = 48000)
		: CMixer(AISampleRate, DACSampleRate) {};
	
	virtual void Premix(short *samples, unsigned int numSamples);
};

#endif // HLEMIXER_H


