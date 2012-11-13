#ifndef __SYSTEMIMAGELIST_H__
#define __SYSTEMIMAGELIST_H__

#ifdef __WXMSW__
#include <shellapi.h>
#include <commctrl.h>
#endif

enum filetype
{
	file,
	dir,
	opened_dir
};

// Required wxImageList extension
class wxImageListEx : public wxImageList
{
public:
	wxImageListEx();
	wxImageListEx(int width, int height, const bool mask = true, int initialCount = 1);
	virtual ~wxImageListEx() {}

#ifdef __WXMSW__
	wxImageListEx(WXHIMAGELIST hList) { m_hImageList = hList; }
	HIMAGELIST GetHandle() const { return (HIMAGELIST)m_hImageList; }
	HIMAGELIST Detach();
#endif
};

class CSystemImageList
{
public:
	CSystemImageList(int size = -1);

	bool CreateSystemImageList(int size);
	virtual ~CSystemImageList();

	wxImageList* GetSystemImageList() { return m_pImageList; }

	int GetIconIndex(enum filetype type, const wxString& fileName = _T(""), bool physical = true, bool symlink = false);

#ifdef __WXMSW__
	int GetLinkOverlayIndex();
#endif

protected:
	wxImageListEx *m_pImageList;

#ifndef __WXMSW__
	std::map<wxString, int> m_iconCache;
	std::map<wxString, int> m_iconSymlinkCache;
#endif
};

#endif //__SYSTEMIMAGELIST_H__
