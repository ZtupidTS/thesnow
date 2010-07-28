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

#ifndef _OGL_CONFIGDIALOG_H_
#define _OGL_CONFIGDIALOG_H_

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/filepicker.h>
#include <wx/gbsizer.h>

enum
{
	OGL_HACK_NONE = 0,
	OGL_HACK_ZELDA_TP_BLOOM_HACK = 1,
	OGL_HACK_SONIC_AND_THE_BLACK_KNIGHT = 2,
	OGL_HACK_BLEACH_VERSUS_CRUSADE = 3,
	OGL_HACK_SKIES_OF_ARCADIA = 4
};


class GFXConfigDialogOGL : public wxDialog
{
	public:
		GFXConfigDialogOGL(wxWindow *parent, wxWindowID id = wxID_ANY,
#ifdef DEBUGFAST
			const wxString &title = wxT("OpenGL (DEBUGFAST) Plugin Configuration"),
#else
#ifndef _DEBUG
			const wxString &title = wxT("OpenGL Plugin Configuration"),
#else
			const wxString &title = wxT("OpenGL (DEBUG) Plugin Configuration"),
#endif
#endif
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxDEFAULT_DIALOG_STYLE);
		virtual ~GFXConfigDialogOGL();
		void CloseClick(wxCommandEvent& event);

		void CreateGUIControls();
		void GameIniLoad();

	private:
		DECLARE_EVENT_TABLE();

		wxBoxSizer* sGeneral;
		wxStaticBoxSizer* sbBasic, *sbBasicAdvanced;
		wxGridBagSizer* sBasic, *sBasicAdvanced;
		wxStaticBoxSizer* sbEnhancements;
		wxGridBagSizer* sEnhancements;
		wxBoxSizer* sAdvanced;
		wxStaticBoxSizer* sbInfo;
		wxGridBagSizer* sInfo;
		wxStaticBoxSizer* sbRendering;
		wxGridBagSizer* sRendering;
		wxStaticBoxSizer* sbUtilities;
		wxGridBagSizer* sUtilities;
		wxStaticBoxSizer* sHacks;
		wxStaticBoxSizer* sbHacks;
		
		wxButton *m_About;
		wxButton *m_Close;
		wxButton *m_ReloadShader;
		wxButton *m_EditShader;

		wxNotebook *m_Notebook;
		wxPanel *m_PageGeneral;
		wxPanel *m_PageAdvanced;
		wxCheckBox *m_VSync;
		wxCheckBox *m_NativeResolution, *m_2xResolution;
		wxCheckBox *m_WidescreenHack;
		wxCheckBox *m_ForceFiltering;
		wxCheckBox *m_Crop;
		wxCheckBox *m_UseXFB;
		wxCheckBox *m_UseNativeMips;
		wxCheckBox *m_EFBScaledCopy;
		wxCheckBox *m_UseRealXFB;
		wxCheckBox *m_AutoScale;
		wxChoice *m_MaxAnisotropyCB;
		wxChoice *m_MSAAModeCB, *m_PhackvalueCB, *m_PostShaderCB, *m_KeepAR;

		wxCheckBox *m_ShowFPS;
		wxCheckBox *m_ShaderErrors;
		wxCheckBox *m_Statistics;
		wxCheckBox *m_ProjStats;
		wxCheckBox *m_ShowEFBCopyRegions;
		wxCheckBox *m_TexFmtOverlay;
		wxCheckBox *m_TexFmtCenter;
		wxCheckBox *m_Wireframe;
		wxCheckBox *m_DisableLighting;
		wxCheckBox *m_DisableTexturing;
		wxCheckBox *m_DisableFog;
		wxCheckBox *m_DstAlphaPass;
		wxCheckBox *m_DumpTextures;
		wxCheckBox *m_HiresTextures;
		wxCheckBox *m_DumpEFBTarget;
		wxCheckBox *m_DumpFrames;
		wxCheckBox *m_FreeLook;
		wxStaticBox * m_StaticBox_EFB;
		wxCheckBox *m_CheckBox_DisableCopyEFB;
		wxRadioButton *m_Radio_CopyEFBToRAM, *m_Radio_CopyEFBToGL;
		wxCheckBox *m_OSDHotKey;
		wxCheckBox *m_Hack;
		wxCheckBox *m_SafeTextureCache;
		wxRadioButton *m_Radio_SafeTextureCache_Safe;
		wxRadioButton *m_Radio_SafeTextureCache_Normal;
		wxRadioButton *m_Radio_SafeTextureCache_Fast;
		// Screen size
		wxStaticText *m_TextScreenWidth, *m_TextScreenHeight, *m_TextScreenLeft, *m_TextScreenTop;
		wxSlider *m_SliderWidth, *m_SliderHeight, *m_SliderLeft, *m_SliderTop;
		wxCheckBox *m_ScreenSize;

