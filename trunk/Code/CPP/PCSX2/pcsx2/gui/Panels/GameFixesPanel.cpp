/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
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
#include "ConfigurationPanels.h"

Panels::GameFixesPanel::GameFixesPanel( wxWindow* parent ) :
	BaseApplicableConfigPanel( parent )
{
        	
	*this	+= new pxStaticHeading( this, _("一些游戏需要特殊设置.\n在这里启用它们."));

	wxStaticBoxSizer& groupSizer = *new wxStaticBoxSizer( wxVERTICAL, this, _("PCSX2 游戏修正") );
	
	// NOTE: Order of checkboxes must match the order of the bits in the GamefixOptions structure!
	// NOTE2: Don't make this static, because translations can change at run-time :)

	struct CheckTextMess
	{
		wxString label, tooltip;
	};

	const CheckTextMess check_text[NUM_OF_GAME_FIXES] = 
	{
		{
			_("VU 添加破解 - 三个王牌游戏!"),
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
			_("DMA 执行破解 - for Fatal Frame(致命构架)."),
			wxEmptyString
		},
		{
			_("VU XGkick 破解 - for EREMENTAR GERAD(武器种族传说)."),
			wxEmptyString
		}
	};

	for( int i=0; i<NUM_OF_GAME_FIXES; ++i )
	{
		groupSizer += (m_checkbox[i] = new pxCheckBox( this, check_text[i].label ));
		m_checkbox[i]->SetToolTip( check_text[i].tooltip );
	}
	
	m_check_Enable = new pxCheckBox( this, _("Enable game fixes"),
		_("(Warning, can cause compatibility or performance issues!)"));
	m_check_Enable->SetToolTip(_("The safest way to make sure that all game fixes are completely disabled."));
	m_check_Enable		->SetValue( g_Conf->EnableGameFixes );
	
	*this	+= groupSizer	| wxSF.Centre();

	*this	+= m_check_Enable;
	*this	+= new pxStaticHeading( this, pxE( ".Panels:Gamefixes:Compat Warning",
		L"Enabling game fixes can cause compatibility or performance issues in other games.  You "
		L"will need to turn off fixes manually when changing games."
	));
	Connect( m_check_Enable->GetId(),		wxEVT_COMMAND_CHECKBOX_CLICKED,	wxCommandEventHandler( GameFixesPanel::OnEnable_Toggled ) );
	EnableStuff();
	
	AppStatusEvent_OnSettingsApplied();
}

// I could still probably get rid of the for loop, but I think this is clearer.
void Panels::GameFixesPanel::Apply()
{
	g_Conf->EnableGameFixes = m_check_Enable->GetValue();
	
	Pcsx2Config::GamefixOptions& opts( g_Conf->EmuOptions.Gamefixes );
    for (int i = 0; i < NUM_OF_GAME_FIXES; i++)
    {
        if (m_checkbox[i]->GetValue())
        {
            opts.bitset |= (1 << i);
        }
        else
        {
            opts.bitset &= ~(1 << i);
        }
    }
}

void Panels::GameFixesPanel::EnableStuff()
{
    for (int i = 0; i < NUM_OF_GAME_FIXES; i++)
    {
    	m_checkbox[i]->Enable(m_check_Enable->GetValue());
    }
}

void Panels::GameFixesPanel::OnEnable_Toggled( wxCommandEvent& evt )
{
	EnableStuff();
	evt.Skip();
}

void Panels::GameFixesPanel::AppStatusEvent_OnSettingsApplied()
{
	const Pcsx2Config::GamefixOptions& opts( g_Conf->EmuOptions.Gamefixes );
	for( int i=0; i<NUM_OF_GAME_FIXES; ++i )
	{
		m_checkbox[i]->SetValue( !!(opts.bitset & (1 << i)) );
	}
}
