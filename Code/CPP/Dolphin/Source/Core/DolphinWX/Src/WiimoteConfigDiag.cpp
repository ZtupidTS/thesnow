
#include "WiimoteConfigDiag.h"
#include "HW/Wiimote.h"
#include "Frame.h"

#define _connect_macro_(b, f, c, s)	(b)->Connect(wxID_ANY, (c), wxCommandEventHandler( f ), (wxObject*)0, (wxEvtHandler*)s)

const wxString& ConnectedWiimotesString()
{
	static wxString str;
	str.Printf(_("Connected to %i Wiimotes"), WiimoteReal::Initialize());
	return str;
}

WiimoteConfigPage::WiimoteConfigPage(wxWindow* const parent, const int index)
	: wxNotebookPage(parent, -1, wxDefaultPosition, wxDefaultSize)
	, m_index(index)
{
	// input source
	const wxString src_choices[] = { _("None"),
		_("Emulated Wiimote"), _("Real Wiimote"), _("Hybrid Wiimote") };

	wxChoice* const input_src_choice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize,
		sizeof(src_choices)/sizeof(*src_choices), src_choices);
	input_src_choice->Select(g_wiimote_sources[m_index]);
	_connect_macro_(input_src_choice, WiimoteConfigPage::SelectSource, wxEVT_COMMAND_CHOICE_SELECTED, this);

	wxStaticBoxSizer* const input_src_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Input Source"));
	input_src_sizer->Add(input_src_choice, 1, wxEXPAND | wxALL, 5);

	// emulated wiimote
	wxButton* const configure_wiimote_emu_btn = new wxButton(this, -1, _("Configure"));
	wxStaticBoxSizer* const wiimote_emu_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Emulated Wiimote"));
	wiimote_emu_sizer->Add(configure_wiimote_emu_btn, 1, wxEXPAND | wxALL, 5);
	_connect_macro_(configure_wiimote_emu_btn, WiimoteConfigDiag::ConfigEmulatedWiimote, wxEVT_COMMAND_BUTTON_CLICKED, parent->GetParent());

	// real wiimote
	connected_wiimotes_txt = new wxStaticText(this, -1, ConnectedWiimotesString());

	wxButton* const refresh_btn = new wxButton(this, -1, _("Refresh"), wxDefaultPosition);
	_connect_macro_(refresh_btn, WiimoteConfigDiag::RefreshRealWiimotes, wxEVT_COMMAND_BUTTON_CLICKED, parent->GetParent());

	wxStaticBoxSizer* const wiimote_real_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Real Wiimote"));
	wiimote_real_sizer->AddStretchSpacer(1);
	wiimote_real_sizer->Add(connected_wiimotes_txt, 0, wxALIGN_CENTER | wxBOTTOM | wxLEFT | wxRIGHT, 5);
#ifdef _WIN32
	wxButton* const pairup_btn = new wxButton(this, -1, _("Pair Up"), wxDefaultPosition);
	_connect_macro_(pairup_btn, WiimoteConfigDiag::PairUpRealWiimotes, wxEVT_COMMAND_BUTTON_CLICKED, parent->GetParent());
	wiimote_real_sizer->Add(pairup_btn, 0, wxALIGN_CENTER | wxBOTTOM, 5);
#endif
	wiimote_real_sizer->Add(refresh_btn, 0, wxALIGN_CENTER, 5);
	wiimote_real_sizer->AddStretchSpacer(1);

	// sizers
	wxBoxSizer* const left_sizer = new wxBoxSizer(wxVERTICAL);
	left_sizer->Add(input_src_sizer, 0, wxEXPAND | wxBOTTOM, 5);
	left_sizer->Add(wiimote_emu_sizer, 0, wxEXPAND, 0);

	wxBoxSizer* const main_sizer = new wxBoxSizer(wxHORIZONTAL);
	main_sizer->Add(left_sizer, 1, wxLEFT | wxTOP | wxBOTTOM | wxEXPAND, 5);
	main_sizer->Add(wiimote_real_sizer, 1, wxALL | wxEXPAND, 5);

	SetSizerAndFit(main_sizer);
	Layout();
}

