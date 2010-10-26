// Copyright (C) 2010 Dolphin Project.

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

#include "ControllerEmu.h"

#if defined(HAVE_X11) && HAVE_X11
#include <X11/Xlib.h>
#endif

ControllerEmu::~ControllerEmu()
{
	// control groups
	std::vector<ControlGroup*>::const_iterator
		i = groups.begin(),
		e = groups.end();
	for ( ; i!=e; ++i )
		delete *i;
}

ControllerEmu::ControlGroup::~ControlGroup()
{
	// controls
	std::vector<Control*>::const_iterator
		ci = controls.begin(),
		ce = controls.end();
	for ( ; ci!=ce; ++ci )
		delete *ci;

	// settings
	std::vector<Setting*>::const_iterator
		si = settings.begin(),
		se = settings.end();
	for ( ; si!=se; ++si )
		delete *si;
}

ControllerEmu::Extension::~Extension()
{
	// attachments
	std::vector<ControllerEmu*>::const_iterator
		ai = attachments.begin(),
		ae = attachments.end();
	for ( ; ai!=ae; ++ai )
		delete *ai;
}
ControllerEmu::ControlGroup::Control::~Control()
{
	delete control_ref;
}

void ControllerEmu::UpdateReferences(ControllerInterface& devi)
{
	std::vector<ControlGroup*>::const_iterator
		i = groups.begin(),
		e = groups.end();
	for ( ; i!=e; ++i )
	{
		std::vector<ControlGroup::Control*>::const_iterator
			ci = (*i)->controls.begin(),
			ce = (*i)->controls.end();
		for ( ; ci!=ce; ++ci )
			devi.UpdateReference((*ci)->control_ref, default_device);

		// extension
		if ( GROUP_TYPE_EXTENSION == (*i)->type )
		{
			std::vector<ControllerEmu*>::const_iterator
				ai = ((Extension*)*i)->attachments.begin(),
				ae = ((Extension*)*i)->attachments.end();
			for ( ; ai!=ae; ++ai )
				(*ai)->UpdateReferences(devi);
		}
	}
}

void ControllerEmu::UpdateDefaultDevice()
{
	std::vector<ControlGroup*>::const_iterator
		i = groups.begin(),
		e = groups.end();
	for ( ; i!=e; ++i )
	{
		//std::vector<ControlGroup::Control*>::const_iterator
			//ci = (*i)->controls.begin(),
			//ce = (*i)->controls.end();
		//for ( ; ci!=ce; ++ci )
			//(*ci)->control_ref->device_qualifier = default_device;

		// extension
		if ( GROUP_TYPE_EXTENSION == (*i)->type )
		{
			std::vector<ControllerEmu*>::const_iterator
				ai = ((Extension*)*i)->attachments.begin(),
				ae = ((Extension*)*i)->attachments.end();
			for ( ; ai!=ae; ++ai )
			{
				(*ai)->default_device = default_device;
				(*ai)->UpdateDefaultDevice();
			}
		}
	}
}

void ControllerEmu::ControlGroup::LoadConfig(IniFile::Section *sec, const std::string& defdev, const std::string& base )
{
	std::string group( base + name ); group += "/";

	// settings
	std::vector<ControlGroup::Setting*>::const_iterator
		si = settings.begin(),
		se = settings.end();
	for ( ; si!=se; ++si )
	{
		sec->Get((group+(*si)->name).c_str(), &(*si)->value, (*si)->default_value*100);
		(*si)->value /= 100;
	}

	// controls
	std::vector<ControlGroup::Control*>::const_iterator
		ci = controls.begin(),
		ce = controls.end();
	for ( ; ci!=ce; ++ci )
	{
		// control expression
		sec->Get((group + (*ci)->name).c_str(), &(*ci)->control_ref->expression, "");

		// range
		sec->Get( (group+(*ci)->name+"/Range").c_str(), &(*ci)->control_ref->range, 100.0f);
		(*ci)->control_ref->range /= 100;

	}

	// extensions
	if ( GROUP_TYPE_EXTENSION == type )
	{
		Extension* const ex = ((Extension*)this);

		ex->switch_extension = 0;
		unsigned int n = 0;
		std::string extname;
		sec->Get((base + name).c_str(), &extname, "");

		std::vector<ControllerEmu*>::const_iterator
			ai = ((Extension*)this)->attachments.begin(),
			ae = ((Extension*)this)->attachments.end();
		for ( ; ai!=ae; ++ai,++n )
		{
			(*ai)->default_device.FromString( defdev );
			(*ai)->LoadConfig( sec, base + (*ai)->GetName() + "/" );

			if ( (*ai)->GetName() == extname )
				ex->switch_extension = n;
		}
	}
}

void ControllerEmu::LoadConfig( IniFile::Section *sec, const std::string& base )
{
	std::string defdev = default_device.ToString();
	if (base.empty())
	{
		sec->Get((base + "Device").c_str(), &defdev, "");
		default_device.FromString(defdev);
	}
	std::vector<ControlGroup*>::const_iterator i = groups.begin(),
		e = groups.end();
	for ( ; i!=e; ++i )
		(*i)->LoadConfig(sec, defdev, base);
}

