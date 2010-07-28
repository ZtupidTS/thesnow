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

#include "Setup.h" // Common

#include "NetWindow.h"

#include "Common.h" // Common
#include "FileUtil.h"
#include "FileSearch.h"
#include "Timer.h"

#include "Globals.h" // Local
#include "Frame.h"
#include "ConfigMain.h"
#include "PluginManager.h"
#include "MemcardManager.h"
#include "CheatsWindow.h"
#include "AboutDolphin.h"
#include "GameListCtrl.h"
#include "BootManager.h"
#include "LogWindow.h"
#include "WxUtils.h"

#include "ConfigManager.h" // Core
#include "ConsoleListener.h"
#include "Core.h"
#include "OnFrame.h"
#include "HW/DVDInterface.h"
#include "State.h"
#include "VolumeHandler.h"
#include "NANDContentLoader.h"

#include <wx/datetime.h> // wxWidgets

// ------------
// Aui events

void CFrame::OnManagerResize(wxAuiManagerEvent& event)
{
	event.Skip();
	ResizeConsole();
}

void CFrame::OnPaneClose(wxAuiManagerEvent& event)
{
	event.Veto();

	wxAuiNotebook * nb = (wxAuiNotebook*)event.pane->window;
	if (!nb) return;

	if (!g_pCodeWindow)
	{
		if ((nb->GetPage(0)->GetId() == IDM_LOGWINDOW || 
					nb->GetPage(0)->GetId() == IDM_CONSOLEWINDOW))
		{
			// Closing a pane containing the logwindow or a console closes both
			SConfig::GetInstance().m_InterfaceConsole = false;
			SConfig::GetInstance().m_InterfaceLogWindow = false;
			ToggleConsole(false);
			ToggleLogWindow(false);
		}
	}
	else
	{
		if (GetNotebookCount() == 1)
			wxMessageBox(wxT("At least one pane must remain open."),
					wxT("Notice"), wxOK, this);
		else if (nb->GetPageCount() != 0 && !nb->GetPageText(0).IsSameAs(wxT("<>")))
			wxMessageBox(wxT("You can't close panes that have pages in them."),
					wxT("Notice"), wxOK, this);
		else
		{
			// Detach and delete the empty notebook
			event.pane->DestroyOnClose(true);
			m_Mgr->ClosePane(*event.pane);
		}
	}

	m_Mgr->Update();
}

void CFrame::ToggleLogWindow(bool bShow)
{
	GetMenuBar()->FindItem(IDM_LOGWINDOW)->Check(bShow);

	if (bShow)
	{
		if (!m_LogWindow) m_LogWindow = new CLogWindow(this, IDM_LOGWINDOW);
		m_LogWindow->Enable();
		DoAddPage(m_LogWindow,
				g_pCodeWindow ? g_pCodeWindow->iNbAffiliation[0] : 0,
				g_pCodeWindow ? bFloatWindow[0] : false);
	}
	else
	{
		m_LogWindow->Disable();
		DoRemovePage(m_LogWindow, true);
	}

	// Hide or Show the pane
	if (!g_pCodeWindow)
		TogglePane();
}

void CFrame::ToggleConsole(bool bShow)
{
#ifdef _WIN32
	GetMenuBar()->FindItem(IDM_CONSOLEWINDOW)->Check(bShow);

	if (bShow)
	{
		// If the console doesn't exist, we create it
		if (!GetConsoleWindow())
		{
			ConsoleListener *Console = LogManager::GetInstance()->getConsoleListener();
			Console->Open();
		}
		else
			ShowWindow(GetConsoleWindow(), SW_SHOW);

		// Create the parent window if it doesn't exist
		wxPanel *ConsoleParent = (wxPanel*)FindWindowById(IDM_CONSOLEWINDOW);
		if (!ConsoleParent) ConsoleParent = new wxPanel(this, IDM_CONSOLEWINDOW,
				wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _("Console"));

		wxWindow *ConsoleWin = new wxWindow();
		ConsoleWin->SetHWND((WXHWND)GetConsoleWindow());
		ConsoleWin->AdoptAttributesFromHWND();
		ConsoleWin->Reparent(ConsoleParent);

		ConsoleParent->Enable();
		DoAddPage(ConsoleParent,
			   	g_pCodeWindow ? g_pCodeWindow->iNbAffiliation[1] : 0,
			   	g_pCodeWindow ? bFloatWindow[1] : false);
	}
	else // Hide
	{
		if(GetConsoleWindow())
			ShowWindow(GetConsoleWindow(), SW_HIDE); // WIN32

		wxPanel *ConsoleParent = (wxPanel*)FindWindowById(IDM_CONSOLEWINDOW);
		if (ConsoleParent)
			ConsoleParent->Disable();

		// Then close the page
		DoRemovePage(ConsoleParent, true);
	}

	// Hide or Show the pane
	if (!g_pCodeWindow)
		TogglePane();
#endif
}

