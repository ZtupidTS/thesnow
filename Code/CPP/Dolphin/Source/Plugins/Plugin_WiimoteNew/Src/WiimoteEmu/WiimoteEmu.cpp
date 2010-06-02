
#include "Attachment/Classic.h"
#include "Attachment/Nunchuk.h"
#include "Attachment/Guitar.h"
#include "Attachment/Drums.h"

#include "WiimoteEmu.h"
#include "WiimoteHid.h"

#include <Timer.h>
#include <Common.h>

// buttons

#define WIIMOTE_PAD_LEFT		0x01
#define WIIMOTE_PAD_RIGHT		0x02
#define WIIMOTE_PAD_DOWN		0x04
#define WIIMOTE_PAD_UP			0x08
#define WIIMOTE_PLUS 			0x10

#define WIIMOTE_TWO				0x0100
#define WIIMOTE_ONE				0x0200
#define WIIMOTE_B				0x0400
#define WIIMOTE_A				0x0800
#define WIIMOTE_MINUS	 		0x1000
#define	WIIMOTE_HOME			0x8000

namespace WiimoteEmu
{

/* An example of a factory default first bytes of the Eeprom memory. There are differences between
   different Wiimotes, my Wiimote had different neutral values for the accelerometer. */
static const u8 eeprom_data_0[] = {
	// IR, maybe more
	// assuming last 2 bytes are checksum
	0xA1, 0xAA, 0x8B, 0x99, 0xAE, 0x9E, 0x78, 0x30, 0xA7, /*0x74, 0xD3,*/ 0x00, 0x00,	// messing up the checksum on purpose
	0xA1, 0xAA, 0x8B, 0x99, 0xAE, 0x9E, 0x78, 0x30, 0xA7, /*0x74, 0xD3,*/ 0x00, 0x00,
	// Accelerometer
	// 0g x,y,z, 1g x,y,z, 2 byte checksum
	0x82, 0x82, 0x82, 0x15, 0x9C, 0x9C, 0x9E, 0x38, 0x40, 0x3E,
	0x82, 0x82, 0x82, 0x15, 0x9C, 0x9C, 0x9E, 0x38, 0x40, 0x3E
};

static const u8 motion_plus_id[] = { 0x00, 0x00, 0xA6, 0x20, 0x00, 0x05 };

static const u8 eeprom_data_16D0[] = {
	0x00, 0x00, 0x00, 0xFF, 0x11, 0xEE, 0x00, 0x00,
	0x33, 0xCC, 0x44, 0xBB, 0x00, 0x00, 0x66, 0x99,
	0x77, 0x88, 0x00, 0x00, 0x2B, 0x01, 0xE8, 0x13
};

struct ReportFeatures
{
	u8		core, accel, ir, ext, size;
} const reporting_mode_features[] = 
{
    //0x30: Core Buttons
	{ 2, 0, 0, 0, 4 },
    //0x31: Core Buttons and Accelerometer
	{ 2, 4, 0, 0, 7 },
    //0x32: Core Buttons with 8 Extension bytes
	{ 2, 0, 0, 4, 12 },
    //0x33: Core Buttons and Accelerometer with 12 IR bytes
	{ 2, 4, 7, 0, 19 },
    //0x34: Core Buttons with 19 Extension bytes
	{ 2, 0, 0, 4, 23 },
    //0x35: Core Buttons and Accelerometer with 16 Extension Bytes
	{ 2, 4, 0, 7, 23 },
    //0x36: Core Buttons with 10 IR bytes and 9 Extension Bytes
	{ 2, 0, 4, 14, 23 },
    //0x37: Core Buttons and Accelerometer with 10 IR bytes and 6 Extension Bytes
	{ 2, 4, 7, 17, 23 },
    //0x3d: 21 Extension Bytes
	{ 0, 0, 0, 2, 23 },
    //0x3e / 0x3f: Interleaved Core Buttons and Accelerometer with 36 IR bytes
	// UNSUPPORTED
	{ 0, 0, 0, 0, 23 },
};

void EmulateShake( u8* const accel
				  , ControllerEmu::Buttons* const buttons_group
				  , unsigned int* const shake_step )
{
	static const u8 shake_data[] = { 0x40, 0x01, 0x40, 0x80, 0xC0, 0xFF, 0xC0, 0x80 };
	static const unsigned int btns[] = { 0x01, 0x02, 0x04 };
	unsigned int shake = 0;

	buttons_group->GetState( &shake, btns );
	for ( unsigned int i=0; i<3; ++i )
		if (shake & (1 << i))
		{
			accel[i] = shake_data[shake_step[i]++];
			shake_step[i] %= sizeof(shake_data);
		}
		else
			shake_step[i] = 0;
}

void EmulateTilt( wm_accel* const accel
				 , ControllerEmu::Tilt* const tilt_group
				 , const accel_cal* const cal
				 , bool focus, bool sideways, bool upright)
{
	float roll, pitch;
	tilt_group->GetState( &roll, &pitch, 0, focus ? (PI / 2) : 0 ); // 90 degrees

	// this isn't doing anything with those low bits in the calib data, o well

	const u8* const zero_g = &cal->zero_g.x;
	s8 one_g[3];
	for ( unsigned int i=0; i<3; ++i )
		one_g[i] = (&cal->one_g.x)[i] - zero_g[i];

	unsigned int	ud = 0, lr = 0, fb = 0;

	// some notes that no one will understand but me :p

	// left, forward, up
	// lr/ left == negative for all orientations
	// ud/ up == negative for upright longways
	// fb/ forward == positive for (sideways flat)

	//if (sideways)
	//{
	//	if (upright)
	//	{
	//		ud = 0;
	//		lr = 1;
	//		fb = 2;
	//	}
	//	else
	//	{
	//		ud = 2;
	//		lr = 1;
	//		fb = 0;
	//		one_g[fb] *= -1;
	//	}
	//}
	//else
	//{
	//	if (upright)
	//	{
	//		ud = 1;
	//		lr = 0;
	//		fb = 2;
	//		one_g[ud] *= -1;
	//	}
	//	else
	//	{
	//		ud = 2;
	//		lr = 0;
	//		fb = 1;
	//	}
	//}

	// this is the above statements compacted
	ud = upright ? (sideways ? 0 : 1) : 2;
	lr = sideways;
	fb = upright ? 2 : (sideways ? 0 : 1);

	if (sideways && !upright)
		one_g[fb] *= -1;
	if (!sideways && upright)
		one_g[ud] *= -1;

	(&accel->x)[ud] = u8(sin( (PI / 2) - std::max( abs(roll), abs(pitch) ) ) * one_g[ud] + zero_g[ud]);
	(&accel->x)[lr] = u8(sin(roll) * -one_g[lr] + zero_g[lr]);
	(&accel->x)[fb] = u8(sin(pitch) * one_g[fb] + zero_g[fb]);
}

//void EmulateSwing()
//{
//
//}

const u16 button_bitmasks[] =
{
	WIIMOTE_A, WIIMOTE_B, WIIMOTE_ONE, WIIMOTE_TWO, WIIMOTE_MINUS, WIIMOTE_PLUS, WIIMOTE_HOME
};

const u16 dpad_bitmasks[] =
{
	WIIMOTE_PAD_UP, WIIMOTE_PAD_DOWN, WIIMOTE_PAD_LEFT, WIIMOTE_PAD_RIGHT
};
const u16 dpad_sideways_bitmasks[] =
{
	WIIMOTE_PAD_RIGHT, WIIMOTE_PAD_LEFT, WIIMOTE_PAD_UP, WIIMOTE_PAD_DOWN
};

const char* const named_buttons[] =
{
	"A",
	"B",
	"One",
	"Two",
	"Minus",
	"Plus",
	"Home",
};

void Wiimote::Reset()
{
	m_reporting_mode = WM_REPORT_CORE;
	// i think these two are good
	m_reporting_channel = 0;
	m_reporting_auto = false;

	m_rumble_on = false;
	m_speaker_mute = false;

	// will make the first Update() call send a status request
	// the first call to RequestStatus() will then set up the status struct extension bit
	m_extension->active_extension = -1;

	// eeprom
	memset( m_eeprom, 0, sizeof(m_eeprom) );
	// calibration data
	memcpy( m_eeprom, eeprom_data_0, sizeof(eeprom_data_0) );
	// dunno what this is for, copied from old plugin
	memcpy( m_eeprom + 0x16D0, eeprom_data_16D0, sizeof(eeprom_data_16D0) );

	// set up the register
	m_register.clear();
	m_register[0xa20000].resize(WIIMOTE_REG_SPEAKER_SIZE,0);
	m_register[0xa40000].resize(WIIMOTE_REG_EXT_SIZE,0);
	m_register[0xa60000].resize(WIIMOTE_REG_EXT_SIZE,0);
	m_register[0xB00000].resize(WIIMOTE_REG_IR_SIZE,0);

	m_reg_speaker		= (SpeakerReg*)&m_register[0xa20000][0];
	m_reg_ext			= (ExtensionReg*)&m_register[0xa40000][0];
	m_reg_motion_plus	= &m_register[0xa60000][0];
	m_reg_ir			= (IrReg*)&m_register[0xB00000][0];

	// testing
	//memcpy( m_reg_motion_plus + 0xfa, motion_plus_id, sizeof(motion_plus_id) );

	// status
	memset( &m_status, 0, sizeof(m_status) );
	// Battery levels in voltage
	//   0x00 - 0x32: level 1
	//   0x33 - 0x43: level 2
	//   0x33 - 0x54: level 3
	//   0x55 - 0xff: level 4
	m_status.battery = 0x5f;

	memset(m_shake_step, 0, sizeof(m_shake_step));
	memset(m_swing_step, 0, sizeof(m_swing_step));

	// clear read request queue
	while (m_read_requests.size())
	{
		delete[] m_read_requests.front().data;
		m_read_requests.pop();
	}
}

Wiimote::Wiimote( const unsigned int index )
	: m_index(index)
//	, m_sound_stream( NULL )
{
	// ---- set up all the controls ----

	// buttons
	groups.push_back( m_buttons = new Buttons( "Buttons" ) );
	for ( unsigned int i=0; i < sizeof(named_buttons)/sizeof(*named_buttons); ++i )
		m_buttons->controls.push_back( new ControlGroup::Input( named_buttons[i] ) );

	// ir
	groups.push_back( m_ir = new Cursor( "IR", &g_WiimoteInitialize ) );

	// tilt
	groups.push_back( m_tilt = new Tilt( "Tilt" ) );

	// swing
	//groups.push_back( m_swing = new Force( "Swing" ) );

	// shake
	groups.push_back( m_shake = new Buttons( "Shake" ) );
	m_shake->controls.push_back( new ControlGroup::Input( "X" ) );
	m_shake->controls.push_back( new ControlGroup::Input( "Y" ) );
	m_shake->controls.push_back( new ControlGroup::Input( "Z" ) );

	// extension
	groups.push_back( m_extension = new Extension( "Extension" ) );
	m_extension->attachments.push_back( new WiimoteEmu::None() );
	m_extension->attachments.push_back( new WiimoteEmu::Nunchuk() );
	m_extension->attachments.push_back( new WiimoteEmu::Classic() );
	m_extension->attachments.push_back( new WiimoteEmu::Guitar() );
	m_extension->attachments.push_back( new WiimoteEmu::Drums() );

	// rumble
	groups.push_back( m_rumble = new ControlGroup( "Rumble" ) );
	m_rumble->controls.push_back( new ControlGroup::Output( "Motor" ) );

	// dpad
	groups.push_back( m_dpad = new Buttons( "D-Pad" ) );
	for ( unsigned int i=0; i < 4; ++i )
		m_dpad->controls.push_back( new ControlGroup::Input( named_directions[i] ) );

	// options
	groups.push_back( m_options = new ControlGroup( "Options" ) );
	m_options->settings.push_back( new ControlGroup::Setting( "Background Input", false ) );
	m_options->settings.push_back( new ControlGroup::Setting( "Sideways Wiimote", false ) );
	m_options->settings.push_back( new ControlGroup::Setting( "Upright Wiimote", false ) );
	
#ifdef USE_WIIMOTE_EMU_SPEAKER
	// set up speaker stuff
	// this doesnt belong here

	// TODO: i never clean up any of this audio stuff

	if (0 == m_index)	// very dumb
	{
		ALCdevice* pDevice;
		ALchar DeviceName[] = "DirectSound3D";
		pDevice = alcOpenDevice(DeviceName);
		ALCcontext* pContext;
		pContext = alcCreateContext(pDevice, NULL);
		alcMakeContextCurrent(pContext);
	}

	alListener3f(AL_POSITION, 0.0, 0.0, 0.0);
	alListener3f(AL_VELOCITY, 0.0, 0.0, 0.0);
	alListener3f(AL_DIRECTION, 0.0, 0.0, 0.0);

	alGenSources(1, &m_audio_source);
	alSourcef(m_audio_source, AL_PITCH, 1.0);
	alSourcef(m_audio_source, AL_GAIN, 1.0);
	alSourcei(m_audio_source, AL_LOOPING, false);
#endif

	// --- reset eeprom/register/values to default ---
	Reset();
}

std::string Wiimote::GetName() const
{
	return std::string("Wiimote") + char('1'+m_index);
}

void Wiimote::Update()
{
	const bool is_sideways = m_options->settings[1]->value > 0;
	const bool is_upright = m_options->settings[2]->value > 0; 

	// if windows is focused or background input is enabled
	const bool is_focus = g_WiimoteInitialize.pRendererHasFocus() || (m_options->settings[0]->value != 0);

	// no rumble if no focus
	if (false == is_focus)
		m_rumble_on = false;
	m_rumble->controls[0]->control_ref->State(m_rumble_on);

	// ----speaker----
#ifdef USE_WIIMOTE_EMU_SPEAKER

	ALint processed = 0;
	alGetSourcei(m_audio_source, AL_BUFFERS_PROCESSED, &processed);

	while (processed--)
	{
		//PanicAlert("Buffer Processed");
		alSourceUnqueueBuffers(m_audio_source, 1, &m_audio_buffers.front().buffer);
		alDeleteBuffers(1, &m_audio_buffers.front().buffer);
		delete[] m_audio_buffers.front().samples;
		m_audio_buffers.pop();
	}

	// testing speaker crap
	//m_rumble->controls[0]->control_ref->State( m_speaker_data.size() > 0 );
	//if ( m_speaker_data.size() )
		//m_speaker_data.pop();

	//while ( m_speaker_data.size() )
	//{
	//	std::ofstream file;
	//	file.open( "test.pcm", std::ios::app | std::ios::out | std::ios::binary );
	//	file.put(m_speaker_data.front());
	//	file.close();
	//	m_speaker_data.pop();
	//}
#endif

	// update buttons in status struct
	m_status.buttons = 0;
	if (is_focus)
	{
		m_buttons->GetState( &m_status.buttons, button_bitmasks );
		m_dpad->GetState( &m_status.buttons, is_sideways ? dpad_sideways_bitmasks : dpad_bitmasks );
	}

	// check if there is a read data request
	if (m_read_requests.size())
	{
		ReadRequest& rr = m_read_requests.front();
		// send up to 16 bytes to the wii
		SendReadDataReply(m_reporting_channel, rr);
		//SendReadDataReply(rr.channel, rr);

		// if there is no more data, remove from queue
		if (0 == rr.size)
		{
			delete[] rr.data;
			m_read_requests.pop();
		}

		// dont send any other reports
		return;
	}

	// -- maybe this should happen before the read request stuff?
	// check if a status report needs to be sent
	// this happens on wiimote sync and when extensions are switched
	if (m_extension->active_extension != m_extension->switch_extension)
	{
		RequestStatus(m_reporting_channel);

		// Wiibrew: Following a connection or disconnection event on the Extension Port,
		// data reporting is disabled and the Data Reporting Mode must be reset before new data can arrive.
		// after a game receives an unrequested status report,
		// it expects data reports to stop until it sets the reporting mode again
		m_reporting_auto = false;
	}

	if (false == m_reporting_auto)
		return;

	// figure out what data we need
	const ReportFeatures& rpt = reporting_mode_features[m_reporting_mode - WM_REPORT_CORE];

	// what does the real wiimote do when put in a reporting mode with extension data,
	// but with no extension attached? should i just send zeros? sure
	//if (rpt.ext && (m_extension->active_extension <= 0))
	//{
	//	m_reporting_auto = false;
	//	return;
	//}

	// set up output report
	// made data bigger than needed in case the wii specifies the wrong ir mode for a reporting mode
	u8 data[46];
	memset( data, 0, sizeof(data) );

	data[0] = 0xA1;
	data[1] = m_reporting_mode;

	// core buttons
	if (rpt.core)
		*(wm_core*)(data + rpt.core) = m_status.buttons;

	// ----accelerometer----
	if (rpt.accel)
	{
		// ----TILT----
		EmulateTilt((wm_accel*)&data[rpt.accel], m_tilt, (accel_cal*)&m_eeprom[0x16], is_focus, is_sideways, is_upright );

		// ----SWING----
		//const s8 swing_data[] = { 0x20, 0x40, 0x20, 0x00 };
		//u8 swing[3];
		//m_swing->GetState( swing, 0x80, 0x40 );

		//// up/down
		//if (swing[0] != 0x80)
		//{
		//	//data[rpt.accel + 0] = swing[0];
		//	data[rpt.accel + 2] += swing_data[m_swing_step[0]/4];
		//	if (m_swing_step[0] < 12)
		//		++m_swing_step[0];
		//}
		//else
		//	m_swing_step[0] = 0;

		//// left/right
		//if (swing[1] != 0x80)
		//	data[rpt.accel + !is_sideways] = swing[1];

		//// forward/backward
		//if (swing[2] != 0x80)
		//	data[rpt.accel + is_sideways] = swing[2];

		// ----SHAKE----
		if (is_focus)
			EmulateShake(data + rpt.accel, m_shake, m_shake_step);

	}

	// ----extension----
	if (rpt.ext)
	{
		m_extension->GetState(data + rpt.ext, is_focus);

		// i dont think anything accesses the extension data like this, but ill support it
		// i think it should be unencrpyted in the register, encrypted when read
		memcpy(m_reg_ext->controller_data, data + rpt.ext, sizeof(wm_extension));

		// both of these ifs work
		//if (0x55 != m_reg_ext->encryption)
		if (0xAA == m_reg_ext->encryption)
			wiimote_encrypt(&m_ext_key, data + rpt.ext, 0x00, sizeof(wm_extension));
	}

	// ----ir----
	// only if camera is fully enabled.
	// should send 0xFF if camera isn't enabled maybe,
	// 0x00 is working fine though
	if (rpt.ir && 0x08 == m_reg_ir->data[0x30])
	{
		float xx = 10000, yy = 0, zz = 0;

		if (is_focus)
			m_ir->GetState(&xx, &yy, &zz, true);

		xx *= (-256 * 0.95f);
		xx += 512;

		yy *= (-256 * 0.90f);
		yy += 490;

		const unsigned int distance = (unsigned int)(100 + 100 * zz);

		// TODO: make roll affect the dot positions
		const unsigned int y = (unsigned int)yy;

		unsigned int x[4];
		x[0] = (unsigned int)(xx - distance);
		x[1] = (unsigned int)(xx + distance);
		x[2] = (unsigned int)(xx - 1.2f * distance);
		x[3] = (unsigned int)(xx + 1.2f * distance);

		// ir mode
		switch (m_reg_ir->mode)
		{
		// basic
		case 1 :
			{
			memset(data + rpt.ir, 0xFF, 10);
			wm_ir_basic* const irdata = (wm_ir_basic*)(data + rpt.ir);
			if (y < 768)
			{
				for ( unsigned int i=0; i<2; ++i )
				{
					if (x[i*2] < 1024)
					{
						irdata[i].x1 = u8(x[i*2]);
						irdata[i].x1hi = x[i*2] >> 8;

						irdata[i].y1 = u8(y);
						irdata[i].y1hi = y >> 8;
					}
					if (x[i*2+1] < 1024)
					{
						irdata[i].x2 = u8(x[i*2+1]);
						irdata[i].x2hi = x[i*2+1] >> 8;

						irdata[i].y2 = u8(y);
						irdata[i].y2hi = y >> 8;
					}
				}
			}
			}
			break;
		// extended
		case 3 :
			{
			memset(data + rpt.ir, 0xFF, 12);
			wm_ir_extended* const irdata = (wm_ir_extended*)(data + rpt.ir);
			if (y < 768)
			{
				for ( unsigned int i=0; i<4; ++i )
					if (x[i] < 1024)
					{
						irdata[i].x = u8(x[i]);
						irdata[i].xhi = x[i] >> 8;

						irdata[i].y = u8(y);
						irdata[i].yhi = y >> 8;

						irdata[i].size = 10;
					}
			}
			}
			break;
		// full
		case 5 :
			// UNSUPPORTED
			break;

		}
	}

	// send data report
	g_WiimoteInitialize.pWiimoteInput( m_index, m_reporting_channel, data, rpt.size );
}

void Wiimote::ControlChannel(const u16 _channelID, const void* _pData, u32 _Size) 
{

	// Check for custom communication
	if (99 == _channelID)
	{
		// wiimote disconnected
		//PanicAlert( "Wiimote Disconnected" );

		// reset eeprom/register/reporting mode
		Reset();
		return;
	}

	hid_packet* hidp = (hid_packet*)_pData;

	INFO_LOG(WIIMOTE, "Emu ControlChannel (page: %i, type: 0x%02x, param: 0x%02x)", m_index, hidp->type, hidp->param);

	switch(hidp->type)
	{
	case HID_TYPE_HANDSHAKE :
		PanicAlert("HID_TYPE_HANDSHAKE - %s", (hidp->param == HID_PARAM_INPUT) ? "INPUT" : "OUPUT");
		break;

	case HID_TYPE_SET_REPORT :
		if (HID_PARAM_INPUT == hidp->param)
		{
			PanicAlert("HID_TYPE_SET_REPORT - INPUT"); 
		}
		else
		{
			// AyuanX: My experiment shows Control Channel is never used
			// shuffle2: but homebrew uses this, so we'll do what we must :)
			HidOutputReport(_channelID, (wm_report*)hidp->data);

			u8 handshake = HID_HANDSHAKE_SUCCESS;
			g_WiimoteInitialize.pWiimoteInput(m_index, _channelID, &handshake, 1);

			PanicAlert("HID_TYPE_DATA - OUTPUT: Ambiguous Control Channel Report!");
		}
		break;

	case HID_TYPE_DATA :
		PanicAlert("HID_TYPE_DATA - %s", (hidp->param == HID_PARAM_INPUT) ? "INPUT" : "OUTPUT");
		break;

	default :
		PanicAlert("HidControlChannel: Unknown type %x and param %x", hidp->type, hidp->param);
		break;
	}

}

void Wiimote::InterruptChannel(const u16 _channelID, const void* _pData, u32 _Size)
{
	hid_packet* hidp = (hid_packet*)_pData;

	switch (hidp->type)
	{
	case HID_TYPE_DATA:
		switch (hidp->param)
		{
		case HID_PARAM_OUTPUT :
			{
				wm_report* sr = (wm_report*)hidp->data;
				HidOutputReport(_channelID, sr);
			}
			break;

		default :
			PanicAlert("HidInput: HID_TYPE_DATA - param 0x%02x", hidp->type, hidp->param);
			break;
		}
		break;

	default:
		PanicAlert("HidInput: Unknown type 0x%02x and param 0x%02x", hidp->type, hidp->param);
		break;
	}
}

// TODO: i need to test this
void Wiimote::Register::Read( size_t address, void* dst, size_t length )
{
	const_iterator i = begin();
	const const_iterator e = end();
	while (length)
	{
		const std::vector<u8>* block = NULL;
		size_t addr_start = 0;
		size_t addr_end = address+length;

		// find block and start of next block
		for ( ; i!=e; ++i )
			// if address is inside or after this block
			if ( address >= i->first )
			{
				block = &i->second;
				addr_start = i->first;
			}
			// if address is before this block
			else
			{
				// how far til the start of the next block
				addr_end = std::min( i->first, addr_end );
				break;
			}

		// read bytes from a mapped block
		if (block)
		{
			// offset of wanted data in the vector
			const size_t offset = std::min( address - addr_start, block->size() );
			// how much data we can read depending on the vector size and how much we want
			const size_t amt = std::min( block->size()-offset, length );

			memcpy( dst, &block->operator[](offset), amt );
			
			address += amt;
			dst = ((u8*)dst) + amt;
			length -= amt;
		}

		// read zeros for unmapped regions
		const size_t amt = addr_end - address;

		memset( dst, 0, amt );

		address += amt;
		dst = ((u8*)dst) + amt;
		length -= amt;
	}
}

// TODO: i need to test this
void Wiimote::Register::Write( size_t address, void* src, size_t length )
{
	iterator i = begin();
	const const_iterator e = end();
	while (length)
	{
		std::vector<u8>* block = NULL;
		size_t addr_start = 0;
		size_t addr_end = address+length;

		// find block and start of next block
		for ( ; i!=e; ++i )
			// if address is inside or after this block
			if ( address >= i->first )
			{
				block = &i->second;
				addr_start = i->first;
			}
			// if address is before this block
			else
			{
				// how far til the start of the next block
				addr_end = std::min( i->first, addr_end );
				break;
			}

		// write bytes to a mapped block
		if (block)
		{
			// offset of wanted data in the vector
			const size_t offset = std::min( address - addr_start, block->size() );
			// how much data we can read depending on the vector size and how much we want
			const size_t amt = std::min( block->size()-offset, length );

			memcpy( &block->operator[](offset), src, amt );
			
			address += amt;
			src = ((u8*)src) + amt;
			length -= amt;
		}

		// do nothing for unmapped regions
		const size_t amt = addr_end - address;

		address += amt;
		src = ((u8*)src) + amt;
		length -= amt;
	}
}

}

