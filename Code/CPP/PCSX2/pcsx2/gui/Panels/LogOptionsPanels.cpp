/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "LogOptionsPanels.h"

#include "DebugTools/Debug.h"
#include <wx/statline.h>


using namespace pxSizerFlags;

Panels::eeLogOptionsPanel::eeLogOptionsPanel( LogOptionsPanel* parent )
	: CheckedStaticBox( parent, wxVERTICAL, L"EE ��־" )
{
	SetMinWidth( 260 );

	m_disasmPanel	= new CheckedStaticBox( this, wxVERTICAL, L"�����" );
	m_hwPanel		= new CheckedStaticBox( this, wxVERTICAL, L"Ӳ��" );
	m_evtPanel		= new CheckedStaticBox( this, wxVERTICAL, L"�¼�" );

	wxSizer& s_disasm	( m_disasmPanel->ThisSizer );
	wxSizer& s_hw		( m_hwPanel->ThisSizer );
	wxSizer& s_evt		( m_evtPanel->ThisSizer );

	wxStaticBoxSizer& s_misc = *new wxStaticBoxSizer( wxVERTICAL, this, L"����" );
	wxPanelWithHelpers* m_miscPanel = this;		// helper for our newCheckBox macro.

	s_misc.Add( m_Bios		= new pxCheckBox( m_miscPanel, L"Bios" ) );
	s_misc.Add( m_Memory	= new pxCheckBox( m_miscPanel, L"�ڴ�" ) );
	s_misc.Add( m_Cache		= new pxCheckBox( m_miscPanel, L"����" ));
	s_misc.Add( m_SysCtrl	= new pxCheckBox( m_miscPanel, L"SysCtrl / MMU" ) );

	s_disasm.Add( m_R5900		= new pxCheckBox( m_disasmPanel, L"R5900" ));
	s_disasm.Add( m_COP0		= new pxCheckBox( m_disasmPanel, L"COP0 (MMU/SysCtrl)" ));
	s_disasm.Add( m_COP1		= new pxCheckBox( m_disasmPanel, L"COP1 (FPU)" ));
	s_disasm.Add( m_COP2		= new pxCheckBox( m_disasmPanel, L"COP2 (VU0 macro)" ));
	s_disasm.Add( m_VU0micro	= new pxCheckBox( m_disasmPanel, L"VU0 micro" ));
	s_disasm.Add( m_VU1micro	= new pxCheckBox( m_disasmPanel, L"VU1 micro" ));

	s_hw.Add( m_KnownHw		= new pxCheckBox( m_hwPanel, L"�Ĵ���" ));
	s_hw.Add( m_UnknownHw	= new pxCheckBox( m_hwPanel, L"Unknown Regs" ));
	s_hw.Add( m_DMA			= new pxCheckBox( m_hwPanel, L"DMA" ));

	s_evt.Add( m_Counters	= new pxCheckBox( m_evtPanel, L"Counters" ));
	s_evt.Add( m_VIF		= new pxCheckBox( m_evtPanel, L"VIF" ));
	s_evt.Add( m_GIF		= new pxCheckBox( m_evtPanel, L"GIF" ));
	s_evt.Add( m_IPU		= new pxCheckBox( m_evtPanel, L"IPU" ));
	s_evt.Add( m_SPR		= new pxCheckBox( m_evtPanel, L"SPR" ));

	m_Cache->SetToolTip(_("(not implemented yet)"));

	wxFlexGridSizer& eeTable( *new wxFlexGridSizer( 2, 5 ) );

	eeTable.AddGrowableCol(0);
	eeTable.AddGrowableCol(1);

	eeTable	+= s_misc			| SubGroup();
	eeTable += m_hwPanel		| SubGroup();
	eeTable += m_evtPanel		| SubGroup();
	eeTable += m_disasmPanel	| SubGroup();

	ThisSizer	+= 4;
	ThisSizer	+= eeTable | pxExpand;

	SetValue( true );
}