void CFrame::OnToggleWindow(wxCommandEvent& event)
{
	bool bShow = GetMenuBar()->IsChecked(event.GetId());

	switch(event.GetId())
	{
		case IDM_LOGWINDOW:
			if (!g_pCodeWindow)
				SConfig::GetInstance().m_InterfaceLogWindow = event.IsChecked();
			ToggleLogWindow(bShow);
			break;
		case IDM_CONSOLEWINDOW:
			if (!g_pCodeWindow)
				SConfig::GetInstance().m_InterfaceConsole = event.IsChecked();
			ToggleConsole(event.IsChecked());
			break;
		case IDM_REGISTERWINDOW:
			g_pCodeWindow->ToggleRegisterWindow(bShow);
			break;
		case IDM_BREAKPOINTWINDOW:
			g_pCodeWindow->ToggleBreakPointWindow(bShow);
			break;
		case IDM_MEMORYWINDOW:
			g_pCodeWindow->ToggleMemoryWindow(bShow);
			break;
		case IDM_JITWINDOW:
			g_pCodeWindow->ToggleJitWindow(bShow);
			break;
		case IDM_SOUNDWINDOW:
			g_pCodeWindow->ToggleDLLWindow(IDM_SOUNDWINDOW, bShow);
			break;
		case IDM_VIDEOWINDOW:
			g_pCodeWindow->ToggleDLLWindow(IDM_VIDEOWINDOW, bShow);
			break;
	}
	event.Skip();
}

// Notebooks
// ---------------------
void CFrame::ClosePages()
{
	ToggleLogWindow(false);
	ToggleConsole(false);
	if (g_pCodeWindow)
	{
		g_pCodeWindow->ToggleCodeWindow(false);
		g_pCodeWindow->ToggleRegisterWindow(false);
		g_pCodeWindow->ToggleBreakPointWindow(false);
		g_pCodeWindow->ToggleMemoryWindow(false);
		g_pCodeWindow->ToggleJitWindow(false);
		g_pCodeWindow->ToggleDLLWindow(IDM_SOUNDWINDOW, false);
		g_pCodeWindow->ToggleDLLWindow(IDM_VIDEOWINDOW, false);
	}
}

void CFrame::OnNotebookPageChanged(wxAuiNotebookEvent& event)
{
	event.Skip();
	if (!g_pCodeWindow) return;

	// Remove the blank page if any
	AddRemoveBlankPage();

	// Update the notebook affiliation
	for (int i = IDM_LOGWINDOW; i <= IDM_CODEWINDOW; i++)
	{
		if(GetNotebookAffiliation(i) >= 0)
			g_pCodeWindow->iNbAffiliation[i - IDM_LOGWINDOW] = GetNotebookAffiliation(i);
	}
}

void CFrame::OnNotebookPageClose(wxAuiNotebookEvent& event)
{
	// Override event
	event.Veto();

	wxAuiNotebook* Ctrl = (wxAuiNotebook*)event.GetEventObject();

	if (Ctrl->GetPage(event.GetSelection())->GetId() == IDM_LOGWINDOW)
	   	ToggleLogWindow(false);
	if (Ctrl->GetPage(event.GetSelection())->GetId() == IDM_CONSOLEWINDOW)
	   	ToggleConsole(false);
	if (Ctrl->GetPage(event.GetSelection())->GetId() == IDM_REGISTERWINDOW)
		g_pCodeWindow->ToggleRegisterWindow(false);
	if (Ctrl->GetPage(event.GetSelection())->GetId() == IDM_BREAKPOINTWINDOW)
		g_pCodeWindow->ToggleBreakPointWindow(false);
	if (Ctrl->GetPage(event.GetSelection())->GetId() == IDM_JITWINDOW)
		g_pCodeWindow->ToggleJitWindow(false);
	if (Ctrl->GetPage(event.GetSelection())->GetId() == IDM_MEMORYWINDOW)
		g_pCodeWindow->ToggleMemoryWindow(false);
	if (Ctrl->GetPage(event.GetSelection())->GetId() == IDM_SOUNDWINDOW)
		g_pCodeWindow->ToggleDLLWindow(IDM_SOUNDWINDOW, false);
	if (Ctrl->GetPage(event.GetSelection())->GetId() == IDM_VIDEOWINDOW)
		g_pCodeWindow->ToggleDLLWindow(IDM_VIDEOWINDOW, false);
}

void CFrame::OnFloatingPageClosed(wxCloseEvent& event)
{
	ToggleFloatWindow(event.GetId() - IDM_LOGWINDOW_PARENT + IDM_FLOAT_LOGWINDOW);
}

void CFrame::OnFloatingPageSize(wxSizeEvent& event)
{
	event.Skip();
	ResizeConsole();
}

void CFrame::OnFloatWindow(wxCommandEvent& event)
{
	ToggleFloatWindow(event.GetId());
}