void ControllerEmu::ControlGroup::SaveConfig( IniFile::Section *sec, const std::string& defdev, const std::string& base )
{
	std::string group( base + name ); group += "/";

	// settings
	std::vector<ControlGroup::Setting*>::const_iterator
		si = settings.begin(),
		se = settings.end();
	for ( ; si!=se; ++si )
		sec->Set( (group+(*si)->name).c_str(), (*si)->value*100.0f, (*si)->default_value*100.0f);

	// controls
	std::vector<ControlGroup::Control*>::const_iterator
		ci = controls.begin(),
		ce = controls.end();
	for ( ; ci!=ce; ++ci )
	{
		// control expression
		sec->Set( (group+(*ci)->name).c_str(), (*ci)->control_ref->expression, "");

		// range
		sec->Set( (group+(*ci)->name+"/Range").c_str(), (*ci)->control_ref->range*100.0f, 100.0f);
	}

	// extensions
	if ( GROUP_TYPE_EXTENSION == type )
	{
		Extension* const ext = ((Extension*)this);
		sec->Set((base + name).c_str(), ext->attachments[ext->switch_extension]->GetName(), "None");

		std::vector<ControllerEmu*>::const_iterator
			ai = ((Extension*)this)->attachments.begin(),
			ae = ((Extension*)this)->attachments.end();
		for ( ; ai!=ae; ++ai )
			(*ai)->SaveConfig( sec, base + (*ai)->GetName() + "/" );
	}
}

void ControllerEmu::SaveConfig( IniFile::Section *sec, const std::string& base )
{
	const std::string defdev = default_device.ToString();
	if ( base.empty() )
		sec->Set( (/*std::string(" ") +*/ base + "Device").c_str(), defdev, "");

	std::vector<ControlGroup*>::const_iterator i = groups.begin(),
		e = groups.end();
	for ( ; i!=e; ++i )
		(*i)->SaveConfig( sec, defdev, base );
}

ControllerEmu::AnalogStick::AnalogStick( const char* const _name ) : ControlGroup( _name, GROUP_TYPE_STICK )
{
	for ( unsigned int i = 0; i < 4; ++i )
		controls.push_back( new Input( named_directions[i] ) );

	controls.push_back( new Input( "Modifier" ) );

	settings.push_back( new Setting("Dead Zone", 0, 0, 50 ) );
	settings.push_back( new Setting("Square Stick", 0 ) );

}

ControllerEmu::Buttons::Buttons( const char* const _name ) : ControlGroup( _name, GROUP_TYPE_BUTTONS )
{
	settings.push_back( new Setting("Threshold", 0.5f ) );
}

ControllerEmu::MixedTriggers::MixedTriggers( const char* const _name ) : ControlGroup( _name, GROUP_TYPE_MIXED_TRIGGERS )
{
	settings.push_back( new Setting("Threshold", 0.9f ) );
}

ControllerEmu::Triggers::Triggers( const char* const _name ) : ControlGroup( _name, GROUP_TYPE_TRIGGERS )
{
	settings.push_back( new Setting("Dead Zone", 0, 0, 50 ) );
}

ControllerEmu::Slider::Slider(const char* const _name) : ControlGroup(_name, GROUP_TYPE_SLIDER)
{
	controls.push_back(new Input("Left"));
	controls.push_back(new Input("Right"));

	settings.push_back(new Setting("Dead Zone", 0, 0, 50));
}

ControllerEmu::Force::Force( const char* const _name ) : ControlGroup( _name, GROUP_TYPE_FORCE )
{
	memset(m_swing, 0, sizeof(m_swing));

	controls.push_back( new Input( "Up" ) );
	controls.push_back( new Input( "Down" ) );
	controls.push_back( new Input( "Left" ) );
	controls.push_back( new Input( "Right" ) );
	controls.push_back( new Input( "Forward" ) );
	controls.push_back( new Input( "Backward" ) );
	//controls.push_back( new Input( "Modifier" ) );

	settings.push_back( new Setting("Dead Zone", 0, 0, 50 ) );
}

ControllerEmu::Tilt::Tilt( const char* const _name )
	: ControlGroup( _name, GROUP_TYPE_TILT )
{
	memset(m_tilt, 0, sizeof(m_tilt));

	controls.push_back( new Input( "Forward" ) );
	controls.push_back( new Input( "Backward" ) );
	controls.push_back( new Input( "Left" ) );
	controls.push_back( new Input( "Right" ) );

	controls.push_back( new Input( "Modifier" ) );

	settings.push_back( new Setting("Dead Zone", 0, 0, 50 ) );
	settings.push_back( new Setting("Circle Stick", 0 ) );
}

ControllerEmu::Cursor::Cursor(const char* const _name)
	: ControlGroup(_name, GROUP_TYPE_CURSOR)
	, m_z(0)
{
	for ( unsigned int i = 0; i < 4; ++i )
		controls.push_back( new Input( named_directions[i] ) );
	controls.push_back( new Input( "Forward" ) );
	controls.push_back( new Input( "Backward" ) );
	controls.push_back( new Input( "Hide" ) );

	settings.push_back( new Setting("Center", 0.5f ) );
	settings.push_back( new Setting("Width", 0.5f ) );
	settings.push_back( new Setting("Height", 0.5f ) );

}

void ControllerEmu::LoadDefaults(const ControllerInterface &ciface)
{
	// load an empty inifile section, clears everything
	IniFile::Section sec;
	LoadConfig(&sec);

	if (ciface.Devices().size())
	{
		default_device.FromDevice(ciface.Devices()[0]);
		UpdateDefaultDevice();
	}
}
