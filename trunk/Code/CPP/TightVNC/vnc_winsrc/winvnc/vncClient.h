//  Copyright (C) 2001 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncClient.h

// vncClient class handles the following functions:
// - Recieves requests from the connected client and
//   handles them
// - Handles incoming updates properly, using a vncBuffer
//   object to keep track of screen changes
// It uses a vncBuffer and is passed the WinDesktop and
// vncServer to communicate with.

class vncClient;
typedef SHORT vncClientId;

#if (!defined(_WINVNC_VNCCLIENT))
#define _WINVNC_VNCCLIENT

#include <list>

typedef std::list<vncClientId> vncClientList;

// Includes
#include "stdhdrs.h"
#include "VSocket.h"
#include <omnithread.h>

// Custom
#include "rectlist.h"
#include "WinDesktop.h"
#include "vncRegion.h"
#include "vncBuffer.h"
#include "vncKeymap.h"
#include "libscreen/FrameBuffer.h"
#include "WindowsScreenJpegEncoder.h"

// The vncClient class itself

class vncClient
{
public:
	// Constructor/destructor
	vncClient();
	~vncClient();

	// Allow the client thread to see inside the client object
	friend class vncClientThread;

	// Init
	virtual BOOL Init(vncServer *server,
						VSocket *socket,
						BOOL reverse,
						BOOL shared,
						vncClientId newid);

	// Kill
	// The server uses this to close the client socket, causing the
	// client thread to fail, which in turn deletes the client object
	virtual void Kill();

	// Client manipulation functions for use by the server
	virtual void SetBuffer(vncBuffer *buffer);

	// Update handling functions
	virtual void TriggerUpdate(const FrameBuffer *fb);
	virtual void UpdateMouse();
	virtual void UpdateMouseShape();
	virtual void UpdateRect(RECT &rect);
	virtual void UpdateRegion(vncRegion &region);
	virtual void CopyRect(RECT &dest, POINT &source);
	virtual void UpdateClipText(LPSTR text);
	virtual void UpdatePalette();

	// Has the client sent an input event?
	virtual BOOL RemoteEventReceived() {
		BOOL result = m_remoteevent;
		m_remoteevent = FALSE;
		return result;
	}

	virtual void SetCursorPosChanged() {
		if (time(NULL) - m_pointer_event_time > 1) {
			m_cursor_pos_changed = TRUE;
		}
	}

	// Functions for setting & getting the client settings
	virtual void EnableKeyboard(BOOL enable) { m_keyboardenabled = enable; }
	virtual void EnablePointer(BOOL enable)  { m_pointerenabled = enable;  }
	virtual void BlockInput(BOOL block)      { m_inputblocked = block; }
	virtual BOOL IsKeyboardEnabled() { return m_keyboardenabled; }
	virtual BOOL IsPointerEnabled()  { return m_pointerenabled;  }
	virtual BOOL IsInputEnabled()    { return m_keyboardenabled || m_pointerenabled; }
	virtual BOOL IsInputBlocked()    { return m_inputblocked; }

	virtual const char *GetClientName();
	virtual const char *GetServerName();
	virtual vncClientId GetClientId() {return m_id;};
	char*	getStartTime() { return ctime(&startTime); }
	void	setStartTime(time_t start) {startTime = start;}
	void	setStopUpdate(BOOL stop) {m_stopupdate = stop;}
	BOOL	getStopUpdate() {return m_stopupdate;}
	BOOL SetNewFBSize();
	BOOL IncrRgnRequested(){return !m_incr_rgn.IsEmpty();};
	BOOL FullRgnRequested(){return !m_full_rgn.IsEmpty();};
	void getFullRgnRequested(rfb::Region *region);
	void UpdateLocalFormat();

	char * ConvertPath(char *path);
	char * ConvertToNetPath(char *path);

	void setViewport(const RECT &r) { m_viewport = r; }

	// Update routines
protected:
	BOOL SendUpdate(const FrameBuffer *fb);
	BOOL SendRFBMsg(CARD8 type, BYTE *buffer, int buflen);

    //
    // Return the number of rectangles produced by currently selected encoder
    // while processing the specified list of rectangles. The returned value
    // may be greater than the number of elements in rectlist because some
    // encoders may actually produce several smaller rectangles while encoding
    // one big rectangle.
    //
    // A special value -1 is returned when the encoder requires that LastRect
    // extension will be used. In that case, the number of rectangles sent can
    // be arbitrary and special "LastRect" pseudo-rectangle is sent to specify
    // that there are no more rectangles in this update.
    //
    // If asVideo is true, all the rectangles are assumed to be encoded using
    // an encoder suitable for video (currently, JPEG encoder is be used for
    // video).
    //
    int getNumEncodedRects(rectlist &rects, bool asVideo);

