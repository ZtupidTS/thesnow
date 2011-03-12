﻿// Copyright (C) 2003 Dolphin Project.

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

#include <CoreServices/CoreServices.h>

#include "CoreAudioSoundStream.h"

OSStatus CoreAudioSound::callback(void *inRefCon,
	AudioUnitRenderActionFlags *ioActionFlags,
	const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
	UInt32 inNumberFrames, AudioBufferList *ioData)
{
	for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
		((CoreAudioSound *)inRefCon)->m_mixer->
			Mix((short *)ioData->mBuffers[i].mData,
				ioData->mBuffers[i].mDataByteSize / 4);

	return noErr;
}

CoreAudioSound::CoreAudioSound(CMixer *mixer) : SoundStream(mixer)
{
}

CoreAudioSound::~CoreAudioSound()
{
}

bool CoreAudioSound::Start()
{
	OSStatus err;
	AURenderCallbackStruct callback_struct;
	AudioStreamBasicDescription format;
	ComponentDescription desc;
	Component component;

	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	component = FindNextComponent(NULL, &desc);
	if (component == NULL) {
		ERROR_LOG(AUDIO, "error finding audio component");
		return false;
	}

	err = OpenAComponent(component, &audioUnit);
	if (err != noErr) {
		ERROR_LOG(AUDIO, "error opening audio component");
		return false;
	}

	FillOutASBDForLPCM(format, m_mixer->GetSampleRate(),
				2, 16, 16, false, false, false);
	err = AudioUnitSetProperty(audioUnit,
				kAudioUnitProperty_StreamFormat,
				kAudioUnitScope_Input, 0, &format,
				sizeof(AudioStreamBasicDescription));
	if (err != noErr) {
		ERROR_LOG(AUDIO, "error setting audio format");
		return false;
	}

	callback_struct.inputProc = callback;
	callback_struct.inputProcRefCon = this;
	err = AudioUnitSetProperty(audioUnit,
				kAudioUnitProperty_SetRenderCallback,
				kAudioUnitScope_Input, 0, &callback_struct,
				sizeof callback_struct);
	if (err != noErr) {
		ERROR_LOG(AUDIO, "error setting audio callback");
		return false;
	}

	err = AudioUnitInitialize(audioUnit);
	if (err != noErr) {
		ERROR_LOG(AUDIO, "error initializing audiounit");
		return false;
	}

	err = AudioOutputUnitStart(audioUnit);
	if (err != noErr) {
		ERROR_LOG(AUDIO, "error starting audiounit");
		return false;
	}

	return true;
}

void CoreAudioSound::SetVolume(int volume)
{
	OSStatus err;

	err = AudioUnitSetParameter(audioUnit,
					kHALOutputParam_Volume,
					kAudioUnitParameterFlag_Output, 0,
					volume / 100., 0);
	if (err != noErr)
		ERROR_LOG(AUDIO, "error setting volume");
}

void CoreAudioSound::SoundLoop()
{
}

void CoreAudioSound::Stop()
{
	OSStatus err;

	err = AudioOutputUnitStop(audioUnit);
	if (err != noErr)
		ERROR_LOG(AUDIO, "error stopping audiounit");

	err = AudioUnitUninitialize(audioUnit);
	if (err != noErr)
		ERROR_LOG(AUDIO, "error uninitializing audiounit");

	err = CloseComponent(audioUnit);
	if (err != noErr)
		ERROR_LOG(AUDIO, "error closing audio component");
}

void CoreAudioSound::Update()
{
}
