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

#include "AudioCommon.h"
#include "Mixer.h"
#include "NullSoundStream.h"
#include "DSoundStream.h"
#include "XAudio2Stream.h"
#include "AOSoundStream.h"
#include "AlsaSoundStream.h"
#include "CoreAudioSoundStream.h"
#include "OpenALStream.h"
#include "PulseAudioStream.h"

namespace AudioCommon 
{	
	SoundStream *InitSoundStream(CMixer *mixer) 
	{
		// This looks evil.
		if (!mixer)
			mixer = new CMixer();

		std::string backend = ac_Config.sBackend;
		if (backend == BACKEND_OPENAL           && OpenALStream::isValid()) 
			soundStream = new OpenALStream(mixer);
		else if (backend == BACKEND_NULLSOUND   && NullSound::isValid()) 
			soundStream = new NullSound(mixer, g_dspInitialize.hWnd);
		else if (backend == BACKEND_DIRECTSOUND && DSound::isValid()) 
			soundStream = new DSound(mixer, g_dspInitialize.hWnd);
		else if (backend == BACKEND_XAUDIO2     && XAudio2::isValid()) 
			soundStream = new XAudio2(mixer);
		else if (backend == BACKEND_AOSOUND     && AOSound::isValid()) 
			soundStream = new AOSound(mixer);
		else if (backend == BACKEND_ALSA        && AlsaSound::isValid())
			soundStream = new AlsaSound(mixer);
		else if (backend == BACKEND_COREAUDIO   && CoreAudioSound::isValid()) 
			soundStream = new CoreAudioSound(mixer);
		else if (backend == BACKEND_PULSEAUDIO  && PulseAudio::isValid())
			soundStream = new PulseAudio(mixer);

		if (soundStream != NULL)
		{
			ac_Config.Update();
			if (soundStream->Start())
			{
				// Start the sound recording
				/*
				  if (ac_Config.record) {
				  soundStream->StartLogAudio(FULL_DUMP_DIR g_Config.recordFile);
				  }
				*/
				return soundStream;
			}
			PanicAlertT("Could not initialize backend %s.", backend.c_str());
		}
		PanicAlertT("Sound backend %s is not valid.", backend.c_str());

		delete soundStream;
		soundStream = NULL;
		return NULL;
	}

	void ShutdownSoundStream() 
	{
		INFO_LOG(DSPHLE, "Shutting down sound stream");

		if (soundStream) 
		{
			soundStream->Stop();
			soundStream->StopLogAudio();
			delete soundStream;
			soundStream = NULL;
		}

		INFO_LOG(DSPHLE, "Done shutting down sound stream");	
	}

	std::vector<std::string> GetSoundBackends() 
	{
		std::vector<std::string> backends;

		if (NullSound::isValid())  
			backends.push_back(BACKEND_NULLSOUND);
		if (DSound::isValid())  
			backends.push_back(BACKEND_DIRECTSOUND);
		if (XAudio2::isValid())  
			backends.push_back(BACKEND_XAUDIO2);
		if (OpenALStream::isValid())
			backends.push_back(BACKEND_OPENAL);
		if (AOSound::isValid())   
			backends.push_back(BACKEND_AOSOUND);
		if (AlsaSound::isValid()) 
			backends.push_back(BACKEND_ALSA);
		if (CoreAudioSound::isValid())       
			backends.push_back(BACKEND_COREAUDIO);
		if (PulseAudio::isValid()) 
			backends.push_back(BACKEND_PULSEAUDIO);
	   
		return backends;
	}

	bool UseJIT() {
		return ac_Config.m_EnableJIT;
	}
}
