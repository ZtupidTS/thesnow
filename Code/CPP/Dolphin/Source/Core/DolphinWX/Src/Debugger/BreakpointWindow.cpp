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

#include "BreakpointWindow.h"
#include "BreakpointView.h"
#include "CodeWindow.h"
#include "HW/Memmap.h"
#include "BreakpointDlg.h"
#include "MemoryCheckDlg.h"
#include "Host.h"
#include "PowerPC/PowerPC.h"
#include "FileUtil.h"

extern "C" {
#include "../../resources/toolbar_add_breakpoint.c"
#include "../../resources/toolbar_add_memorycheck.c"
#include "../../resources/toolbar_debugger_delete.c"
}

#define _connect_macro_(id, handler) \
	Connect(id, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(handler), (wxObject*)0, (wxEvtHandler*)parent)

class CBreakPointBar : public wxAuiToolBar
{
public:
	CBreakPointBar(wxWindow* parent, const wxWindowID id)
		: wxAuiToolBar(parent, id, wxDefaultPosition, wxDefaultSize,
				wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_TEXT)
	{
		SetToolBitmapSize(wxSize(24, 24));

		m_Bitmaps[Toolbar_Delete] =
		   	wxBitmap(wxGetBitmapFromMemory(toolbar_delete_png).ConvertToImage().Rescale(24, 24));
		m_Bitmaps[Toolbar_Add_BP] =
		   	wxBitmap(wxGetBitmapFromMemory(toolbar_add_breakpoint_png).ConvertToImage().Rescale(24, 24));
		m_Bitmaps[Toolbar_Add_MC] =
			wxBitmap(wxGetBitmapFromMemory(toolbar_add_memcheck_png).ConvertToImage().Rescale(24, 24));

		AddTool(ID_DELETE, wxT("Delete"), m_Bitmaps[Toolbar_Delete]);
		_connect_macro_(ID_DELETE, CBreakPointWindow::OnDelete);

		AddTool(ID_CLEAR, wxT("Clear"), m_Bitmaps[Toolbar_Delete]);
		_connect_macro_(ID_CLEAR, CBreakPointWindow::OnClear);

		AddTool(ID_ADDBP, wxT("+BP"), m_Bitmaps[Toolbar_Add_BP]);
		_connect_macro_(ID_ADDBP, CBreakPointWindow::OnAddBreakPoint);

		// Add memory breakpoints if you can use them
		if (Memory::AreMemoryBreakpointsActivated())
		{
			AddTool(ID_ADDMC, wxT("+MC"), m_Bitmaps[Toolbar_Add_MC]);
			_connect_macro_(ID_ADDMC, CBreakPointWindow::OnAddMemoryCheck);
		}

		AddTool(ID_LOAD, wxT("Load"), m_Bitmaps[Toolbar_Delete]);
		_connect_macro_(ID_LOAD, CBreakPointWindow::LoadAll);

		AddTool(ID_SAVE, wxT("Save"), m_Bitmaps[Toolbar_Delete]);
		_connect_macro_(ID_SAVE, CBreakPointWindow::Event_SaveAll);
	}

private:

	enum
	{
		Toolbar_Delete,
		Toolbar_Add_BP,
		Toolbar_Add_MC,
		Num_Bitmaps
	};

	enum
	{
		ID_DELETE = 2000,
		ID_CLEAR,
		ID_ADDBP,
		ID_ADDMC,
		ID_LOAD,
		ID_SAVE
	};

	wxBitmap m_Bitmaps[Num_Bitmaps];
};

BEGIN_EVENT_TABLE(CBreakPointWindow, wxPanel)
	EVT_CLOSE(CBreakPointWindow::OnClose)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, CBreakPointWindow::OnSelectBP)
END_EVENT_TABLE()

CBreakPointWindow::CBreakPointWindow(CCodeWindow* _pCodeWindow, wxWindow* parent,
	   	wxWindowID id, const wxString& title, const wxPoint& position,
	   	const wxSize& size, long style)
	: wxPanel(parent, id, position, size, style, title)
	, m_pCodeWindow(_pCodeWindow)
{
	m_mgr.SetManagedWindow(this);
	m_mgr.SetFlags(wxAUI_MGR_DEFAULT | wxAUI_MGR_LIVE_RESIZE);

	m_BreakPointListView = new CBreakPointView(this, wxID_ANY);

	m_mgr.AddPane(new CBreakPointBar(this, wxID_ANY), wxAuiPaneInfo().ToolbarPane().Top().
			LeftDockable(false).RightDockable(false).BottomDockable(false).Floatable(false));
	m_mgr.AddPane(m_BreakPointListView, wxAuiPaneInfo().CenterPane());
	m_mgr.Update();
}

CBreakPointWindow::~CBreakPointWindow()
{
	m_mgr.UnInit();
}

void CBreakPointWindow::OnClose(wxCloseEvent& event)
{
	SaveAll();
	event.Skip();
}

void CBreakPointWindow::NotifyUpdate()
{
	m_BreakPointListView->Update();
}

void CBreakPointWindow::OnDelete(wxCommandEvent& WXUNUSED(event))
{
	m_BreakPointListView->DeleteCurrentSelection();
}

// jump to begin addr
void CBreakPointWindow::OnSelectBP(wxListEvent& event)
{
	long Index = event.GetIndex();
	if (Index >= 0)
	{
		u32 Address = (u32)m_BreakPointListView->GetItemData(Index);
		if (m_pCodeWindow)
			m_pCodeWindow->JumpToAddress(Address);
	}
}

// Clear all breakpoints and memchecks
void CBreakPointWindow::OnClear(wxCommandEvent& WXUNUSED(event))
{
	PowerPC::breakpoints.Clear();
	PowerPC::memchecks.Clear();
	NotifyUpdate();
}

void CBreakPointWindow::OnAddBreakPoint(wxCommandEvent& WXUNUSED(event))
{
	BreakPointDlg bpDlg(this);
	bpDlg.ShowModal();
}

void CBreakPointWindow::OnAddMemoryCheck(wxCommandEvent& WXUNUSED(event))
{
	MemoryCheckDlg memDlg(this);
	memDlg.ShowModal();
}

void CBreakPointWindow::Event_SaveAll(wxCommandEvent& WXUNUSED(event))
{
	SaveAll();
}

void CBreakPointWindow::SaveAll()
{
	// simply dump all to bp/mc files in a way we can read again
	IniFile ini;
	if (ini.Load(File::GetUserPath(F_DEBUGGERCONFIG_IDX)))
	{
		ini.SetLines("BreakPoints", PowerPC::breakpoints.GetStrings());
		ini.SetLines("MemoryChecks", PowerPC::memchecks.GetStrings());
		ini.Save(File::GetUserPath(F_DEBUGGERCONFIG_IDX));
	}
}

void CBreakPointWindow::LoadAll(wxCommandEvent& WXUNUSED(event))
{
	IniFile ini;
	BreakPoints::TBreakPointsStr newbps;
	MemChecks::TMemChecksStr newmcs;
	
	if (!ini.Load(File::GetUserPath(F_DEBUGGERCONFIG_IDX)))
		return;
	
	if (ini.GetLines("BreakPoints", newbps, false))
		PowerPC::breakpoints.AddFromStrings(newbps);
	if (ini.GetLines("MemoryChecks", newmcs, false))
		PowerPC::memchecks.AddFromStrings(newmcs);

	NotifyUpdate();
}