		wxArrayString arrayStringFor_FullscreenCB;
		wxArrayString arrayStringFor_AspectRatio;
		wxArrayString arrayStringFor_MaxAnisotropyCB;
		wxArrayString arrayStringFor_MSAAModeCB;
		wxArrayString arrayStringFor_PhackvalueCB;
		wxArrayString arrayStringFor_PostShaderCB;

		enum
		{
			ID_NOTEBOOK = 1000,
			ID_PAGEGENERAL,
			ID_PAGEADVANCED,

			ID_VSYNC,
			ID_NATIVERESOLUTION, ID_2X_RESOLUTION,
			ID_ASPECT, 
			ID_CROP,
			ID_USEREALXFB,
			ID_USEXFB,
			ID_USENATIVEMIPS,
			ID_EFBSCALEDCOPY,
			ID_AUTOSCALE,
			ID_WIDESCREENHACK,

			ID_FORCEFILTERING,
			ID_MAXANISOTROPY,
			ID_MAXANISOTROPYTEXT,
			ID_MSAAMODECB,
			ID_MSAAMODETEXT,

			ID_SHOWFPS,
			ID_SHADERERRORS,
			ID_STATISTICS,
			ID_PROJSTATS,
			ID_SHOWEFBCOPYREGIONS,
			ID_TEXFMTOVERLAY,
			ID_TEXFMTCENTER,

			ID_WIREFRAME,
			ID_DISABLELIGHTING,
			ID_DISABLETEXTURING,
			ID_DISABLEFOG,
			ID_STATICBOX_EFB,
			ID_SAFETEXTURECACHE,
			ID_RADIO_SAFETEXTURECACHE_SAFE,
			ID_RADIO_SAFETEXTURECACHE_NORMAL,
			ID_RADIO_SAFETEXTURECACHE_FAST,
			ID_HACK,
			ID_PHACKVALUE,

			ID_DUMPTEXTURES,
			ID_HIRESTEXTURES,
			ID_DUMPEFBTARGET,
			ID_DUMPFRAMES,
			ID_FREELOOK,
			ID_TEXTUREPATH,

			ID_CHECKBOX_DISABLECOPYEFB, 
			ID_OSDHOTKEY,
			//ID_PROJECTIONHACK1,
			ID_DSTALPHAPASS,
			ID_RADIO_COPYEFBTORAM,
			ID_RADIO_COPYEFBTOGL,
			ID_POSTSHADER,
			ID_POSTSHADERTEXT,
			ID_RELOADSHADER,
			ID_EDITSHADER,
		};

		void LoadShaders();
		void InitializeGUILists();
		void InitializeGUIValues();
		void InitializeGUITooltips();

		void OnClose(wxCloseEvent& event);
		void UpdateGUI();
		void UpdateHack();

		void AboutClick(wxCommandEvent& event);
		void ReloadShaderClick(wxCommandEvent& event);
		void EditShaderClick(wxCommandEvent& event);
		void GeneralSettingsChanged(wxCommandEvent& event);
		void AdvancedSettingsChanged(wxCommandEvent& event);
		void CloseWindow();
};

#endif // _OGL_CONFIGDIALOG_H_
