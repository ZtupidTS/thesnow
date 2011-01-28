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

#include "LuaWindow.h"
#include "LuaInterface.h"
#include "../../Core/Src/CoreTiming.h"

#include <map>


int wxLuaWindow::luaCount = 0;

// Constant Colors
const unsigned long COLOR_GRAY = 0xDCDCDC;

BEGIN_EVENT_TABLE(wxLuaWindow, wxWindow)
	EVT_SIZE(                                            wxLuaWindow::OnEvent_Window_Resize)
	EVT_CLOSE(                                           wxLuaWindow::OnEvent_Window_Close)
    EVT_BUTTON(ID_BUTTON_CLOSE,                          wxLuaWindow::OnEvent_ButtonClose_Press)
	EVT_BUTTON(ID_BUTTON_LOAD,                           wxLuaWindow::OnEvent_ScriptLoad_Press)
	EVT_BUTTON(ID_BUTTON_RUN,                            wxLuaWindow::OnEvent_ScriptRun_Press)
	EVT_BUTTON(ID_BUTTON_STOP,                           wxLuaWindow::OnEvent_ScriptStop_Press)
	EVT_BUTTON(ID_BUTTON_CLEAR,                          wxLuaWindow::OnEvent_ButtonClear_Press)
END_EVENT_TABLE()

std::map<int, wxLuaWindow *> g_contextMap;

static int ev_LuaOpen, ev_LuaClose, ev_LuaStart, ev_LuaStop;


void LuaPrint(int uid, const char *msg)
{
	g_contextMap[uid]->PrintMessage(msg);
}

void LuaStop(int uid, bool ok)
{
	if(ok)
		g_contextMap[uid]->PrintMessage(_("脚本成功完成!\n"));
	//else // disabled because this message makes no sense in certain contexts, and there's always an earlier error message anyway.
	//	g_contextMap[uid]->PrintMessage("Script failed.\n");

	g_contextMap[uid]->OnStop();
}

wxLuaWindow::wxLuaWindow(wxFrame* parent, const wxPoint& pos, const wxSize& size) :
	wxFrame(parent, wxID_ANY, _("Lua 脚本控制台"), pos, size, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
	LuaWindow_InitFirstTime();

	// Create Lua context
	luaID = luaCount;
	luaCount++;
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_LuaOpen, luaID);
	g_contextMap[luaID] = this;
	bScriptRunning = false;

	// Create the GUI controls
	InitGUIControls();

	// Setup Window
	SetBackgroundColour(wxColour(COLOR_GRAY));
	SetSize(size);
	SetPosition(pos);
	Layout();
	Show();
}

wxLuaWindow::~wxLuaWindow()
{
	// On Disposal
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_LuaClose, luaID);
	g_contextMap.erase(luaID);
}

void wxLuaWindow::PrintMessage(const char *text)
{
	m_TextCtrl_Log->AppendText(wxString::FromAscii(text));
}

void wxLuaWindow::PrintMessage(const WCHAR *text)		//added
{
	m_TextCtrl_Log->AppendText(text);
}

