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
#include "DSoundStream.h"
#include "AOSoundStream.h"
#include "AlsaSoundStream.h"
#include "NullSoundStream.h"
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
		else if (backend == BACKEND_DIRECTSOUND && DSound::isValid()) 
			soundStream = new DSound(mixer, g_dspInitialize.hWnd);
		else if (backend == BACKEND_AOSOUND     && AOSound::isValid()) 
			soundStream = new AOSound(mixer);
		else if (backend == BACKEND_ALSA        && AlsaSound::isValid())
			soundStream = new AlsaSound(mixer);
		else if (backend == BACKEND_COREAUDIO   && CoreAudioSound::isValid()) 
			soundStream = new CoreAudioSound(mixer);
		else if (backend == BACKEND_PULSEAUDIO  && PulseAudio::isValid())
			soundStream = new PulseAudio(mixer);
		else if (backend == BACKEND_NULL        && NullSound::isValid()) 
			soundStream = new NullSound(mixer);

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
			PanicAlert("Could not initialize backend %s, falling back to NULL", backend.c_str());
		}
		PanicAlert("Sound backend %s is not valid, falling back to NULL", backend.c_str());

		delete soundStream;
		soundStream = new NullSound(mixer);
		soundStream->Start();
		
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

		if (DSound::isValid())  
			backends.push_back(BACKEND_DIRECTSOUND);
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
		if (NullSound::isValid()) 
			backends.push_back(BACKEND_NULL);
	   
		return backends;
	}
}