void CFrame::ToggleFloatWindow(int Id)
{
	wxWindowID WinId = Id - IDM_FLOAT_LOGWINDOW + IDM_LOGWINDOW;
	if (GetNotebookPageFromId(WinId))
	{
		DoFloatNotebookPage(WinId);
		bFloatWindow[WinId - IDM_LOGWINDOW] = true;
	}
	else
	{
		if (FindWindowById(WinId))
			DoUnfloatPage(WinId - IDM_LOGWINDOW + IDM_LOGWINDOW_PARENT);
		bFloatWindow[WinId - IDM_LOGWINDOW] = false;
	}
}

void CFrame::DoFloatNotebookPage(wxWindowID Id)
{
	wxPanel *Win = (wxPanel*)FindWindowById(Id);
	if (!Win) return;

	for (int i = 0; i < GetNotebookCount(); i++)
	{
		wxAuiNotebook *nb = GetNotebookFromId(i);
		if (nb->GetPageIndex(Win) != wxNOT_FOUND)
		{
			nb->RemovePage(nb->GetPageIndex(Win));
			// Create the parent frame and reparent the window
			CreateParentFrame(Win->GetId() + IDM_LOGWINDOW_PARENT - IDM_LOGWINDOW, Win->GetName(), Win);
			if (nb->GetPageCount() == 0)
				AddRemoveBlankPage();
		}
	}
}

void CFrame::DoUnfloatPage(int Id)
{
	wxFrame * Win = (wxFrame*)FindWindowById(Id);
	if (!Win) return;

	wxWindow * Child = Win->GetWindowChildren().Item(0)->GetData();
	Child->Reparent(this);
	DoAddPage(Child, g_pCodeWindow->iNbAffiliation[Child->GetId() - IDM_LOGWINDOW], false);
	Win->Destroy();
}

void CFrame::OnTab(wxAuiNotebookEvent& event)
{
	event.Skip();
	if (!g_pCodeWindow) return;

	// Create the popup menu
	wxMenu* MenuPopup = new wxMenu;

	wxMenuItem* Item =  new wxMenuItem(MenuPopup, wxID_ANY,
			 wxT("ѡ�񸡶�����"));
	MenuPopup->Append(Item);
	Item->Enable(false);
	MenuPopup->Append(new wxMenuItem(MenuPopup));
	for (int i = IDM_LOGWINDOW; i <= IDM_CODEWINDOW; i++)
	{
		wxWindow *Win = FindWindowById(i);
		if (Win && Win->IsEnabled())
		{
			Item = new wxMenuItem(MenuPopup, i + IDM_FLOAT_LOGWINDOW - IDM_LOGWINDOW,
				   	Win->GetName(), wxT(""), wxITEM_CHECK);
			MenuPopup->Append(Item);
			Item->Check(!!FindWindowById(i + IDM_LOGWINDOW_PARENT - IDM_LOGWINDOW));
		}
	}

	// Line up our menu with the cursor
	wxPoint Pt = ::wxGetMousePosition();
	Pt = ScreenToClient(Pt);
	// Show
	PopupMenu(MenuPopup, Pt);
}

void CFrame::OnAllowNotebookDnD(wxAuiNotebookEvent& event)
{
	event.Skip();
	event.Allow();
	ResizeConsole();
}

void CFrame::TogglePane()
{
	// Get the first notebook
	wxAuiNotebook * NB = NULL;
	for (u32 i = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook)))
			NB = (wxAuiNotebook*)m_Mgr->GetAllPanes().Item(i).window;
	}
	if (NB)
	{
		if (NB->GetPageCount() == 0)
		{
			m_LogWindow->x = m_Mgr->GetPane(wxT("Pane 1")).rect.GetWidth();
			m_LogWindow->y = m_Mgr->GetPane(wxT("Pane 1")).rect.GetHeight();
			m_LogWindow->winpos = m_Mgr->GetPane(wxT("Pane 1")).dock_direction;
			m_Mgr->GetPane(wxT("Pane 1")).Hide();
		}
		else
		{
			m_Mgr->GetPane(wxT("Pane 1")).BestSize(m_LogWindow->x, m_LogWindow->y)
				.MinSize(m_LogWindow->x, m_LogWindow->y)
				.Direction(m_LogWindow->winpos).Show();
			m_Mgr->Update();

			// Reset the minimum size of the pane
			m_Mgr->GetPane(wxT("Pane 1")).MinSize(-1, -1);
		}
		m_Mgr->Update();
	}
}

void CFrame::DoRemovePage(wxWindow *Win, bool bHide)
{
	if (!Win) return;

	wxWindow *Parent = FindWindowById(Win->GetId() + IDM_LOGWINDOW_PARENT - IDM_LOGWINDOW);

	if (Parent)
	{
		if (bHide)
		{
			Win->Hide();
			Win->Reparent(this);
		}
		else
			Win->Close();
		Parent->Destroy();
	}
	else
	{
		for (int i = 0; i < GetNotebookCount(); i++)
		{
			int PageIndex = GetNotebookFromId(i)->GetPageIndex(Win);
			if (PageIndex != wxNOT_FOUND)
			{
				GetNotebookFromId(i)->RemovePage(PageIndex);
				if (bHide)
				{
					Win->Hide();
					Win->Reparent(this);
				}
				else
					Win->Close();
			}
		}
	}
	if (g_pCodeWindow)
		AddRemoveBlankPage();
}