void wxLuaWindow::InitGUIControls()
{
	// $ Log Console
	m_Tab_Log = new wxPanel(this, ID_TAB_LOG, wxDefaultPosition, wxDefaultSize);
	m_TextCtrl_Log = new wxTextCtrl(m_Tab_Log, ID_TEXTCTRL_LOG, wxT(""), wxDefaultPosition, wxSize(100, 600),
									wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
	wxBoxSizer *HStrip1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sTabLog = new wxBoxSizer(wxVERTICAL);
	sTabLog->Add(HStrip1, 0, wxALL, 5);
	sTabLog->Add(m_TextCtrl_Log, 1, wxALL|wxEXPAND, 5);

	m_Tab_Log->SetSizer(sTabLog);
	m_Tab_Log->Layout();

	// Button Strip
	m_Button_Close = new wxButton(this, ID_BUTTON_CLOSE, _("关闭"), wxDefaultPosition, wxDefaultSize);
	m_Button_LoadScript = new wxButton(this, ID_BUTTON_LOAD, _("载入脚本..."), wxDefaultPosition, wxDefaultSize);
	m_Button_Run = new wxButton(this, ID_BUTTON_RUN, _("运行"), wxDefaultPosition, wxDefaultSize);
	m_Button_Stop = new wxButton(this, ID_BUTTON_STOP, _("停止"), wxDefaultPosition, wxDefaultSize);
	m_Button_Clear = new wxButton(this, ID_BUTTON_CLEAR, _("清除"), wxDefaultPosition, wxDefaultSize);
	wxBoxSizer* sButtons = new wxBoxSizer(wxHORIZONTAL);

	m_Button_Run->Disable();
	m_Button_Stop->Disable();

	sButtons->Add(m_Button_Close, 0, wxALL, 5);
	sButtons->Add(m_Button_LoadScript, 0, wxALL, 5);
	sButtons->Add(m_Button_Run, 0, wxALL, 5);
	sButtons->Add(m_Button_Stop, 0, wxALL, 5);
	sButtons->Add(m_Button_Clear, 0, wxALL, 5);

	wxBoxSizer* sMain = new wxBoxSizer(wxVERTICAL);
	sMain->Add(m_Tab_Log, 1, wxEXPAND|wxALL, 5);
	sMain->Add(sButtons, 0, wxALL, 5);
	SetSizer(sMain);
	Layout();

	Fit();
}

void wxLuaWindow::OnEvent_ScriptLoad_Press(wxCommandEvent&  WXUNUSED(event)) 
{
	wxString path = wxFileSelector(
		_("选择要载入的脚本"),
		wxEmptyString, wxEmptyString, wxEmptyString,
		wxString::Format
		(
		_T("Lua 脚本 (lua)|*.lua|所有文件 (%s)|%s"),
		wxFileSelectorDefaultWildcardStr,
		wxFileSelectorDefaultWildcardStr
		),
		wxFD_OPEN | wxFD_PREVIEW | wxFD_FILE_MUST_EXIST,
		this);

	if(!path.IsEmpty())
		currentScript = path;
	else
		return;

	m_TextCtrl_Log->Clear();
	m_TextCtrl_Log->AppendText(wxString::FromAscii(
		StringFromFormat("Script %s loaded successfully.\n", 
						 (const char *)path.mb_str()).c_str()));
	m_Button_Run->Enable();
}

void wxLuaWindow::OnEvent_ScriptRun_Press(wxCommandEvent&  WXUNUSED(event)) 
{
	m_TextCtrl_Log->AppendText(_("运行脚本...\n"));
	bScriptRunning = true;
	m_Button_LoadScript->Disable();
	m_Button_Run->Disable();
	m_Button_Stop->Enable();

	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_LuaStart, luaID);
}

void wxLuaWindow::OnEvent_ScriptStop_Press(wxCommandEvent&  WXUNUSED(event)) 
{
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_LuaStop, luaID);
	OnStop();
	PrintMessage("Stopping script...\n");
}

void wxLuaWindow::OnStop()
{
	bScriptRunning = false;
	m_Button_LoadScript->Enable();
	m_Button_Run->Enable();
	m_Button_Stop->Disable();
}

void wxLuaWindow::OnEvent_ButtonClear_Press(wxCommandEvent& WXUNUSED (event))
{
	m_TextCtrl_Log->Clear();
}

void wxLuaWindow::OnEvent_Window_Resize(wxSizeEvent& WXUNUSED (event))
{
	Layout();
}
void wxLuaWindow::OnEvent_ButtonClose_Press(wxCommandEvent& WXUNUSED (event))
{
	Destroy();
}
void wxLuaWindow::OnEvent_Window_Close(wxCloseEvent& WXUNUSED (event))
{
	Destroy();
}


// this layer of event stuff is because Lua needs to run on the CPU thread
void wxLuaWindow::LuaOpenCallback(u64 userdata, int)
{
	Lua::OpenLuaContext((int)userdata, LuaPrint, NULL, LuaStop);
}
void wxLuaWindow::LuaCloseCallback(u64 userdata, int)
{
	Lua::CloseLuaContext((int)userdata);
}
void wxLuaWindow::LuaStartCallback(u64 userdata, int)
{
	int lid = (int)userdata;
	Lua::RunLuaScriptFile(lid,
		(const char *)g_contextMap[lid]->currentScript.mb_str());
}
void wxLuaWindow::LuaStopCallback(u64 userdata, int)
{
	Lua::StopLuaScript((int)userdata);
}
void wxLuaWindow::LuaWindow_InitFirstTime()
{
	static bool initialized = false;
	if(!initialized)
	{
		ev_LuaOpen = CoreTiming::RegisterEvent("LuaOpen", &wxLuaWindow::LuaOpenCallback);
		ev_LuaClose = CoreTiming::RegisterEvent("LuaClose", &wxLuaWindow::LuaCloseCallback);
		ev_LuaStart = CoreTiming::RegisterEvent("LuaStart", &wxLuaWindow::LuaStartCallback);
		ev_LuaStop = CoreTiming::RegisterEvent("LuaStop", &wxLuaWindow::LuaStopCallback);
		initialized = true;
	}
}
