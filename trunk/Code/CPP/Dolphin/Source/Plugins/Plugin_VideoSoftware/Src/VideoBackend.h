
#ifndef SW_VIDEO_BACKEND_H_
#define SW_VIDEO_BACKEND_H_

#include "VideoBackendBase.h"

namespace SW
{

class VideoBackend : public VideoBackendLLE
{
	void Initialize();
	void Shutdown();

	std::string GetName();

	void EmuStateChange(PLUGIN_EMUSTATE newState);

	void DoState(PointerWrap &p);

	void ShowConfig(void* parent);

	void Video_Prepare();

	void Video_EnterLoop();
	void Video_ExitLoop();
	void Video_BeginField(u32, FieldType, u32, u32);
	void Video_EndField();
	u32 Video_AccessEFB(EFBAccessType, u32, u32, u32);

	void Video_AddMessage(const char* pstr, unsigned int milliseconds);
	bool Video_Screenshot(const char* filename);

	void Video_SetRendering(bool bEnabled);

	void Video_WaitForFrameFinish();
	bool Video_IsFifoBusy();
	void Video_AbortFrame();

	void UpdateFPSDisplay(const char*);
	unsigned int PeekMessages();
};

}

#endif