void CFrame::DoAddPage(wxWindow *Win, int i, bool Float)
{
	if (!Win) return;
	if (GetNotebookCount() == 0) return;
	if (i < 0 || i > GetNotebookCount()-1) i = 0;
	if (Win && GetNotebookFromId(i)->GetPageIndex(Win) != wxNOT_FOUND) return;
	if (!Float)
		GetNotebookFromId(i)->AddPage(Win, Win->GetName(), true);
	else
		CreateParentFrame(Win->GetId() + IDM_LOGWINDOW_PARENT - IDM_LOGWINDOW, Win->GetName(), Win);
}

// Toolbar
void CFrame::OnDropDownSettingsToolbar(wxAuiToolBarEvent& event)
{
	event.Skip();
	ClearStatusBar();

	if (event.IsDropDownClicked())
	{
		wxAuiToolBar* Tb = static_cast<wxAuiToolBar*>(event.GetEventObject());
		Tb->SetToolSticky(event.GetId(), true);

		// Create the popup menu
		wxMenu* menuPopup = new wxMenu;

		wxMenuItem* Item =  new wxMenuItem(menuPopup, IDM_PERSPECTIVES_ADD_PANE, wxT("��������"));
		menuPopup->Append(Item);
		menuPopup->Append(new wxMenuItem(menuPopup));
		Item = new wxMenuItem(menuPopup, IDM_TAB_SPLIT, wxT("Tab split"), wxT(""), wxITEM_CHECK);
		menuPopup->Append(Item);
		Item->Check(m_bTabSplit);
		Item = new wxMenuItem(menuPopup, IDM_NO_DOCKING, wxT("No docking"), wxT(""), wxITEM_CHECK);
		menuPopup->Append(Item);
		Item->Check(m_bNoDocking);

		// Line up our menu with the button
		wxRect rect = Tb->GetToolRect(event.GetId());
		wxPoint Pt = Tb->ClientToScreen(rect.GetBottomLeft());
		Pt = ScreenToClient(Pt);
		// Show
		PopupMenu(menuPopup, Pt);
		// Make the button un-stuck again
		if (!m_bEdit) Tb->SetToolSticky(event.GetId(), false);
	}
}

void CFrame::OnDropDownToolbarItem(wxAuiToolBarEvent& event)
{
	event.Skip();
	ClearStatusBar();

	if (event.IsDropDownClicked())
	{
		wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(event.GetEventObject());
		tb->SetToolSticky(event.GetId(), true);

		// create the popup menu
		wxMenu* menuPopup = new wxMenu;
		wxMenuItem* Item = new wxMenuItem(menuPopup, IDM_ADD_PERSPECTIVE, wxT("Create new perspective"));
		menuPopup->Append(Item);

		if (Perspectives.size() > 0)
		{
			menuPopup->Append(new wxMenuItem(menuPopup));
			for (u32 i = 0; i < Perspectives.size(); i++)
			{
				wxMenuItem* mItem = new wxMenuItem(menuPopup, IDM_PERSPECTIVES_0 + i,
					   	wxString::FromAscii(Perspectives[i].Name.c_str()), wxT(""), wxITEM_CHECK);
				menuPopup->Append(mItem);
				if (i == ActivePerspective) mItem->Check(true);
			}
		}

		// line up our menu with the button
		wxRect rect = tb->GetToolRect(event.GetId());
		wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
		pt = ScreenToClient(pt);
		// show
		PopupMenu(menuPopup, pt);
		// make sure the button is "un-stuck"
		tb->SetToolSticky(event.GetId(), false);
	}
}

void CFrame::OnToolBar(wxCommandEvent& event)
{
	ClearStatusBar();

	switch (event.GetId())
	{
		case IDM_SAVE_PERSPECTIVE:
			if (Perspectives.size() == 0)
			{
				wxMessageBox(wxT("Please create a perspective before saving"),
						wxT("Notice"), wxOK, this);
				return;
			}
			SaveIniPerspectives();
			GetStatusBar()->SetStatusText(wxString::FromAscii(std::string
						("Saved " + Perspectives[ActivePerspective].Name).c_str()), 0);
			break;
		case IDM_PERSPECTIVES_ADD_PANE:
			AddPane();
			break;
		case IDM_EDIT_PERSPECTIVES:
			m_bEdit = !m_bEdit;
			m_ToolBarAui->SetToolSticky(IDM_EDIT_PERSPECTIVES, m_bEdit);
			TogglePaneStyle(m_bEdit, IDM_EDIT_PERSPECTIVES);
			break;
	}
}

