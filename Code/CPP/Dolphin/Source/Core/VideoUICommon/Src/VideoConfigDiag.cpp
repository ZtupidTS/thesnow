
#include "VideoConfigDiag.h"

#include "FileUtil.h"

#define _connect_macro_(b, f, c, s)	(b)->Connect(wxID_ANY, (c), wxCommandEventHandler( f ), (wxObject*)0, (wxEvtHandler*)s)

// template instantiation
template class BoolSetting<wxCheckBox>;
template class BoolSetting<wxRadioButton>;

template <>
SettingCheckBox::BoolSetting(wxWindow* parent, const wxString& label, const wxString& tooltip, bool &setting, bool reverse, long style)
	: wxCheckBox(parent, -1, label, wxDefaultPosition, wxDefaultSize, style)
	, m_setting(setting)
	, m_reverse(reverse)
{
	SetToolTip(tooltip);
	SetValue(m_setting ^ m_reverse);
	_connect_macro_(this, SettingCheckBox::UpdateValue, wxEVT_COMMAND_CHECKBOX_CLICKED, this);
}

template <>
SettingRadioButton::BoolSetting(wxWindow* parent, const wxString& label, const wxString& tooltip, bool &setting, bool reverse, long style)
	: wxRadioButton(parent, -1, label, wxDefaultPosition, wxDefaultSize, style)
	, m_setting(setting)
	, m_reverse(reverse)
{
	SetToolTip(tooltip);
	SetValue(m_setting ^ m_reverse);
	_connect_macro_(this, SettingRadioButton::UpdateValue, wxEVT_COMMAND_RADIOBUTTON_SELECTED, this);
}

SettingChoice::SettingChoice(wxWindow* parent, int &setting, const wxString& tooltip, int num, const wxString choices[])
	: wxChoice(parent, -1, wxDefaultPosition, wxDefaultSize, num, choices)
	, m_setting(setting)
{
	SetToolTip(tooltip);
	Select(m_setting);
	_connect_macro_(this, SettingChoice::UpdateValue, wxEVT_COMMAND_CHOICE_SELECTED, this);
}

void SettingChoice::UpdateValue(wxCommandEvent& ev)
{
	m_setting = ev.GetInt();
	ev.Skip();
}

void VideoConfigDiag::Event_ClickClose(wxCommandEvent&)
{
	Close();
}

void VideoConfigDiag::Event_Close(wxCloseEvent& ev)
{
	g_Config.Save((File::GetUserPath(D_CONFIG_IDX) + ininame + ".ini").c_str());

	ev.Skip();
}

