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

#ifndef _DX_DEBUGGER_H_
#define _DX_DEBUGGER_H_

#include <wx/wx.h>
#include <wx/notebook.h>

#include "../Globals.h"

class IniFile;

class GFXDebuggerDX9 : public wxPanel
{
public:
	GFXDebuggerDX9(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL,
		const wxString &title = wxT("DX9 Debugger"));

	virtual ~GFXDebuggerDX9();

	void SaveSettings() const;
	void LoadSettings();

	bool bInfoLog;
	bool bPrimLog;
	bool bSaveTextures;
	bool bSaveTargets;
	bool bSaveShaders;

	void EnableButtons(bool enable);

private:
	DECLARE_EVENT_TABLE();

	wxPanel *m_MainPanel;

	wxCheckBox	*m_Check[6];
	wxButton	*m_pButtonPause;
	wxButton	*m_pButtonPauseAtNext;
	wxButton	*m_pButtonPauseAtNextFrame;
	wxButton	*m_pButtonGo;
	wxChoice	*m_pPauseAtList;
	wxButton	*m_pButtonDump;
	wxChoice	*m_pDumpList;
	wxButton	*m_pButtonUpdateScreen;
	wxButton	*m_pButtonClearScreen;
	wxButton	*m_pButtonClearTextureCache;
	wxButton	*m_pButtonClearVertexShaderCache;
	wxButton	*m_pButtonClearPixelShaderCache;
	wxTextCtrl	*m_pCount;


	// WARNING: Make sure these are not also elsewhere
	enum
	{
		ID_MAINPANEL = 3900,
		ID_SAVETOFILE,
		ID_INFOLOG,
		ID_PRIMLOG,
		ID_SAVETEXTURES,
		ID_SAVETARGETS,
		ID_SAVESHADERS,
		NUM_OPTIONS,
		ID_GO,
		ID_PAUSE,
		ID_PAUSE_AT_NEXT,
		ID_PAUSE_AT_NEXT_FRAME,
		ID_PAUSE_AT_LIST,
		ID_DUMP,
		ID_DUMP_LIST,
		ID_UPDATE_SCREEN,
		ID_CLEAR_SCREEN,
		ID_CLEAR_TEXTURE_CACHE,
		ID_CLEAR_VERTEX_SHADER_CACHE,
		ID_CLEAR_PIXEL_SHADER_CACHE,
		ID_COUNT
	};

	void OnClose(wxCloseEvent& event);		
	void CreateGUIControls();

	void GeneralSettings(wxCommandEvent& event);
	void OnPauseButton(wxCommandEvent& event);
	void OnPauseAtNextButton(wxCommandEvent& event);
	void OnPauseAtNextFrameButton(wxCommandEvent& event);
	void OnDumpButton(wxCommandEvent& event);
	void OnGoButton(wxCommandEvent& event);
	void OnUpdateScreenButton(wxCommandEvent& event);
	void OnClearScreenButton(wxCommandEvent& event);
	void OnClearTextureCacheButton(wxCommandEvent& event);
	void OnClearVertexShaderCacheButton(wxCommandEvent& event);
	void OnClearPixelShaderCacheButton(wxCommandEvent& event);
	void OnCountEnter(wxCommandEvent& event);

};

enum PauseEvent {
	NOT_PAUSE	=	0,
	NEXT_FRAME	=	1<<0,
	NEXT_FLUSH	=	1<<1,

	NEXT_PIXEL_SHADER_CHANGE	=	1<<2,
	NEXT_VERTEX_SHADER_CHANGE	=	1<<3,
	NEXT_TEXTURE_CHANGE	=	1<<4,
	NEXT_NEW_TEXTURE	=	1<<5,

	NEXT_XFB_CMD	=	1<<6,
	NEXT_EFB_CMD	=	1<<7,

	NEXT_MATRIX_CMD	=	1<<8,
	NEXT_VERTEX_CMD	=	1<<9,
	NEXT_TEXTURE_CMD	=	1<<10,
	NEXT_LIGHT_CMD	=	1<<11,
	NEXT_FOG_CMD	=	1<<12,

	NEXT_SET_TLUT	=	1<<13,

	NEXT_ERROR	=	1<<14,
};

extern volatile bool DX9DebuggerPauseFlag;
extern volatile PauseEvent DX9DebuggerToPauseAtNext;
extern volatile int DX9DebuggerEventToPauseCount;
void ContinueDX9Debugger();
void DX9DebuggerCheckAndPause(bool update);
void DX9DebuggerToPause(bool update);

#undef ENABLE_DX_DEBUGGER
#if defined(_DEBUG) || defined(DEBUGFAST)
#define ENABLE_DX_DEBUGGER
#endif

#ifdef ENABLE_DX_DEBUGGER

#define DEBUGGER_PAUSE_AT(event,update) {if (((DX9DebuggerToPauseAtNext & event) && --DX9DebuggerEventToPauseCount<=0) || DX9DebuggerPauseFlag) DX9DebuggerToPause(update);}
#define DEBUGGER_PAUSE_LOG_AT(event,update,dumpfunc) {if (((DX9DebuggerToPauseAtNext & event) && --DX9DebuggerEventToPauseCount<=0) || DX9DebuggerPauseFlag) {{dumpfunc};DX9DebuggerToPause(update);}}
#define DEBUGGER_LOG_AT(event,dumpfunc) {if (( DX9DebuggerToPauseAtNext & event ) ) {{dumpfunc};}}

#else
// Not to use debugger in release build
#define DEBUGGER_PAUSE_AT(event,update)
#define DEBUGGER_PAUSE_LOG_AT(event,update,dumpfunc)
#define DEBUGGER_LOG_AT(event,dumpfunc)

#endif ENABLE_DX_DEBUGGER


#endif // _DX_DEBUGGER_H_