void CFrame::OnDropDownToolbarSelect(wxCommandEvent& event)
{
	ClearStatusBar();

	switch(event.GetId())
	{
		case IDM_ADD_PERSPECTIVE:
			{
				wxTextEntryDialog dlg(this, wxT("Enter a name for the new perspective:"),
					   	wxT("Create new perspective"));
				wxString DefaultValue = wxString::Format(wxT("Perspective %d"),
					   	Perspectives.size() + 1);
				dlg.SetValue(DefaultValue);
				bool DlgOk = false; int Return = 0;
				while (!DlgOk)
				{
					Return = dlg.ShowModal();
					if (Return == wxID_CANCEL)
						return;
					else if (dlg.GetValue().Find(wxT(",")) != -1)
					{
						wxMessageBox(wxT("���Ʋ��ܺ���Сд�� ',' ������"), 
								wxT("Notice"), wxOK, this);
						wxString Str = dlg.GetValue();
						Str.Replace(wxT(","), wxT(""), true);
						dlg.SetValue(Str);
					}
					else if (dlg.GetValue().IsSameAs(wxT("")))
					{
						wxMessageBox(wxT("���Ʋ���Ϊ��"), 
						wxT("Notice"), wxOK, this);
						dlg.SetValue(DefaultValue);
					}
					else
						DlgOk = true;
				}

				SPerspectives Tmp;
				Tmp.Name = dlg.GetValue().mb_str();
				Tmp.Perspective = m_Mgr->SavePerspective();

				ActivePerspective = Perspectives.size();
				Perspectives.push_back(Tmp);

				UpdateCurrentPerspective();
			}
			break;
		case IDM_TAB_SPLIT:
			m_bTabSplit = event.IsChecked();
			ToggleNotebookStyle(m_bTabSplit, wxAUI_NB_TAB_SPLIT);
			break;
		case IDM_NO_DOCKING:
			m_bNoDocking = event.IsChecked();
			TogglePaneStyle(m_bNoDocking, IDM_NO_DOCKING);
			break;
	}
}

void CFrame::ResetToolbarStyle()
{
	wxAuiPaneInfoArray& AllPanes = m_Mgr->GetAllPanes();
	for (int i = 0, Count = (int)AllPanes.GetCount(); i < Count; ++i)
	{
		wxAuiPaneInfo& Pane = AllPanes.Item(i);
		if (Pane.window->IsKindOf(CLASSINFO(wxAuiToolBar)))
		{
			Pane.Show();
			// Show all of it
			if (Pane.rect.GetLeft() > GetClientSize().GetX() - 50)
				Pane.Position(GetClientSize().GetX() - Pane.window->GetClientSize().GetX());
		}
	}
	m_Mgr->Update();
}

void CFrame::TogglePaneStyle(bool On, int EventId)
{
	wxAuiPaneInfoArray& AllPanes = m_Mgr->GetAllPanes();
	for (u32 i = 0; i < AllPanes.GetCount(); ++i)
	{
		wxAuiPaneInfo& Pane = AllPanes.Item(i);
		if (Pane.window->IsKindOf(CLASSINFO(wxAuiNotebook)))
		{
			// Default
			Pane.CloseButton(true);
			Pane.MaximizeButton(true);
			Pane.MinimizeButton(true);
			Pane.PinButton(true);
			Pane.Show();

			switch(EventId)
			{
				case IDM_EDIT_PERSPECTIVES:
					Pane.CaptionVisible(On);
					Pane.Movable(On);
					Pane.Floatable(On);
					Pane.Dockable(On);
					break;
			}
			Pane.Dockable(!m_bNoDocking);
		}
	}
	m_Mgr->Update();
}

void CFrame::ToggleNotebookStyle(bool On, long Style)
{
	wxAuiPaneInfoArray& AllPanes = m_Mgr->GetAllPanes();
	for (int i = 0, Count = (int)AllPanes.GetCount(); i < Count; ++i)
	{
		wxAuiPaneInfo& Pane = AllPanes.Item(i);
		if (Pane.window->IsKindOf(CLASSINFO(wxAuiNotebook)))
		{
			wxAuiNotebook* NB = (wxAuiNotebook*)Pane.window;

			if (On)
				NB->SetWindowStyleFlag(NB->GetWindowStyleFlag() | Style);
			else
				NB->SetWindowStyleFlag(NB->GetWindowStyleFlag() &~ Style);

			NB->Refresh();
		}
	}
}

void CFrame::OnSelectPerspective(wxCommandEvent& event)
{
	u32 _Selection = event.GetId() - IDM_PERSPECTIVES_0;
	if (Perspectives.size() <= _Selection) _Selection = 0;
	ActivePerspective = _Selection;
	DoLoadPerspective();
}