    //
    // Send a list of rectangles. Each rectangle which was sent successfully is
    // removed from the rectangle list. If asVideo is true, all the rectangles
    // will be encoded using an encoder suitable for video (currently, JPEG
    // encoder is be used for video).
    //
    // NOTE: fb is used only for encoding video data.
    //
    bool sendRectangles(rectlist &rects, const FrameBuffer *fb, bool asVideo);

    BOOL SendRectangle(RECT &rect);
	BOOL SendVideoRectangle(const FrameBuffer *fb, RECT &rect);
	BOOL SendCopyRect(RECT &dest, POINT &source);
	BOOL SendCursorShapeUpdate();
	BOOL SendCursorPosUpdate();
	BOOL SendLastRect();
	BOOL SendPalette();
	BOOL isPtInSharedArea(POINT &p);
    BOOL SendNewFBSize();

	RECT getEffectiveViewport() const;

	// Internal stuffs
protected:
	// Per-client settings
	int				m_protocol_minor_version;
	BOOL			m_protocol_tightvnc;
	BOOL			m_keyboardenabled;
	BOOL			m_pointerenabled;
	BOOL			m_inputblocked;
	BOOL			m_copyrect_use;
	vncClientId		m_id;

	// The screen buffer
	vncBuffer		*m_buffer;

	// Special encoder for video data
	WindowsScreenJpegEncoder *m_jpegEncoder;

	// The server
	vncServer		*m_server;

	// The socket
	VSocket			*m_socket;
	char			*m_client_name;
	char			*m_server_name;
	
	time_t startTime;
	BOOL m_stopupdate;

	// The client thread
	omni_thread		*m_thread;

	// Flag to indicate whether the client is ready for RFB messages
	BOOL			m_protocol_ready;

	// Flag to indicate that our framebuffer size has changed before
	// the client has told that it supports NewFBSize message
	BOOL			m_fb_size_changed;

	// User input information
	RECT			m_oldmousepos;
	BOOL			m_mousemoved;
	rfbPointerEventMsg	m_ptrevent;

	// Support for cursor shape updates (XCursor, RichCursor encodings)
	BOOL			m_cursor_update_pending;
	BOOL			m_cursor_update_sent;
	BOOL			m_cursor_pos_changed;
	time_t			m_pointer_event_time;
	HCURSOR			m_hcursor;
	POINT			m_cursor_pos;

	// Region structures used when preparing updates
	// - region of rects which may have changed since last update
	// - region for which incremental data is requested
	// - region for which full data is requested
	vncRegion		m_changed_rgn;
	vncRegion		m_incr_rgn;
	vncRegion		m_full_rgn;
	omni_mutex		m_regionLock;

	BOOL			m_copyrect_set;
	RECT			m_copyrect_rect;
	POINT			m_copyrect_src;

	BOOL			m_updatewanted;
	RECT			m_fullscreen;

	// When the local display is palettized, it sometimes changes...
	BOOL			m_palettechanged;

	// Information used in polling mode!
	BOOL			m_remoteevent;

	BOOL			m_use_NewFBSize;
	BOOL			m_use_PointerPos;

	// True if both Tight encoding and JPEG image quality were specified
	// by the client in SetEncodings message.
	bool			m_isJpegAllowed;

	omni_mutex		m_sendUpdateLock;

	RECT			m_viewport;

private:
	unsigned int FiletimeToTime70(FILETIME filetime);
	void SendFileDownloadData(unsigned short sizeFile, char *pFile);
	void SendFileDownloadData(unsigned int mTime);
	void SendFileUploadCancel(unsigned short reasonLen, char *reason);
	void SendFileDownloadFailed(unsigned short reasonLen, char *reason);
	void SendFileDirSizeData(DWORD64 size64);
	void SendLastRequestFailed(CARD8 typeOfRequest, CARD16 reasonLen, 
							   CARD32 sysError, char *reason);
	void SendFileSpecDirData(CARD8 flags, CARD16 specFlags, char *pDirName);
	void CloseUndoneFileTransfer();
	BOOL m_bUploadStarted;
	BOOL m_bDownloadStarted;
	HANDLE m_hFileToRead;
	HANDLE m_hFileToWrite;
	char m_UploadFilename[MAX_PATH];
	char m_DownloadFilename[MAX_PATH];
	void Time70ToFiletime(unsigned int mTime, FILETIME *pFiletime);
	unsigned int m_modTime;
	unsigned int beginUploadTime;
	unsigned int endUploadTime;
	DWORD m_rfbBlockSize;
public:
	void SendFileDownloadPortion();
};

#endif
