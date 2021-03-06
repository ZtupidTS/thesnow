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
#include "App.h"
#include "ConfigurationPanels.h"

using namespace pxSizerFlags;

Panels::GameFixesPanel::GameFixesPanel( wxWindow* parent )
	: BaseApplicableConfigPanel_SpecificConfig( parent )
{
	wxStaticBoxSizer& groupSizer = *new wxStaticBoxSizer( wxVERTICAL, this, _("游戏修正") );

	// NOTE: Order of checkboxes must match the order of the bits in the GamefixOptions structure!
	// NOTE2: Don't make this static, because translations can change at run-time :)

	struct CheckTextMess
	{
		wxString label, tooltip;
	};

	const CheckTextMess check_text[GamefixId_COUNT] =
	{
		{
			_("VU 添加破解 - 修正三个王牌游戏启动崩溃 (启用 IPU 破解.)"),
			_("游戏需要这些破解才能启动:\n * Star Ocean 3(海洋之心3)\n * Radiata Stories(凡人物语)\n * Valkyrie Profile 2(北欧女神2)")
		},
		{
			_("VU Clip Flag 破解 - for Persona games (只能 SuperVU 重编译器使用!)"),
			wxEmptyString
		},
		{
			_("FPU 比较破解 - for Digimon Rumble Arena 2(数码宝贝竞技场2)."),
			wxEmptyString
		},
		{
			_("FPU 乘法破解 - for Tales of Destiny(宿命传说)."),
			wxEmptyString
		},
		{
			_("FPU Negative Div 破解 - for Gundam games.(高达游戏)"),
			wxEmptyString
		},
		{
			_("VU XGkick 破解 - for Erementar Gerad(武器种族传说)."),
			wxEmptyString
		},
		{
			_("FFX 视频修正 - Fixes bad graphics overlay in FFX videos."),
			wxEmptyString
		},
		{
			_("EE timing hack - Multi purpose hack. Try if all else fails."),
			pxEt( "!ContextTip:Gamefixes:EE Timing Hack",
				L"Known to affect following games:\n"
				L" * Digital Devil Saga (Fixes FMV and crashes)\n"
				L" * SSX (Fixes bad graphics and crashes)\n"
				L" * Resident Evil: Dead Aim (Causes garbled textures)"
			)
		},
		{
			_("Skip MPEG hack - Skips videos/FMVs in games to avoid game hanging/freezes."),
			wxEmptyString
		},
		{
			_("OPH Flag hack - Try if your game freezes showing the same frame."),
			pxEt( "!ContextTip:Gamefixes:OPH Flag hack",
				L"Known to affect following games:\n"
				L" * Bleach Blade Battler\n"
				L" * Growlanser II and III\n"
				L" * Wizardry"
			)
		},
		{
			_("Ignore DMAC writes when it is busy."),
			pxEt( "!ContextTip:Gamefixes:DMA Busy hack",
				L"Known to affect following games:\n"
				L" * Mana Khemia 1 (Going \"off campus\")\n"
			)
		},
		{
			_("Simulate VIF1 FIFO read ahead. Fixes slow loading games."),
			pxEt( "!ContextTip:Gamefixes:VIF1 FIFO hack",
				L"Known to affect following games:\n"
				L" * Test Drive Unlimited\n"
				L" * Transformers"
			)
		}
	};

	for( int i=0; i<GamefixId_COUNT; ++i )
	{
		groupSizer += (m_checkbox[i] = new pxCheckBox( this, check_text[i].label ));
		m_checkbox[i]->SetToolTip( check_text[i].tooltip );
	}

	m_check_Enable = new pxCheckBox( this, _("启用游戏修正 [不推荐]"),
		pxE( "!Panel:Gamefixes:Compat Warning",
			L"Gamefixes can work around wrong emulation in some titles. \n"
			L"They may also cause compatibility or performance issues. \n\n"
			L"It's better to enable 'Automatic game fixes' at the main menu instead, and leave this page empty. \n"
			L"('Automatic' means: selectively use specific tested fixes for specific games)"
		)
	);

	m_check_Enable->SetToolTip(_("The safest way to make sure that all game fixes are completely disabled.")).SetSubPadding( 1 );
	m_check_Enable->SetValue( g_Conf->EnableGameFixes );

	*this	+= m_check_Enable	| StdExpand();
	*this	+= groupSizer		| pxCenter;


	Connect( m_check_Enable->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GameFixesPanel::OnEnable_Toggled ) );

	EnableStuff();
}

void Panels::GameFixesPanel::Apply()
{
	g_Conf->EnableGameFixes = m_check_Enable->GetValue();

	Pcsx2Config::GamefixOptions& opts( g_Conf->EmuOptions.Gamefixes );
	for (GamefixId i=GamefixId_FIRST; i < pxEnumEnd; ++i)
		opts.Set((GamefixId)i, m_checkbox[i]->GetValue());

	// make sure the user's command line specifications are disabled (if present).
	wxGetApp().Overrides.ApplyCustomGamefixes = false;
}

void Panels::GameFixesPanel::EnableStuff( AppConfig* configToUse )
{
	if( !configToUse ) configToUse = g_Conf;
	for (GamefixId i=GamefixId_FIRST; i < pxEnumEnd; ++i)
		m_checkbox[i]->Enable(m_check_Enable->GetValue() && !configToUse->EnablePresets);
}

void Panels::GameFixesPanel::OnEnable_Toggled( wxCommandEvent& evt )
{
	AppConfig tmp=*g_Conf;
	tmp.EnablePresets=false; //if clicked, button was enabled, so not using a preset --> let EnableStuff work

	EnableStuff( &tmp );
	evt.Skip();
}

void Panels::GameFixesPanel::AppStatusEvent_OnSettingsApplied()
{
	ApplyConfigToGui( *g_Conf );
}

void Panels::GameFixesPanel::ApplyConfigToGui( AppConfig& configToApply, int flags )
{
	const Pcsx2Config::GamefixOptions& opts( configToApply.EmuOptions.Gamefixes );
	for (GamefixId i=GamefixId_FIRST; i < pxEnumEnd; ++i)
		m_checkbox[i]->SetValue( opts.Get((GamefixId)i) );//apply the use/don't-use fix values
	
	m_check_Enable->SetValue( configToApply.EnableGameFixes );//main gamefixes checkbox
	EnableStuff( &configToApply );// enable/disable the all the fixes controls according to the main one
	
	this->Enable(!configToApply.EnablePresets);
}