Panels::iopLogOptionsPanel::iopLogOptionsPanel( LogOptionsPanel* parent )
	: CheckedStaticBox( parent, wxVERTICAL, L"IOP ��־" )
{
	SetMinWidth( 260 );
	
	m_disasmPanel	= new CheckedStaticBox( this, wxVERTICAL, L"�����" );
	m_hwPanel		= new CheckedStaticBox( this, wxVERTICAL, L"Ӳ��" );
	m_evtPanel		= new CheckedStaticBox( this, wxVERTICAL, L"�¼�" );

	wxSizer& s_disasm	( m_disasmPanel->ThisSizer );
	wxSizer& s_hw		( m_hwPanel->ThisSizer );
	wxSizer& s_evt		( m_evtPanel->ThisSizer );

	wxStaticBoxSizer& s_misc = *new wxStaticBoxSizer( wxVERTICAL, this, L"����" );
	wxPanelWithHelpers* m_miscPanel = this;		// helper for our newCheckBox macro.

	s_misc		+= m_Bios		= new pxCheckBox( m_miscPanel, L"Bios" );
	s_misc		+= m_Memory		= new pxCheckBox( m_miscPanel, L"�ڴ�" );
	s_misc		+= m_GPU		= new pxCheckBox( m_miscPanel, L"GPU (PS1 only)", L"(Not implemented yet)" );

	s_disasm	+= m_R3000A		= new pxCheckBox( m_disasmPanel, L"R3000A" );
	s_disasm	+= m_COP2		= new pxCheckBox( m_disasmPanel, L"COP2 (Geometry)" );

	s_hw		+= m_KnownHw	= new pxCheckBox( m_hwPanel, L"�Ĵ���" );
	s_hw		+= m_UnknownHw	= new pxCheckBox( m_hwPanel, L"UnknownRegs" );
	s_hw		+= m_DMA		= new pxCheckBox( m_hwPanel, L"DMA" );

	s_evt		+= m_Counters	= new pxCheckBox( m_evtPanel, L"Counters" );
	s_evt		+= m_Memcards	= new pxCheckBox( m_evtPanel, L"�ڴ濨" );
	s_evt		+= m_PAD		= new pxCheckBox( m_evtPanel, L"�ֱ�" );
	s_evt		+= m_SPU2		= new pxCheckBox( m_evtPanel, L"SPU2" );
	s_evt		+= m_CDVD		= new pxCheckBox( m_evtPanel, L"CDVD" );
	s_evt		+= m_USB		= new pxCheckBox( m_evtPanel, L"USB" );
	s_evt		+= m_FW			= new pxCheckBox( m_evtPanel, L"FW" );

	wxFlexGridSizer& iopTable( *new wxFlexGridSizer( 2, 5 ) );

	iopTable.AddGrowableCol(0);
	iopTable.AddGrowableCol(1);

	iopTable	+= s_misc			| SubGroup();
	iopTable	+= m_hwPanel		| SubGroup();
	iopTable	+= m_evtPanel		| SubGroup();
	iopTable	+= m_disasmPanel	| SubGroup();

	ThisSizer	+= 4;
	ThisSizer	+= iopTable	| pxExpand;

	SetValue( true );
}