wxString adapter_tooltip = wxT("Select a hardware adapter to use.\nWhen in doubt, use the first one");
wxString ar_tooltip = wxT("Select what aspect ratio to use when rendering:\nAuto: Use the native aspect ratio (4:3)\nForce 16:9: Stretch the picture to an aspect ratio of 16:9.\nForce 4:3: Stretch the picture to an aspect ratio of 4:3.\nStretch to window: Stretch the picture to the window size.");
wxString ws_hack_tooltip = wxT("Force the game to output graphics for widescreen resolutions.\nNote that this might cause graphical glitches");
wxString vsync_tooltip = wxT("Wait for vertical blanks.\nReduces tearing but might also decrease performance");
wxString af_tooltip = wxT("Enables anisotropic filtering.\nEnhances visual quality of textures that are at oblique viewing angles.");
wxString aa_tooltip = wxT("Reduces the amount of aliasing caused by rasterizing 3D graphics.\nThis makes the rendered picture look less blocky but also heavily decreases performance.");
wxString native_mips_tooltip = wxT("Loads native mipmaps instead of generating them.\nLoading native mipmaps is the more accurate behavior, but might also decrease performance (your mileage might vary though).");
wxString scaled_efb_copy_tooltip = wxT("Uses the high-resolution render buffer for EFB copies instead of scaling them down to native resolution.\nVastly improves visual quality in games which use EFB copies but might cause glitches in some games.");
wxString pixel_lighting_tooltip = wxT("Calculates lighting of 3D graphics on a per-pixel basis rather than per vertex.\nThis is the more accurate behavior but reduces performance.");
wxString pixel_depth_tooltip = wxT("");
wxString force_filtering_tooltip = wxT("Forces bilinear texture filtering even if the game explicitly disabled it.\nImproves texture quality (especially when using a high internal resolution) but causes glitches in some games.");
wxString _3d_vision_tooltip = wxT("");
wxString internal_res_tooltip = wxT("Specifies the resolution used to render at.\nA high resolution will improve visual quality a lot but is also quite heavy on performance and might cause glitches in a very limited number of games.\nAuto (performance): Uses the display resolution to render at. This is slightly faster than integral scaling, but might cause visual glitches in a small number of games.\nAuto (accuracy): Uses a multiple of the native resolution to render at. Visual quality compared to the other automatic resolution is similar and a matter of personal preference.\nThe other options specify a fixed resolution and can be used to further increase visual quality.");
wxString efb_access_tooltip = wxT("Allows the CPU to read or write to the EFB (render buffer).\nThis is needed for certain gameplay functionality (e.g. star pointer in Super Mario Galaxy) as well as for certain visual effects (e.g. Monster Hunter Tri), but enabling this option has a huge negative impact on performance if the game uses this functionality.");
wxString efb_copy_tooltip = wxT("");
wxString stc_tooltip = wxT("");
wxString wireframe_tooltip = wxT("Render the scene as a wireframe.\nThis is only useful for debugging purposes.");
wxString disable_lighting_tooltip = wxT("Disable lighting. Improves performance but causes lighting to disappear in games which use it.");
wxString disable_textures_tooltip = wxT("Disable texturing.\nThis is only useful for debugging purposes.");
wxString disable_fog_tooltip = wxT("Disable fog. Improves performance but causes glitches in games which rely on proper fog emulation.");
wxString disable_alphapass_tooltip = wxT("");
wxString show_fps_tooltip = wxT("Show the number of frames rendered per second.");
wxString show_stats_tooltip = wxT("Show various statistics.\nThis is only useful for debugging purposes.");
wxString proj_stats_tooltip = wxT("Show projection statistics.\nThis is only useful for debugging purposes.");
wxString texfmt_tooltip = wxT("Modify textures to show the format they're using.\nThis is only useful for debugging purposes.");
wxString efb_copy_regions_tooltip = wxT("");
wxString xfb_tooltip = wxT("");
wxString dump_textures_tooltip = wxT("Dump game textures to User/Dump/Textures/<game id>/");
wxString load_hires_textures_tooltip = wxT("Load high-resolution textures from User/Load/Textures/<game id>/");
wxString dump_efb_tooltip = wxT("");
wxString dump_frames_tooltip = wxT("");
wxString free_look_tooltip = wxT("");
wxString crop_tooltip = wxT("");
wxString opencl_tooltip = wxT("");
wxString dlc_tooltip = wxT("");
wxString hotkeys_tooltip = wxT("");
wxString ppshader_tooltip = wxT("");