void CFrame::ResizeConsole()
{
#ifdef _WIN32
	// Get the console parent window
	wxWindow * Win = FindWindowById(IDM_CONSOLEWINDOW);
	if (!Win) return;

	// Get OS version
	int wxBorder, Border, LowerBorder, MenuBar, ScrollBar, WidthReduction;
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwMajorVersion == 6) // Vista (same as 7?)
	{
		wxBorder = 2;
		Border = 4;
		LowerBorder = 6;
		MenuBar = 30; // Including upper border
		ScrollBar = 19;
	}
	else // XP
	{
		wxBorder = 2;
		Border = 4;
		LowerBorder = 6;
		MenuBar = 30;
		ScrollBar = 19;
	}
	WidthReduction = 30 - Border;

	// Get the client size
	int X = Win->GetClientSize().GetX();
	int Y = Win->GetClientSize().GetY();
	int InternalWidth = X - wxBorder*2 - ScrollBar;
	int InternalHeight = Y - wxBorder*2;
	int WindowWidth = InternalWidth + Border*2 + /*max out the width in the word wrap mode*/ 100;
	int WindowHeight = InternalHeight + MenuBar;
	// Resize buffer
	ConsoleListener* Console = LogManager::GetInstance()->getConsoleListener();
	Console->PixelSpace(0,0, InternalWidth,InternalHeight, false);
	// Move the window to hide the border
	MoveWindow(GetConsoleWindow(), -Border-wxBorder,-MenuBar-wxBorder, WindowWidth + 100,WindowHeight, true);
	// Move it to the bottom of the view order so that it doesn't hide the notebook tabs
	// ...
	// Log
	//NOTICE_LOG(CONSOLE, "Size: %ix%i", X, Y);
#endif
}

static int Limit(int i, int Low, int High)
{
	if (i < Low) return Low;
	if (i > High) return High;
	return i;
}

void CFrame::SetPaneSize()
{
	if (Perspectives.size() <= ActivePerspective) return;
	int iClientX = GetSize().GetX(), iClientY = GetSize().GetY();

	for (u32 i = 0, j = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (!m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiToolBar)))
		{
			if (!m_Mgr->GetAllPanes().Item(i).IsOk()) return;
			if (Perspectives[ActivePerspective].Width.size() <= j ||
				   	Perspectives[ActivePerspective].Height.size() <= j)
			   	continue;
			u32 W = Perspectives[ActivePerspective].Width[j],
			   	H = Perspectives[ActivePerspective].Height[j];
			// Check limits
			W = Limit(W, 5, 95);
		   	H = Limit(H, 5, 95);
			// Convert percentages to pixel lengths
			W = (W * iClientX) / 100;
		   	H = (H * iClientY) / 100;
			m_Mgr->GetAllPanes().Item(i).BestSize(W,H).MinSize(W,H);

			j++;
		}
	}
	m_Mgr->Update();

	for (u32 i = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (!m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiToolBar)))
		{
			m_Mgr->GetAllPanes().Item(i).MinSize(-1,-1);
		}
	}
}

void CFrame::ReloadPanes()
{
	// Hide to avoid flickering
	HideAllNotebooks(true);
	// Close all pages
	ClosePages();

	CloseAllNotebooks();

	// Create new panes with notebooks
	for (u32 i = 0; i < Perspectives[ActivePerspective].Width.size() - 1; i++)
	{
		wxString PaneName = wxString::Format(wxT("Pane %i"), i + 1);
		m_Mgr->AddPane(CreateEmptyNotebook(), wxAuiPaneInfo().Hide()
				.CaptionVisible(m_bEdit).Dockable(!m_bNoDocking).Position(i)
				.Name(PaneName).Caption(PaneName));
	}
	HideAllNotebooks(true);

	// Perspectives
	m_Mgr->LoadPerspective(Perspectives[ActivePerspective].Perspective, false);
	// Reset toolbars
	ResetToolbarStyle();
	// Restore settings
	TogglePaneStyle(m_bNoDocking, IDM_NO_DOCKING);
	TogglePaneStyle(m_bEdit, IDM_EDIT_PERSPECTIVES);

	// Load GUI settings
	g_pCodeWindow->Load();
	// Open notebook pages
	AddRemoveBlankPage();
	g_pCodeWindow->OpenPages();
	if (g_pCodeWindow->bShowOnStart[0]) ToggleLogWindow(true);
	if (g_pCodeWindow->bShowOnStart[1]) ToggleConsole(true);
}

void CFrame::DoLoadPerspective()
{
	ReloadPanes();
	// Restore the exact window sizes, which LoadPerspective doesn't always do
	SetPaneSize();
	// Show
	ShowAllNotebooks(true);
}

