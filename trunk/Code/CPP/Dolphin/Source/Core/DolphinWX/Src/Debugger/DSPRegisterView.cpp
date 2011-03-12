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

#include "DSPDebugWindow.h"
#include "DSPRegisterView.h"


wxString CDSPRegTable::GetValue(int row, int col)
{
	if (row < 32) // 32 "normal" regs
	{
		switch (col)
		{
		case 0: return wxString::FromAscii(pdregname(row));
		case 1: return wxString::Format(wxT("0x%04x"), DSPCore_ReadRegister(row));
		default: return wxString::FromAscii("");
		}
	}
	return wxString::FromAscii("");
}

void CDSPRegTable::SetValue(int, int, const wxString &)
{
}

void CDSPRegTable::UpdateCachedRegs()
{
	if (m_CachedCounter == g_dsp.step_counter)
	{
		return;
	}

	m_CachedCounter = g_dsp.step_counter;

	for (int i = 0; i < 32; ++i)
	{
		m_CachedRegHasChanged[i] = (m_CachedRegs[i] != DSPCore_ReadRegister(i));
		m_CachedRegs[i] = DSPCore_ReadRegister(i);
	}
}

wxGridCellAttr *CDSPRegTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind)
{
	wxGridCellAttr *attr = new wxGridCellAttr();

	attr->SetBackgroundColour(wxColour(wxT("#FFFFFF")));

	switch (col)
	{
	case 1:
		attr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
		break;
	default:
		attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER);
		break;
	}

	if (col == 1)
		attr->SetTextColour(m_CachedRegHasChanged[row] ? wxColor(wxT("#FF0000")) : wxColor(wxT("#000000")));

	attr->IncRef();
	return attr;
}

DSPRegisterView::DSPRegisterView(wxWindow *parent, wxWindowID id)
	: wxGrid(parent, id, wxDefaultPosition, wxSize(130, 120))
{
	SetTable(new CDSPRegTable(), true);
	SetRowLabelSize(0);
	SetColLabelSize(0);
	DisableDragRowSize();

	AutoSizeColumns();
}

void DSPRegisterView::Update()
{
	((CDSPRegTable *)GetTable())->UpdateCachedRegs();
	ForceRefresh();
}