WiimoteConfigDiag::WiimoteConfigDiag(wxWindow* const parent, InputPlugin& plugin)
	: wxDialog(parent, -1, _("Dolphin Wiimote Configuration"), wxDefaultPosition, wxDefaultSize)
	, m_plugin(plugin)
{
	m_pad_notebook = new wxNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxNB_DEFAULT);
	for (unsigned int i = 0; i < 4; ++i)
	{
		WiimoteConfigPage* const wpage = new WiimoteConfigPage(m_pad_notebook, i);
		//m_padpages.push_back(wpage);
		m_pad_notebook->AddPage(wpage, wxString(wxT("Wiimote ")) + wxChar('1'+i));
	}

	wxButton* const ok_button = new wxButton(this, -1, _("OK"), wxDefaultPosition);
	_connect_macro_(ok_button, WiimoteConfigDiag::Save, wxEVT_COMMAND_BUTTON_CLICKED, this);

	wxBoxSizer* const main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(m_pad_notebook, 1, wxEXPAND | wxALL, 5);
	main_sizer->Add(ok_button, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 5);

	SetSizerAndFit(main_sizer);

	Center();
}

void WiimoteConfigDiag::ConfigEmulatedWiimote(wxCommandEvent&)
{
	InputConfigDialog* const m_emu_config_diag = new InputConfigDialog(this, m_plugin, "Dolphin Emulated Wiimote Configuration", m_pad_notebook->GetSelection());
	m_emu_config_diag->ShowModal();
	m_emu_config_diag->Destroy();
}

void WiimoteConfigDiag::UpdateGUI()
{
	for (size_t p = 0; p < m_pad_notebook->GetPageCount(); ++p)
		((WiimoteConfigPage*)m_pad_notebook->GetPage(p))->
			connected_wiimotes_txt->SetLabel(ConnectedWiimotesString());
}

#ifdef _WIN32
void WiimoteConfigDiag::PairUpRealWiimotes(wxCommandEvent&)
{
	const int paired = WiimoteReal::PairUp();

	if (paired > 0)
	{
		// Will this message be anoying?
		//PanicAlert("Paired %d wiimotes.", paired);
		WiimoteReal::Refresh();
		UpdateGUI();
	}
	else if (paired < 0)
		PanicAlert("A supported bluetooth device was not found!\n"
		"(Only the Microsoft bluetooth stack is supported.)");
}
#endif

void WiimoteConfigDiag::RefreshRealWiimotes(wxCommandEvent&)
{
	WiimoteReal::Refresh();
	UpdateGUI();
}

void WiimoteConfigPage::SelectSource(wxCommandEvent& event)
{
	// should be kinda fine, maybe should just set when user clicks OK, w/e change it later
	g_wiimote_sources[m_index] = event.GetInt();

	// Connect or disconnect emulated wiimotes (maybe do this in when OK is clicked too?).
	CFrame::ConnectWiimote(m_index, WIIMOTE_SRC_EMU & g_wiimote_sources[m_index]);
}

void WiimoteConfigDiag::Save(wxCommandEvent&)
{
	std::string ini_filename = (std::string(File::GetUserPath(D_CONFIG_IDX)) + WIIMOTE_INI_NAME ".ini" );

	IniFile inifile;
	inifile.Load(ini_filename);

	for (unsigned int i=0; i<MAX_WIIMOTES; ++i)
	{
		std::string secname("Wiimote");
		secname += (char)('1' + i);
		IniFile::Section& sec = *inifile.GetOrCreateSection(secname.c_str());

		sec.Set("Source", (int)g_wiimote_sources[i]);
	}

	inifile.Save(ini_filename);

	Close();
}