// Update the local perspectives array
void CFrame::LoadIniPerspectives()
{
	Perspectives.clear();
	std::vector<std::string> VPerspectives;
	std::string _Perspectives;

	IniFile ini;
	ini.Load(File::GetUserPath(F_DEBUGGERCONFIG_IDX));
	ini.Get("Perspectives", "Perspectives", &_Perspectives, "Perspective 1");
	ini.Get("Perspectives", "Active", &ActivePerspective, 0);
	SplitString(_Perspectives, ",", VPerspectives);

	for (u32 i = 0; i < VPerspectives.size(); i++)
	{
		SPerspectives Tmp;
		std::string _Section, _Perspective, _Width, _Height;
		std::vector<std::string> _SWidth, _SHeight;
		Tmp.Name = VPerspectives[i];
		// Don't save a blank perspective
		if (Tmp.Name.empty()) continue;

		_Section = StringFromFormat("P - %s", Tmp.Name.c_str());
		ini.Get(_Section.c_str(), "Perspective", &_Perspective,
			   	"layout2|"
				"name=Pane 0;caption=Pane 0;state=768;dir=5;prop=100000;|"
				"name=Pane 1;caption=Pane 1;state=31458108;dir=4;prop=100000;|"
				"dock_size(5,0,0)=22|dock_size(4,0,0)=333|");
		ini.Get(_Section.c_str(), "Width", &_Width, "70,25");
		ini.Get(_Section.c_str(), "Height", &_Height, "80,80");

		Tmp.Perspective = wxString::FromAscii(_Perspective.c_str());

		SplitString(_Width, ",", _SWidth);
		SplitString(_Height, ",", _SHeight);
		for (u32 j = 0; j < _SWidth.size(); j++)
		{
			int _Tmp;
			if (TryParseInt(_SWidth[j].c_str(), &_Tmp)) Tmp.Width.push_back(_Tmp);
		}
		for (u32 j = 0; j < _SHeight.size(); j++)
		{
			int _Tmp;
			if (TryParseInt(_SHeight[j].c_str(), &_Tmp)) Tmp.Height.push_back(_Tmp);
		}
		Perspectives.push_back(Tmp);
	}
}

void CFrame::UpdateCurrentPerspective()
{
	SPerspectives *current = &Perspectives[ActivePerspective];
	current->Perspective = m_Mgr->SavePerspective();

	// Get client size
	int iClientX = GetSize().GetX(), iClientY = GetSize().GetY();
	current->Width.clear();
	current->Height.clear();
	for (size_t i = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (!m_Mgr->GetAllPanes().Item(i).window->
				IsKindOf(CLASSINFO(wxAuiToolBar)))
		{
			// Save width and height as a percentage of the client width and height
			current->Width.push_back(
					(m_Mgr->GetAllPanes().Item(i).window->GetClientSize().GetX() * 100) /
				   	iClientX);
			current->Height.push_back(
					(m_Mgr->GetAllPanes().Item(i).window->GetClientSize().GetY() * 100) /
					iClientY);
		}
	}
}

void CFrame::SaveIniPerspectives()
{
	if (Perspectives.size() == 0) return;
	if (ActivePerspective >= Perspectives.size()) ActivePerspective = 0;

	// Turn off edit before saving
	TogglePaneStyle(false, IDM_EDIT_PERSPECTIVES);

	UpdateCurrentPerspective();

	IniFile ini;
	ini.Load(File::GetUserPath(F_DEBUGGERCONFIG_IDX));

	// Save perspective names
	std::string STmp = "";
	for (u32 i = 0; i < Perspectives.size(); i++)
	{
		STmp += Perspectives[i].Name + ",";
	}
	STmp = STmp.substr(0, STmp.length()-1);
	ini.Set("Perspectives", "Perspectives", STmp.c_str());
	ini.Set("Perspectives", "Active", ActivePerspective);

	// Save the perspectives
	for (size_t i = 0; i < Perspectives.size(); i++)
	{
		std::string _Section = "P - " + Perspectives[i].Name;
		ini.Set(_Section.c_str(), "Perspective", Perspectives[i].Perspective.mb_str());

		std::string SWidth = "", SHeight = "";
		for (size_t j = 0; j < Perspectives[i].Width.size(); j++)
		{
			SWidth += StringFromFormat("%i,", Perspectives[i].Width[j]);
			SHeight += StringFromFormat("%i,", Perspectives[i].Height[j]);
		}
		// Remove the ending ","
		SWidth = SWidth.substr(0, SWidth.length()-1);
	   	SHeight = SHeight.substr(0, SHeight.length()-1);

		ini.Set(_Section.c_str(), "Width", SWidth.c_str());
		ini.Set(_Section.c_str(), "Height", SHeight.c_str());
	}

	ini.Save(File::GetUserPath(F_DEBUGGERCONFIG_IDX));

	// Save notebook affiliations
	g_pCodeWindow->Save();

	TogglePaneStyle(m_bEdit, IDM_EDIT_PERSPECTIVES);
}

void CFrame::AddPane()
{
	int PaneNum = GetNotebookCount() + 1;
	wxString PaneName = wxString::Format(wxT("Pane %i"), PaneNum);
	m_Mgr->AddPane(CreateEmptyNotebook(), wxAuiPaneInfo()
		.CaptionVisible(m_bEdit).Dockable(!m_bNoDocking)
		.Name(PaneName).Caption(PaneName)
		.Position(GetNotebookCount()));

	AddRemoveBlankPage();
	m_Mgr->Update();
}

