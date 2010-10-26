// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

// This file is ONLY about disc streaming. It's a bit unfortunately named.
// For the rest of the audio stuff, including the "real" AI, see DSP.cpp/h.

// AI disc streaming is handled completely separately from the rest of the
// audio processing. In short, it simply streams audio directly from disc
// out through the speakers.

#include "Common.h"

#include "StreamADPCM.h"
#include "AudioInterface.h"

#include "CPU.h"
#include "ProcessorInterface.h"
#include "DVDInterface.h"
#include "../PowerPC/PowerPC.h"
#include "../CoreTiming.h"
#include "../HW/SystemTimers.h"

namespace AudioInterface
{

// internal hardware addresses
enum
{
	AI_CONTROL_REGISTER		= 0x6C00,
	AI_VOLUME_REGISTER		= 0x6C04,
	AI_SAMPLE_COUNTER		= 0x6C08,
	AI_INTERRUPT_TIMING		= 0x6C0C,
};

// AI Control Register
union AICR
{
	AICR() { hex = 0;}
	AICR(u32 _hex) { hex = _hex;}
	struct 
	{
		u32 PSTAT		: 1;  // sample counter/playback enable
		u32 AIFR		: 1;  // AI Frequency (0=32khz 1=48khz)
		u32 AIINTMSK	: 1;  // 0=interrupt masked 1=interrupt enabled
		u32 AIINT		: 1;  // audio interrupt status
		u32 AIINTVLD	: 1;  // This bit controls whether AIINT is affected by the AIIT register 
                                  // matching AISLRCNT. Once set, AIINT will hold
		u32 SCRESET		: 1;  // write to reset counter
        u32 DACFR		: 1;  // DAC Frequency (0=48khz 1=32khz)
		u32				:25;
	};
	u32 hex;
};

// AI m_Volume Register
union AIVR
{
	struct
	{
		u32 leftVolume		:  8;
		u32 rightVolume		:  8;
		u32					: 16;
	};
	u32 hex;
};

// AudioInterface-Registers
struct SAudioRegister
{
	AICR m_Control;
	AIVR m_Volume;
	u32 m_SampleCounter;
	u32 m_InterruptTiming;
};

// STATE_TO_SAVE
static SAudioRegister g_AudioRegister;	
static u64 g_LastCPUTime = 0;
static unsigned int g_AISampleRate = 32000;
static unsigned int g_DACSampleRate = 32000;
static u64 g_CPUCyclesPerSample = 0xFFFFFFFFFFFULL;

void DoState(PointerWrap &p)
{
	p.Do(g_AudioRegister);
	p.Do(g_LastCPUTime);
	p.Do(g_AISampleRate);
	p.Do(g_DACSampleRate);
	p.Do(g_CPUCyclesPerSample);
}

void GenerateAudioInterrupt();
void UpdateInterrupts();
void IncreaseSampleCount(const u32 _uAmount);
void ReadStreamBlock(short* _pPCM);	

void Init()
{
	g_AudioRegister.m_SampleCounter	= 0;
	g_AudioRegister.m_Control.AIFR	= 1;
}

void Shutdown()
{
}

void Read32(u32& _rReturnValue, const u32 _Address)
{	
	//__AI_SRC_INIT compares CC006C08 to zero, loops if 2
	switch (_Address & 0xFFFF)
	{
	case AI_CONTROL_REGISTER:		//0x6C00		
        DEBUG_LOG(AUDIO_INTERFACE, "AudioInterface(R) 0x%08x", _Address);
		_rReturnValue = g_AudioRegister.m_Control.hex;

		return;

		// Sample Rate (AIGetDSPSampleRate)
		// 32bit state (highest bit PlayState)  // AIGetStreamPlayState
	case AI_VOLUME_REGISTER:		//0x6C04
        DEBUG_LOG(AUDIO_INTERFACE, "AudioInterface(R) 0x%08x", _Address);
		_rReturnValue = g_AudioRegister.m_Volume.hex;
		return;

	case AI_SAMPLE_COUNTER:			//0x6C08
        _rReturnValue = g_AudioRegister.m_SampleCounter;
		if (g_AudioRegister.m_Control.PSTAT)
			g_AudioRegister.m_SampleCounter++; // FAKE: but this is a must 
		return;

	case AI_INTERRUPT_TIMING:
		// When sample counter reaches the value of this register, the interrupt AIINT should
		// fire.
        DEBUG_LOG(AUDIO_INTERFACE, "AudioInterface(R) 0x%08x", _Address);
		_rReturnValue = g_AudioRegister.m_InterruptTiming;
		return;

	default:
        INFO_LOG(AUDIO_INTERFACE, "AudioInterface(R) 0x%08x", _Address);
		_dbg_assert_msg_(AUDIO_INTERFACE, 0, "AudioInterface - Read from ???");
		_rReturnValue = 0;
		return;
	}
}

void Write32(const u32 _Value, const u32 _Address)
{
	switch (_Address & 0xFFFF)
	{
	case AI_CONTROL_REGISTER:
		{
			AICR tmpAICtrl(_Value);
		
			g_AudioRegister.m_Control.AIINTMSK	= tmpAICtrl.AIINTMSK;
			g_AudioRegister.m_Control.AIINTVLD	= tmpAICtrl.AIINTVLD;

            // Set frequency
            if (tmpAICtrl.AIFR != g_AudioRegister.m_Control.AIFR)
            {	
                INFO_LOG(AUDIO_INTERFACE, "Change Freq to %s", tmpAICtrl.AIFR ? "48khz":"32khz");
                g_AudioRegister.m_Control.AIFR = tmpAICtrl.AIFR;
            }
			// Set DSP frequency
            if (tmpAICtrl.DACFR != g_AudioRegister.m_Control.DACFR)
            {	
                INFO_LOG(AUDIO_INTERFACE, "AI_CONTROL_REGISTER: Change DSPFR Freq to %s", tmpAICtrl.DACFR ? "48khz":"32khz");
                g_AudioRegister.m_Control.DACFR = tmpAICtrl.DACFR;
            }

			g_AISampleRate = tmpAICtrl.AIFR ? 48000 : 32000;
			g_DACSampleRate = tmpAICtrl.DACFR ? 32000 : 48000;

			g_CPUCyclesPerSample = SystemTimers::GetTicksPerSecond() / g_AISampleRate;

            // Streaming counter
            if (tmpAICtrl.PSTAT != g_AudioRegister.m_Control.PSTAT)
            {
                INFO_LOG(AUDIO_INTERFACE, "Change StreamingCounter to %s", tmpAICtrl.PSTAT ? "startet":"stopped");
                g_AudioRegister.m_Control.PSTAT	= tmpAICtrl.PSTAT;
                g_LastCPUTime = CoreTiming::GetTicks();

				// This is the only new code in this ~3,326 revision, it seems to avoid hanging Crazy Taxi,
				// while the 1080 and Wave Race music still works
				if (!tmpAICtrl.PSTAT) DVDInterface::g_bStream = false;
            }

            // AI Interrupt
			if (tmpAICtrl.AIINT)
            {
                INFO_LOG(AUDIO_INTERFACE, "Clear AI Interrupt");
                g_AudioRegister.m_Control.AIINT = 0;
            }

            // Sample Count Reset
            if (tmpAICtrl.SCRESET)	
            {	
                INFO_LOG(AUDIO_INTERFACE, "Reset SampleCounter");
                g_AudioRegister.m_SampleCounter = 0;                
                g_AudioRegister.m_Control.SCRESET = 0;

                // set PSTAT = 0 too ? at least the reversed look like this 

                g_LastCPUTime = CoreTiming::GetTicks();
            }

			// I don't think we need this
			//g_AudioRegister.m_Control = tmpAICtrl;

            UpdateInterrupts();
		}
		break;

	case AI_VOLUME_REGISTER:
		g_AudioRegister.m_Volume.hex = _Value;
		INFO_LOG(AUDIO_INTERFACE,  "Set m_Volume: left(%i) right(%i)", g_AudioRegister.m_Volume.leftVolume, g_AudioRegister.m_Volume.rightVolume);
		break;

	case AI_SAMPLE_COUNTER:
		// _dbg_assert_msg_(AUDIO_INTERFACE, 0, "AudioInterface - m_SampleCounter is Read only");
		g_AudioRegister.m_SampleCounter = _Value;
		break;

	case AI_INTERRUPT_TIMING:		
		g_AudioRegister.m_InterruptTiming = _Value;
		INFO_LOG(AUDIO_INTERFACE, "Set AudioInterrupt: 0x%08x Samples", g_AudioRegister.m_InterruptTiming);
		break;

	default:
		PanicAlert("AudioInterface unknown write");
		_dbg_assert_msg_(AUDIO_INTERFACE,0,"AudioInterface - Write to ??? %08x", _Address);
		break;
	}
}

void UpdateInterrupts()
{
	if (g_AudioRegister.m_Control.AIINT & g_AudioRegister.m_Control.AIINTMSK)
	{
		ProcessorInterface::SetInterrupt(ProcessorInterface::INT_CAUSE_AI, true);
	}
	else
	{
		ProcessorInterface::SetInterrupt(ProcessorInterface::INT_CAUSE_AI, false);
	}
}

void GenerateAudioInterrupt()
{		
	g_AudioRegister.m_Control.AIINT = 1;
	UpdateInterrupts();
}

void Callback_GetSampleRate(unsigned int &_AISampleRate, unsigned int &_DACSampleRate)
{
	_AISampleRate = g_AISampleRate;
	_DACSampleRate = g_DACSampleRate;
}

// Callback for the disc streaming
// WARNING - called from audio thread
unsigned int Callback_GetStreaming(short* _pDestBuffer, unsigned int _numSamples, unsigned int _sampleRate)
{
	if (g_AudioRegister.m_Control.PSTAT && !CCPU::IsStepping())
	{		
		static int pos = 0;
		static short pcm[28*2];
		const int lvolume = g_AudioRegister.m_Volume.leftVolume;
		const int rvolume = g_AudioRegister.m_Volume.rightVolume;


		if (g_AISampleRate == 48000 && _sampleRate == 32000)
		{
			_dbg_assert_msg_(AUDIO_INTERFACE, !(_numSamples & 1), "Number of Samples: %i must be even!", _numSamples);
			_numSamples = _numSamples * 3 / 2;
		}
		else if (g_AISampleRate == 32000 && _sampleRate == 48000)
		{
			// AyuanX: Up-sampling is not implemented yet
			PanicAlert("AUDIO_INTERFACE: Up-sampling is not implemented yet!");
		}

		int pcm_l = 0, pcm_r = 0;
		for (unsigned int i = 0; i < _numSamples; i++)
		{
			if (pos == 0)
				ReadStreamBlock(pcm);

			if (g_AISampleRate == 48000 && _sampleRate == 32000)
			{
				if (i % 3)
				{
					pcm_l = (((pcm_l + (int)pcm[pos*2]) / 2  * lvolume) >> 8) + (int)(*_pDestBuffer);
					if (pcm_l > 32767)
						pcm_l = 32767;
					else if (pcm_l < -32767)
						pcm_l = -32767;
					*_pDestBuffer++ = pcm_l;

					pcm_r = (((pcm_r + (int)pcm[pos*2+1]) / 2 * rvolume) >> 8) + (int)(*_pDestBuffer);
 					if (pcm_r > 32767)
						pcm_r = 32767;
					else if (pcm_r < -32767)
						pcm_r = -32767;
					*_pDestBuffer++ = pcm_r;
				}
				pcm_l = pcm[pos*2];
				pcm_r = pcm[pos*2+1];
			}
			else
			{
				pcm_l = (((int)pcm[pos*2] * lvolume) >> 8) + (int)(*_pDestBuffer);
				if (pcm_l > 32767)
					pcm_l = 32767;
				else if (pcm_l < -32767)
					pcm_l = -32767;
				*_pDestBuffer++ = pcm_l;

				pcm_r = (((int)pcm[pos*2+1] * rvolume) >> 8) + (int)(*_pDestBuffer);
				if (pcm_r > 32767)
					pcm_r = 32767;
				else if (pcm_r < -32767)
					pcm_r = -32767;
				*_pDestBuffer++ = pcm_r;
			}

			if (++pos == 28) 
				pos = 0;
		}
	}
	else
	{
		// Don't overwrite existed sample data
		/*
		for (unsigned int i = 0; i < _numSamples * 2; i++)
		{
			_pDestBuffer[i] = 0; //silence!
		}
		*/
	}

	return _numSamples;
}

// WARNING - called from audio thread
void ReadStreamBlock(short *_pPCM)
{
	char tempADPCM[32];
	if (DVDInterface::DVDReadADPCM((u8*)tempADPCM, 32))
	{
		NGCADPCM::DecodeBlock(_pPCM, (u8*)tempADPCM);
	}
	else
	{
		for (int j=0; j<28; j++)
		{
			*_pPCM++ = 0;
			*_pPCM++ = 0;
		}
	}

    // COMMENT:
    // our whole streaming code is "faked" ... so it shouldn't increase the sample counter
    // streaming will never work correctly this way, but at least the program will think all is alright.

	// This call must not be done wihout going through CoreTiming's threadsafe option.
	// IncreaseSampleCount(28); 
}

void IncreaseSampleCount(const u32 _iAmount)
{
	if (g_AudioRegister.m_Control.PSTAT)
	{
		g_AudioRegister.m_SampleCounter += _iAmount;
		if (g_AudioRegister.m_Control.AIINTVLD && 
            (g_AudioRegister.m_SampleCounter >= g_AudioRegister.m_InterruptTiming))
		{			
			GenerateAudioInterrupt();
		}
	}
}

void Update()
{
    // update timer
    if (g_AudioRegister.m_Control.PSTAT)
    {
        const u64 Diff = CoreTiming::GetTicks() - g_LastCPUTime;
        if (Diff > g_CPUCyclesPerSample)
        {            
            const u32 Samples = static_cast<u32>(Diff / g_CPUCyclesPerSample);
            g_LastCPUTime += Samples * g_CPUCyclesPerSample;
			IncreaseSampleCount(Samples);
        } 
    }
}

} // end of namespace AudioInterface