VideoConfigDiag::VideoConfigDiag(wxWindow* parent, const std::string &title, const std::string& _ininame)
	: wxDialog(parent, -1,
		wxString(wxT("Dolphin ")).append(wxString::FromAscii(title.c_str())).append(wxT(" Graphics Configuration")),
		wxDefaultPosition, wxDefaultSize)
	, vconfig(g_Config)
	, ininame(_ininame)
{
	vconfig.Load((File::GetUserPath(D_CONFIG_IDX) + ininame + ".ini").c_str());

	wxNotebook* const notebook = new wxNotebook(this, -1, wxDefaultPosition, wxDefaultSize);

	// -- GENERAL --
	{
	wxPanel* const page_general = new wxPanel(notebook, -1, wxDefaultPosition);
	notebook->AddPage(page_general, wxT("General"));
	wxBoxSizer* const szr_general = new wxBoxSizer(wxVERTICAL);

	// - basic
	{
	wxStaticBoxSizer* const group_basic = new wxStaticBoxSizer(wxVERTICAL, page_general, wxT("Basic"));
	szr_general->Add(group_basic, 0, wxEXPAND | wxALL, 5);
	wxFlexGridSizer* const szr_basic = new wxFlexGridSizer(2, 5, 5);
	group_basic->Add(szr_basic, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	// graphics api
	//{
	//const wxString gfxapi_choices[] = { wxT("Software [not present]"),
	//	wxT("OpenGL [broken]"), wxT("Direct3D 9 [broken]"), wxT("Direct3D 11") };

	//szr_basic->Add(new wxStaticText(page_general, -1, wxT("Graphics API:")), 1, wxALIGN_CENTER_VERTICAL, 0);
	//wxChoice* const choice_gfxapi = new SettingChoice(page_general,
	//	g_gfxapi, sizeof(gfxapi_choices)/sizeof(*gfxapi_choices), gfxapi_choices);
	//szr_basic->Add(choice_gfxapi, 1, 0, 0);
	// TODO: Connect with Event_Backend()
	//}

	// adapter // for D3D only
	if (vconfig.backend_info.Adapters.size())
	{
	szr_basic->Add(new wxStaticText(page_general, -1, wxT("Adapter:")), 1, wxALIGN_CENTER_VERTICAL, 5);
	wxChoice* const choice_adapter = new SettingChoice(page_general, vconfig.iAdapter, adapter_tooltip);

	std::vector<std::string>::const_iterator
		it = vconfig.backend_info.Adapters.begin(),
		itend = vconfig.backend_info.Adapters.end();
	for (; it != itend; ++it)
		choice_adapter->AppendString(wxString::FromAscii(it->c_str()));

	choice_adapter->Select(vconfig.iAdapter);

	szr_basic->Add(choice_adapter, 1, 0, 0);
	}
	
	// aspect-ratio
	{
	const wxString ar_choices[] = { wxT("Auto [recommended]"),
		wxT("Force 16:9"), wxT("Force 4:3"), wxT("Strech to Window") };

	szr_basic->Add(new wxStaticText(page_general, -1, wxT("Aspect ratio:")), 1, wxALIGN_CENTER_VERTICAL, 0);
	wxChoice* const choice_aspect = new SettingChoice(page_general,
		vconfig.iAspectRatio, ar_tooltip, sizeof(ar_choices)/sizeof(*ar_choices), ar_choices);
	szr_basic->Add(choice_aspect, 1, 0, 0);
	}

	// widescreen hack
	{
	szr_basic->AddStretchSpacer(1);
	szr_basic->Add(new SettingCheckBox(page_general, wxT("Widescreen Hack"), ws_hack_tooltip, vconfig.bWidescreenHack), 1, 0, 0);
	szr_basic->AddStretchSpacer(1);
	szr_basic->Add(new SettingCheckBox(page_general, wxT("V-Sync"), vsync_tooltip, vconfig.bVSync), 1, 0, 0);
	}

	// enhancements
	{
	wxStaticBoxSizer* const group_enh = new wxStaticBoxSizer(wxVERTICAL, page_general, wxT("Enhancements"));
	szr_general->Add(group_enh, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
	wxFlexGridSizer* const szr_enh = new wxFlexGridSizer(2, 5, 5);
	group_enh->Add(szr_enh, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	szr_enh->Add(new wxStaticText(page_general, -1, wxT("Anisotropic Filtering:")), 1, wxALIGN_CENTER_VERTICAL, 0);
	const wxString af_choices[] = {wxT("1x"), wxT("2x"), wxT("4x"), wxT("8x"), wxT("16x")};
	szr_enh->Add(new SettingChoice(page_general, vconfig.iMaxAnisotropy, af_tooltip, 5, af_choices));


	wxStaticText* const text_aamode = new wxStaticText(page_general, -1, wxT("Anti-Aliasing:"));
	szr_enh->Add(text_aamode, 1, wxALIGN_CENTER_VERTICAL, 0);
	SettingChoice* const choice_aamode = new SettingChoice(page_general, vconfig.iMultisampleMode, aa_tooltip);

	std::vector<std::string>::const_iterator
		it = vconfig.backend_info.AAModes.begin(),
		itend = vconfig.backend_info.AAModes.end();
	for (; it != itend; ++it)
		choice_aamode->AppendString(wxString::FromAscii(it->c_str()));

	if (vconfig.backend_info.AAModes.size() <= 1)
	{
		choice_aamode->Disable();
		text_aamode->Disable();
	}

	choice_aamode->Select(vconfig.iMultisampleMode);
	szr_enh->Add(choice_aamode);


	szr_enh->Add(new SettingCheckBox(page_general, wxT("Load Native Mipmaps"), native_mips_tooltip, vconfig.bUseNativeMips));
	szr_enh->Add(new SettingCheckBox(page_general, wxT("EFB Scaled Copy"), scaled_efb_copy_tooltip, vconfig.bCopyEFBScaled));	
	szr_enh->Add(new SettingCheckBox(page_general, wxT("Pixel Lighting"), pixel_lighting_tooltip, vconfig.bEnablePixelLigting));
	szr_enh->Add(new SettingCheckBox(page_general, wxT("Pixel Depth"), pixel_depth_tooltip, vconfig.bEnablePerPixelDepth));
	szr_enh->Add(new SettingCheckBox(page_general, wxT("Force Bi/Trilinear Filtering"), force_filtering_tooltip, vconfig.bForceFiltering));
	
	if (vconfig.backend_info.bSupports3DVision)
	{
		szr_enh->Add(new SettingCheckBox(page_general, wxT("3D Vision (Requires Fullscreen)"), _3d_vision_tooltip, vconfig.b3DVision));
	}

	}

	// - EFB
	{
	wxStaticBoxSizer* const group_efb = new wxStaticBoxSizer(wxVERTICAL, page_general, wxT("EFB"));
	szr_general->Add(group_efb, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	// EFB scale
	{
	wxBoxSizer* const efb_scale_szr = new wxBoxSizer(wxHORIZONTAL);
	// TODO: give this a label
	const wxString efbscale_choices[] = { wxT("Fractional"), wxT("Integral [recommended]"),
		wxT("1x"), wxT("2x"), wxT("3x")/*, wxT("4x")*/ };

	wxChoice *const choice_efbscale = new SettingChoice(page_general,
		vconfig.iEFBScale, internal_res_tooltip, sizeof(efbscale_choices)/sizeof(*efbscale_choices), efbscale_choices);

	efb_scale_szr->Add(new wxStaticText(page_general, -1, wxT("Scale:")), 0, wxALIGN_CENTER_VERTICAL, 5);
	//efb_scale_szr->AddStretchSpacer(1);
	efb_scale_szr->Add(choice_efbscale, 0, wxBOTTOM | wxLEFT, 5);

	group_efb->Add(efb_scale_szr, 0, wxBOTTOM | wxLEFT, 5);
	}

	group_efb->Add(new SettingCheckBox(page_general, wxT("Enable CPU Access"), efb_access_tooltip, vconfig.bEFBAccessEnable), 0, wxBOTTOM | wxLEFT, 5);

	// EFB copy
	wxStaticBoxSizer* const group_efbcopy = new wxStaticBoxSizer(wxHORIZONTAL, page_general, wxT("Copy"));
	group_efb->Add(group_efbcopy, 0, wxEXPAND | wxBOTTOM, 5);

	SettingCheckBox* efbcopy_enable = new SettingCheckBox(page_general, wxT("Enable"), efb_copy_tooltip, vconfig.bEFBCopyEnable);
	_connect_macro_(efbcopy_enable, VideoConfigDiag::Event_EfbCopy, wxEVT_COMMAND_CHECKBOX_CLICKED, this);
	efbcopy_texture = new SettingRadioButton(page_general, wxT("Texture"), efb_copy_tooltip, vconfig.bCopyEFBToTexture, false, wxRB_GROUP);
	efbcopy_ram = new SettingRadioButton(page_general, wxT("RAM"), efb_copy_tooltip, vconfig.bCopyEFBToTexture, true);
	group_efbcopy->Add(efbcopy_enable, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	group_efbcopy->AddStretchSpacer(1);
	group_efbcopy->Add(efbcopy_texture, 0, wxRIGHT, 5);
	group_efbcopy->Add(efbcopy_ram, 0, wxRIGHT, 5);
	if (!vconfig.backend_info.bSupportsEFBToRAM)
	{
		efbcopy_ram->Disable();
		vconfig.bCopyEFBToTexture = true;
		efbcopy_texture->SetValue(true);
	}
	if (!vconfig.bEFBCopyEnable)
	{
		efbcopy_ram->Disable();
		efbcopy_texture->Disable();
	}
	}

	// - safe texture cache
	{
	wxStaticBoxSizer* const group_safetex = new wxStaticBoxSizer(wxHORIZONTAL, page_general, wxT("Safe Texture Cache"));
	szr_general->Add(group_safetex, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	SettingCheckBox* stc_enable = new SettingCheckBox(page_general, wxT("Enable"), stc_tooltip, vconfig.bSafeTextureCache);
	_connect_macro_(stc_enable, VideoConfigDiag::Event_Stc, wxEVT_COMMAND_CHECKBOX_CLICKED, this);
	group_safetex->Add(stc_enable, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	group_safetex->AddStretchSpacer(1);

	stc_safe = new wxRadioButton(page_general, -1, wxT("Safe"),
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	_connect_macro_(stc_safe, VideoConfigDiag::Event_StcSafe, wxEVT_COMMAND_RADIOBUTTON_SELECTED, this);
	group_safetex->Add(stc_safe, 0, wxRIGHT, 5);
	if (0 == vconfig.iSafeTextureCache_ColorSamples)
		stc_safe->SetValue(true);

	stc_normal = new wxRadioButton(page_general, -1, wxT("Normal"));
	_connect_macro_(stc_normal, VideoConfigDiag::Event_StcNormal, wxEVT_COMMAND_RADIOBUTTON_SELECTED, this);
	group_safetex->Add(stc_normal, 0, wxRIGHT, 5);
	if (512 == vconfig.iSafeTextureCache_ColorSamples)
		stc_normal->SetValue(true);

	stc_fast = new wxRadioButton(page_general, -1, wxT("Fast"));
	_connect_macro_(stc_fast, VideoConfigDiag::Event_StcFast, wxEVT_COMMAND_RADIOBUTTON_SELECTED, this);
	group_safetex->Add(stc_fast, 0, wxRIGHT, 5);
	if (128 == vconfig.iSafeTextureCache_ColorSamples)
		stc_fast->SetValue(true);

	if (!vconfig.bSafeTextureCache)
	{
		stc_safe->Disable();
		stc_normal->Disable();
		stc_fast->Disable();
	}
	}

	}

	page_general->SetSizerAndFit(szr_general);
	}

	// -- ADVANCED --
	{
	wxPanel* const page_advanced = new wxPanel(notebook, -1, wxDefaultPosition);
	notebook->AddPage(page_advanced, wxT("Advanced"));
	wxBoxSizer* const szr_advanced = new wxBoxSizer(wxVERTICAL);

	// - rendering
	{
	wxStaticBoxSizer* const group_rendering = new wxStaticBoxSizer(wxVERTICAL, page_advanced, wxT("Rendering"));
	szr_advanced->Add(group_rendering, 0, wxEXPAND | wxALL, 5);
	wxGridSizer* const szr_rendering = new wxGridSizer(2, 5, 5);
	group_rendering->Add(szr_rendering, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	szr_rendering->Add(new SettingCheckBox(page_advanced, wxT("Enable Wireframe"), wireframe_tooltip, vconfig.bWireFrame));
	szr_rendering->Add(new SettingCheckBox(page_advanced, wxT("Disable Lighting"), disable_lighting_tooltip, vconfig.bDisableLighting));
	szr_rendering->Add(new SettingCheckBox(page_advanced, wxT("Disable Textures"), disable_textures_tooltip, vconfig.bDisableTexturing));
	szr_rendering->Add(new SettingCheckBox(page_advanced, wxT("Disable Fog"), disable_fog_tooltip, vconfig.bDisableFog));
	szr_rendering->Add(new SettingCheckBox(page_advanced, wxT("Disable Dest. Alpha Pass"), disable_alphapass_tooltip, vconfig.bDstAlphaPass));
	}

	// - info
	{
	wxStaticBoxSizer* const group_info = new wxStaticBoxSizer(wxVERTICAL, page_advanced, wxT("Overlay Information"));
	szr_advanced->Add(group_info, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
	wxGridSizer* const szr_info = new wxGridSizer(2, 5, 5);
	group_info->Add(szr_info, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	szr_info->Add(new SettingCheckBox(page_advanced, wxT("Show FPS"), show_fps_tooltip, vconfig.bShowFPS));
	szr_info->Add(new SettingCheckBox(page_advanced, wxT("Various Statistics"), show_stats_tooltip, vconfig.bOverlayStats));
	szr_info->Add(new SettingCheckBox(page_advanced, wxT("Projection Stats"), proj_stats_tooltip, vconfig.bOverlayProjStats));
	szr_info->Add(new SettingCheckBox(page_advanced, wxT("Texture Format"), texfmt_tooltip, vconfig.bTexFmtOverlayEnable));
	szr_info->Add(new SettingCheckBox(page_advanced, wxT("EFB Copy Regions"), efb_copy_regions_tooltip, vconfig.bShowEFBCopyRegions));
	}
	
	// - XFB
	{
	wxStaticBoxSizer* const group_xfb = new wxStaticBoxSizer(wxHORIZONTAL, page_advanced, wxT("XFB"));
	szr_advanced->Add(group_xfb, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	SettingCheckBox* enable_xfb = new SettingCheckBox(page_advanced, wxT("Enable"), xfb_tooltip, vconfig.bUseXFB);
	_connect_macro_(enable_xfb, VideoConfigDiag::Event_Xfb, wxEVT_COMMAND_CHECKBOX_CLICKED, this);
	virtual_xfb = new SettingRadioButton(page_advanced, wxT("Virtual"), xfb_tooltip, vconfig.bUseRealXFB, true, wxRB_GROUP);
	real_xfb = new SettingRadioButton(page_advanced, wxT("Real"), xfb_tooltip, vconfig.bUseRealXFB);
	group_xfb->Add(enable_xfb, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	group_xfb->AddStretchSpacer(1);
	group_xfb->Add(virtual_xfb, 0, wxRIGHT, 5);
	group_xfb->Add(real_xfb, 0, wxRIGHT, 5);

	if (!vconfig.backend_info.bSupportsRealXFB)
	{
		real_xfb->Disable();
		vconfig.bUseRealXFB = false;
		virtual_xfb->SetValue(true);
	}

	if (!vconfig.bUseXFB)
	{
		real_xfb->Disable();
		virtual_xfb->Disable();
	}

	}	// xfb

	// - utility
	{
	wxStaticBoxSizer* const group_utility = new wxStaticBoxSizer(wxVERTICAL, page_advanced, wxT("Utility"));
	szr_advanced->Add(group_utility, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
	wxGridSizer* const szr_utility = new wxGridSizer(2, 5, 5);
	group_utility->Add(szr_utility, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	szr_utility->Add(new SettingCheckBox(page_advanced, wxT("Dump Textures"), dump_textures_tooltip, vconfig.bDumpTextures));
	szr_utility->Add(new SettingCheckBox(page_advanced, wxT("Load Hi-Res Textures"), load_hires_textures_tooltip, vconfig.bHiresTextures));
	szr_utility->Add(new SettingCheckBox(page_advanced, wxT("Dump EFB Target"), dump_efb_tooltip, vconfig.bDumpEFBTarget));
	szr_utility->Add(new SettingCheckBox(page_advanced, wxT("Dump Frames"), dump_frames_tooltip, vconfig.bDumpFrames));
	szr_utility->Add(new SettingCheckBox(page_advanced, wxT("Free Look"), free_look_tooltip, vconfig.bFreeLook));
	}

	// - misc
	{
	wxStaticBoxSizer* const group_misc = new wxStaticBoxSizer(wxVERTICAL, page_advanced, wxT("Misc"));
	szr_advanced->Add(group_misc, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
	wxFlexGridSizer* const szr_misc = new wxFlexGridSizer(2, 5, 5);
	group_misc->Add(szr_misc, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	szr_misc->Add(new SettingCheckBox(page_advanced, wxT("Crop"), crop_tooltip, vconfig.bCrop));
	szr_misc->Add(new SettingCheckBox(page_advanced, wxT("Enable OpenCL"), opencl_tooltip, vconfig.bEnableOpenCL));
	szr_misc->Add(new SettingCheckBox(page_advanced, wxT("Enable Display List Caching"), dlc_tooltip, vconfig.bDlistCachingEnable));
	szr_misc->Add(new SettingCheckBox(page_advanced, wxT("Enable Hotkeys"), hotkeys_tooltip, vconfig.bOSDHotKey));

	// postproc shader
	if (vconfig.backend_info.PPShaders.size())
	{
		szr_misc->Add(new wxStaticText(page_advanced, -1, wxT("Post-Processing Shader:")), 1, wxALIGN_CENTER_VERTICAL, 0);

		wxChoice *const choice_ppshader = new wxChoice(page_advanced, -1, wxDefaultPosition);
		choice_ppshader->SetToolTip(ppshader_tooltip);
		choice_ppshader->AppendString(wxT("(off)"));

		std::vector<std::string>::const_iterator
			it = vconfig.backend_info.PPShaders.begin(),
			itend = vconfig.backend_info.PPShaders.end();
		for (; it != itend; ++it)
			choice_ppshader->AppendString(wxString::FromAscii(it->c_str()));

		if (vconfig.sPostProcessingShader.empty())
			choice_ppshader->Select(0);
		else
			choice_ppshader->SetStringSelection(wxString::FromAscii(vconfig.sPostProcessingShader.c_str()));

		_connect_macro_(choice_ppshader, VideoConfigDiag::Event_PPShader, wxEVT_COMMAND_CHOICE_SELECTED, this);

		szr_misc->Add(choice_ppshader, 0, wxLEFT, 5);
	}

	}

	page_advanced->SetSizerAndFit(szr_advanced);
	}

	wxButton* const btn_close = new wxButton(this, -1, wxT("Close"), wxDefaultPosition);
	_connect_macro_(btn_close, VideoConfigDiag::Event_ClickClose, wxEVT_COMMAND_BUTTON_CLICKED, this);

	Connect(-1, wxEVT_CLOSE_WINDOW, wxCloseEventHandler(VideoConfigDiag::Event_Close), (wxObject*)0, this);

	wxBoxSizer* const szr_main = new wxBoxSizer(wxVERTICAL);
	szr_main->Add(notebook, 1, wxEXPAND | wxALL, 5);
	szr_main->Add(btn_close, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);

	SetSizerAndFit(szr_main);
	Center();
}