#define SetCheckValue( cpu, key ) \
	if( m_##key != NULL ) m_##key->SetValue( conf.cpu.m_##key );

void Panels::eeLogOptionsPanel::OnSettingsChanged()
{
	const TraceLogFilters& conf( g_Conf->EmuOptions.Trace );

	SetValue( conf.EE.m_EnableAll );

	m_disasmPanel->SetValue( conf.EE.m_EnableDisasm );
	m_evtPanel->SetValue( conf.EE.m_EnableEvents );
	m_hwPanel->SetValue( conf.EE.m_EnableHardware );

	SetCheckValue( EE, Memory );
	SetCheckValue( EE, Bios );
	SetCheckValue( EE, Cache );
	SetCheckValue( EE, SysCtrl );

	SetCheckValue( EE, R5900 );
	SetCheckValue( EE, COP0 );
	SetCheckValue( EE, COP1 );
	SetCheckValue( EE, COP2 );

	SetCheckValue(EE, VU0micro);
	SetCheckValue(EE, VU1micro);

	SetCheckValue(EE, KnownHw);
	SetCheckValue(EE, UnknownHw);
	SetCheckValue(EE, DMA);

	SetCheckValue(EE, Counters);
	SetCheckValue(EE, VIF);
	SetCheckValue(EE, GIF);
	SetCheckValue(EE, SPR);
	SetCheckValue(EE, IPU);
}

void Panels::iopLogOptionsPanel::OnSettingsChanged()
{
	const TraceLogFilters& conf( g_Conf->EmuOptions.Trace );

	SetValue( conf.IOP.m_EnableAll );

	m_disasmPanel->SetValue( conf.IOP.m_EnableDisasm );
	m_evtPanel->SetValue( conf.IOP.m_EnableEvents );
	m_hwPanel->SetValue( conf.IOP.m_EnableHardware );

	SetCheckValue(IOP, Bios);
	SetCheckValue(IOP, Memory);

	SetCheckValue(IOP, R3000A);
	SetCheckValue(IOP, COP2);

	SetCheckValue(IOP, KnownHw);
	SetCheckValue(IOP, UnknownHw);
	SetCheckValue(IOP, DMA);

	SetCheckValue(IOP, Counters);
	SetCheckValue(IOP, Memcards);
	SetCheckValue(IOP, PAD);
	SetCheckValue(IOP, SPU2);
	SetCheckValue(IOP, USB);
	SetCheckValue(IOP, FW);
	SetCheckValue(IOP, CDVD);
}

// --------------------------------------------------------------------------------------
//  LogOptionsPanel Implementations
// --------------------------------------------------------------------------------------
Panels::LogOptionsPanel::LogOptionsPanel(wxWindow* parent )
	: BaseApplicableConfigPanel( parent )
	, m_eeSection	( *new eeLogOptionsPanel( this ) )
	, m_iopSection	( *new iopLogOptionsPanel( this ) )
{
	m_masterEnabler = new pxCheckBox( this, _("Enable Trace Logging"),
		_("Trace logs are all written to emulog.txt.  Toggle trace logging at any time using F10.") );
	m_masterEnabler->SetToolTip( _("Warning: Enabling trace logs is typically very slow, and is a leading cause of 'What happened to my FPS?' problems. :)") );

	m_SIF		= new pxCheckBox( this, L"SIF (EE<->IOP)" );
	m_VIFunpack	= new pxCheckBox( this, L"VIFunpack" );
	m_GIFtag	= new pxCheckBox( this, L"GIFtag" );
	m_Elf		= new pxCheckBox( this, L"Elves" );

	m_SIF		->SetToolTip(_("Enables logging of both SIF DMAs and SIF Register activity.") );
	m_VIFunpack	->SetToolTip(_("Special detailed logs of VIF packed data handling (does not include VIF control, status, or hwRegs)"));
	m_GIFtag	->SetToolTip(_("(not implemented yet)"));
	m_Elf		->SetToolTip(_("Logging of Elf headers."));


	wxFlexGridSizer&	topSizer	= *new wxFlexGridSizer( 2 );
	wxStaticBoxSizer&	s_misc		= *new wxStaticBoxSizer( wxHORIZONTAL, this, L"Misc" );

	topSizer.AddGrowableCol(0);
	topSizer.AddGrowableCol(1);

	topSizer	+= m_eeSection		| StdExpand();
	topSizer	+= m_iopSection		| StdExpand();

	s_misc		+= m_SIF;
	s_misc		+= m_VIFunpack;
	s_misc		+= m_GIFtag;
	s_misc		+= m_Elf;

	*this		+= m_masterEnabler				| StdExpand();
	*this		+= new wxStaticLine( this )		| StdExpand().Border(wxLEFT | wxRIGHT, 20);
	*this		+= 5;
	*this		+= topSizer						| StdExpand();
	*this		+= s_misc						| StdSpace().Centre();

	Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(LogOptionsPanel::OnCheckBoxClicked) );

	AppStatusEvent_OnSettingsApplied();
}