wxWindow * CFrame::GetNotebookPageFromId(wxWindowID Id)
{
	for (u32 i = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (!m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook))) continue;
		wxAuiNotebook * NB = (wxAuiNotebook*)m_Mgr->GetAllPanes().Item(i).window;
		for(u32 j = 0; j < NB->GetPageCount(); j++)
		{
			if (NB->GetPage(j)->GetId() == Id) return NB->GetPage(j);
		}
	}
	return NULL;
}

wxFrame * CFrame::CreateParentFrame(wxWindowID Id, const wxString& Title, wxWindow * Child)
{
	wxFrame * Frame = new wxFrame(this, Id, Title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);

	Child->Reparent(Frame);
	Child->Show();

	wxBoxSizer * m_MainSizer = new wxBoxSizer(wxHORIZONTAL);

	m_MainSizer->Add(Child, 1, wxEXPAND);

	Frame->Connect(wxID_ANY, wxEVT_CLOSE_WINDOW,
		wxCloseEventHandler(CFrame::OnFloatingPageClosed),
		(wxObject*)0, this);

	if (Id == IDM_CONSOLEWINDOW_PARENT)
	{
		Frame->Connect(wxID_ANY, wxEVT_SIZE,
			wxSizeEventHandler(CFrame::OnFloatingPageSize),
			(wxObject*)0, this);
	}

	// Main sizer
	Frame->SetSizer( m_MainSizer );
	// Minimum frame size
	Frame->SetMinSize(wxSize(200, -1));
	Frame->Show();
	return Frame;
}

wxAuiNotebook* CFrame::CreateEmptyNotebook()
{
	long NOTEBOOK_STYLE = wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_EXTERNAL_MOVE
		| wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER;
	wxAuiNotebook* NB = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, NOTEBOOK_STYLE);
	return NB;
}

void CFrame::AddRemoveBlankPage()
{
	for (u32 i = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (!m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook))) continue;
		wxAuiNotebook * NB = (wxAuiNotebook*)m_Mgr->GetAllPanes().Item(i).window;
		for(u32 j = 0; j < NB->GetPageCount(); j++)
		{
			if (NB->GetPageText(j).IsSameAs(wxT("<>")) && NB->GetPageCount() > 1) NB->DeletePage(j);
		}
		if (NB->GetPageCount() == 0) NB->AddPage(new wxPanel(this, wxID_ANY), wxT("<>"), true);
	}
}

int CFrame::GetNotebookAffiliation(wxWindowID Id)
{
	for (u32 i = 0, j = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (!m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook))) continue;
		wxAuiNotebook * NB = (wxAuiNotebook*)m_Mgr->GetAllPanes().Item(i).window;
		for(u32 k = 0; k < NB->GetPageCount(); k++)
		{
			if (NB->GetPage(k)->GetId() == Id) return j;
		}
		j++;
	}
	return -1;
}

// Close all panes with notebooks
void CFrame::CloseAllNotebooks()
{
	int i = 0;
	while(GetNotebookCount() > 0)
	{
		if (m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook)))
		{
			m_Mgr->GetAllPanes().Item(i).DestroyOnClose(true);
			m_Mgr->ClosePane(m_Mgr->GetAllPanes().Item(i));
			//m_Mgr->GetAllPanes().Item(i).window->Hide();
			//m_Mgr->DetachPane(m_Mgr->GetAllPanes().Item(i).window);

			i = 0;
		}
		else
			i++;
	}
}

int CFrame::GetNotebookCount()
{
	int Ret = 0;
	for (u32 i = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook))) Ret++;
	}
	return Ret;
}

wxAuiNotebook * CFrame::GetNotebookFromId(u32 NBId)
{
	for (u32 i = 0, j = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (!m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook))) continue;
		if (j == NBId) return (wxAuiNotebook*)m_Mgr->GetAllPanes().Item(i).window;
		j++;
	}
	return NULL;
}

void CFrame::ShowAllNotebooks(bool bShow)
{
	for (u32 i = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook)))
		{
			if (bShow)
				m_Mgr->GetAllPanes().Item(i).Show();
			else
				m_Mgr->GetAllPanes().Item(i).window->Hide();
		}
	}
	m_Mgr->Update();
}

void CFrame::HideAllNotebooks(bool Window)
{
	for (u32 i = 0; i < m_Mgr->GetAllPanes().GetCount(); i++)
	{
		if (m_Mgr->GetAllPanes().Item(i).window->IsKindOf(CLASSINFO(wxAuiNotebook)))
		{
			if (Window)
				m_Mgr->GetAllPanes().Item(i).Hide();
			else
				m_Mgr->GetAllPanes().Item(i).window->Hide();
		}
	}
	m_Mgr->Update();
}
