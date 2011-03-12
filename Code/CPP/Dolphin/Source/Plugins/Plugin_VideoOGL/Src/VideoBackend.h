
#ifndef OGL_VIDEO_BACKEND_H_
#define OGL_VIDEO_BACKEND_H_

#include "VideoBackendBase.h"

namespace OGL
{

class VideoBackend : public VideoBackendHLE
{
	bool Initialize(void *&);
	void Shutdown();

	std::string GetName();

	void Video_Prepare();

	void ShowConfig(void* parent);

	void UpdateFPSDisplay(const char*);
	unsigned int PeekMessages();
};

}

#endif