void Panels::LogOptionsPanel::AppStatusEvent_OnSettingsApplied()
{
	TraceLogFilters& conf( g_Conf->EmuOptions.Trace );

	m_masterEnabler->SetValue( conf.Enabled );
	m_SIF->SetValue( conf.SIF );
	m_Elf->SetValue( g_Conf->EmuOptions.Log.ELF );

	SetCheckValue( EE, VIFunpack );
	SetCheckValue( EE, GIFtag );

	m_eeSection.OnSettingsChanged();
	m_iopSection.OnSettingsChanged();

	OnUpdateEnableAll();
}

void Panels::LogOptionsPanel::OnUpdateEnableAll()
{
	bool enabled( m_masterEnabler->GetValue() );

	m_SIF->Enable( enabled );
	m_Elf->Enable( enabled );
	m_VIFunpack->Enable( enabled );
	m_GIFtag->Enable( enabled );

	m_eeSection.Enable( enabled );
	m_iopSection.Enable( enabled );
}

void Panels::LogOptionsPanel::OnCheckBoxClicked(wxCommandEvent &evt)
{
	m_IsDirty = true;
	if( evt.GetId() == m_masterEnabler->GetWxPtr()->GetId() )
		OnUpdateEnableAll();
}

void Panels::LogOptionsPanel::Apply()
{
	if( !m_IsDirty ) return;

	g_Conf->EmuOptions.Trace.Enabled	= m_masterEnabler->GetValue();
	g_Conf->EmuOptions.Trace.SIF		= m_SIF->GetValue();
	g_Conf->EmuOptions.Log.ELF			= m_Elf->GetValue();

	g_Conf->EmuOptions.Trace.EE.m_VIFunpack	= m_VIFunpack->GetValue();
	g_Conf->EmuOptions.Trace.EE.m_GIFtag	= m_GIFtag->GetValue();

	m_eeSection.Apply();
	m_iopSection.Apply();

	m_IsDirty = false;
}

#define GetSet( name )		conf.name	= name->GetValue()

void Panels::eeLogOptionsPanel::Apply()
{
	TraceFiltersEE& conf( g_Conf->EmuOptions.Trace.EE );

	conf.m_EnableAll		= GetValue();
	conf.m_EnableDisasm		= m_disasmPanel->GetValue();
	conf.m_EnableHardware	= m_hwPanel->GetValue();
	conf.m_EnableEvents		= m_evtPanel->GetValue();

	GetSet(m_Bios);
	GetSet(m_Memory);
	GetSet(m_Cache);
	GetSet(m_SysCtrl);

	GetSet(m_R5900);
	GetSet(m_COP0);
	GetSet(m_COP1);
	GetSet(m_COP2);
	GetSet(m_VU0micro);
	GetSet(m_VU1micro);

	GetSet(m_KnownHw);
	GetSet(m_UnknownHw);
	GetSet(m_DMA);

	GetSet(m_Counters);
	GetSet(m_VIF);
	GetSet(m_GIF);
	GetSet(m_SPR);
	GetSet(m_IPU);
}

void Panels::iopLogOptionsPanel::Apply()
{
	TraceFiltersIOP& conf( g_Conf->EmuOptions.Trace.IOP );

	conf.m_EnableAll		= GetValue();
	conf.m_EnableDisasm		= m_disasmPanel->GetValue();
	conf.m_EnableHardware	= m_hwPanel->GetValue();
	conf.m_EnableEvents		= m_evtPanel->GetValue();

	GetSet(m_Bios);
	GetSet(m_Memory);

	GetSet(m_R3000A);
	GetSet(m_COP2);

	GetSet(m_KnownHw);
	GetSet(m_UnknownHw);
	GetSet(m_DMA);

	GetSet(m_Counters);
	GetSet(m_Memcards);
	GetSet(m_PAD);
	GetSet(m_SPU2);
	GetSet(m_USB);
	GetSet(m_FW);
	GetSet(m_CDVD);
}

