
#ifndef _FRAMEBUFFERMANAGERBASE_H
#define _FRAMEBUFFERMANAGERBASE_H

#include <list>

#include "VideoCommon.h"

inline bool addrRangesOverlap(u32 aLower, u32 aUpper, u32 bLower, u32 bUpper)
{
	return !((aLower >= bUpper) || (bLower >= aUpper));
}

struct XFBSourceBase
{
	virtual ~XFBSourceBase() {}

	// TODO: only DX9 uses the width/height params
	virtual void Draw(const MathUtil::Rectangle<float> &sourcerc,
		const MathUtil::Rectangle<float> &drawrc, int width, int height) const = 0;

	virtual void DecodeToTexture(u32 xfbAddr, u32 fbWidth, u32 fbHeight) = 0;

	virtual void CopyEFB(float Gamma) = 0;

	u32 srcAddr;
	u32 srcWidth;
	u32 srcHeight;

	unsigned int texWidth;
	unsigned int texHeight;

	// TODO: only used by OGL
	TargetRectangle sourceRc;
};

class FramebufferManagerBase
{
public:
	enum
	{
		// There may be multiple XFBs in GameCube RAM. This is the maximum number to
		// virtualize.
		MAX_VIRTUAL_XFB = 8
	};

	FramebufferManagerBase();
	virtual ~FramebufferManagerBase();

	static void CopyToXFB(u32 xfbAddr, u32 fbWidth, u32 fbHeight, const EFBRectangle& sourceRc,float Gamma);
	static const XFBSourceBase* const* GetXFBSource(u32 xfbAddr, u32 fbWidth, u32 fbHeight, u32 &xfbCount);

protected:
	struct VirtualXFB
	{
		VirtualXFB() : xfbSource(NULL) {}

		// Address and size in GameCube RAM
		u32 xfbAddr;
		u32 xfbWidth;
		u32 xfbHeight;

		XFBSourceBase *xfbSource;
	};

	typedef std::list<VirtualXFB> VirtualXFBListType;

private:
	virtual XFBSourceBase* CreateXFBSource(unsigned int target_width, unsigned int target_height) = 0;
	// TODO: figure out why OGL is different for this guy
	virtual void GetTargetSize(unsigned int *width, unsigned int *height, const EFBRectangle& sourceRc) = 0;

	static VirtualXFBListType::iterator FindVirtualXFB(u32 xfbAddr, u32 width, u32 height);

	static void ReplaceVirtualXFB();

	// TODO: merge these virtual funcs, they are nearly all the same
	virtual void CopyToRealXFB(u32 xfbAddr, u32 fbWidth, u32 fbHeight, const EFBRectangle& sourceRc,float Gamma = 1.0f) = 0;
	static void CopyToVirtualXFB(u32 xfbAddr, u32 fbWidth, u32 fbHeight, const EFBRectangle& sourceRc,float Gamma = 1.0f);

	static const XFBSourceBase* const* GetRealXFBSource(u32 xfbAddr, u32 fbWidth, u32 fbHeight, u32 &xfbCount);
	static const XFBSourceBase* const* GetVirtualXFBSource(u32 xfbAddr, u32 fbWidth, u32 fbHeight, u32 &xfbCount);

	static XFBSourceBase *m_realXFBSource; // Only used in Real XFB mode
	static VirtualXFBListType m_virtualXFBList; // Only used in Virtual XFB mode

	static const XFBSourceBase* m_overlappingXFBArray[MAX_VIRTUAL_XFB];
};

extern FramebufferManagerBase *g_framebuffer_manager;

#endif
