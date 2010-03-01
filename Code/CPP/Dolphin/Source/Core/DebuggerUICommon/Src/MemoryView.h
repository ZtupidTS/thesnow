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

#ifndef MEMORYVIEW_H_
#define MEMORYVIEW_H_

#include "DebuggerUIUtil.h"
#include "Common.h"
#include "DebugInterface.h"

class CMemoryView : public wxControl
{
public:
	CMemoryView(DebugInterface* debuginterface, wxWindow* parent, wxWindowID Id = -1, const wxSize& Size = wxDefaultSize);
	wxSize DoGetBestSize() const;
	void OnPaint(wxPaintEvent& event);
	void OnErase(wxEraseEvent& event);
	void OnMouseDownL(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseUpL(wxMouseEvent& event);
	void OnMouseDownR(wxMouseEvent& event);
	void OnPopupMenu(wxCommandEvent& event);

	u32 GetSelection() { return selection ; }
	int GetMemoryType() { return memory; }

	void Center(u32 addr)
	{
		curAddress = addr;
		redraw();
	}
	int dataType;	// u8,u16,u32
	int curAddress;	// Will be accessed by parent

private:
	int YToAddress(int y);
	void redraw() {Refresh();}

	DebugInterface* debugger;


	int align;
	int rowHeight;

	u32 selection;
	u32 oldSelection;
	bool selectionChanged;
	bool selecting;
	bool hasFocus;
	bool showHex;

	int memory;

	enum EViewAsType
	{
		VIEWAS_ASCII = 0,
		VIEWAS_FP,
		VIEWAS_HEX,
	};

	EViewAsType viewAsType;

	DECLARE_EVENT_TABLE()
};

#endif /*MEMORYVIEW_H_*/
