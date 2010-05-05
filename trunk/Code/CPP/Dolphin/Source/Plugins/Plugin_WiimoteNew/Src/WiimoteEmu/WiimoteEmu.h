#ifndef _CONEMU_WIIMOTE_H_
#define _CONEMU_WIIMOTE_H_

//#define USE_WIIMOTE_EMU_SPEAKER

// just used to get the OpenAL includes :p
//#include <OpenALStream.h>

#include <ControllerEmu.h>

#include "WiimoteHid.h"
#include "Encryption.h"

#include <vector>
#include <queue>

#define PI	3.14159265358979323846

// Registry sizes 
#define WIIMOTE_EEPROM_SIZE			(16*1024)
#define WIIMOTE_EEPROM_FREE_SIZE	0x1700
#define WIIMOTE_REG_SPEAKER_SIZE	10
#define WIIMOTE_REG_EXT_SIZE		0x100
#define WIIMOTE_REG_IR_SIZE			0x34

extern SWiimoteInitialize g_WiimoteInitialize;

namespace WiimoteEmu
{

void EmulateShake( u8* const accel_data
				  , ControllerEmu::Buttons* const buttons_group
				  , unsigned int* const shake_step );

void EmulateTilt( wm_accel* const accel
				 , ControllerEmu::Tilt* const tilt_group
				 , const accel_cal* const cal
				 , bool focus, bool sideways = false, bool upright = false);

class Wiimote : public ControllerEmu
{
public:
	Wiimote( const unsigned int index );
	std::string GetName() const;

	void Update();
	void InterruptChannel(const u16 _channelID, const void* _pData, u32 _Size);
	void ControlChannel(const u16 _channelID, const void* _pData, u32 _Size);

private:
	struct ReadRequest
	{
		unsigned int	address, size, position;
		u8*		data;
	};

	void Reset();

	void ReportMode(const u16 _channelID, wm_report_mode* dr);
	void HidOutputReport(const u16 _channelID, wm_report* sr);
	void SendAck(const u16 _channelID, u8 _reportID);
	void RequestStatus(const u16 _channelID, wm_request_status* rs = NULL);

	void WriteData(const u16 _channelID, wm_write_data* wd);
	void ReadData(const u16 _channelID, wm_read_data* rd);
	void SendReadDataReply(const u16 _channelID, ReadRequest& _request);

#ifdef USE_WIIMOTE_EMU_SPEAKER
	void SpeakerData(wm_speaker_data* sd);
#endif

	// control groups
	Buttons*				m_buttons;
	Buttons*				m_dpad;
	Buttons*				m_shake;
	Cursor*					m_ir;
	Tilt*					m_tilt;
	Force*					m_swing;
	ControlGroup*			m_rumble;
	Extension*				m_extension;
	ControlGroup*			m_options;

	// wiimote index, 0-3
	const unsigned int		m_index;

	bool		m_rumble_on;
	bool		m_speaker_mute;

	bool					m_reporting_auto;
	unsigned int			m_reporting_mode;
	unsigned int			m_reporting_channel;

	// hax
	unsigned int			m_skip_update;

	unsigned int			m_shake_step[3];
	unsigned int			m_swing_step[3];

	wm_status_report		m_status;

	class Register : public std::map< size_t, std::vector<u8> >
	{
	public:
		void Write( size_t address, void* src, size_t length );
		void Read( size_t address, void* dst, size_t length );

	} m_register;

	// read data request queue
	// maybe it isn't actualy a queue
	// maybe read requests cancel any current requests
	std::queue< ReadRequest >	m_read_requests;

#ifdef USE_WIIMOTE_EMU_SPEAKER
	// speaker stuff
	struct SoundBuffer
	{
		s16* samples;
		ALuint buffer;
	};
	std::queue<SoundBuffer>	m_audio_buffers;
	ALuint					m_audio_source;
	ADPCMChannelStatus		m_channel_status;
#endif

	u8		m_eeprom[WIIMOTE_EEPROM_SIZE];

	u8*		m_reg_motion_plus;

	struct IrReg
	{
		u8	unknown1[0x33];
		u8	mode;

	}	*m_reg_ir;

	struct ExtensionReg
	{
		u8	unknown1[0x08];

		// address 0x08
		u8	controller_data[0x06];
		u8	unknown2[0x12];

		// address 0x20
		u8	calibration[0x10];
		u8	unknown3[0x10];

		// address 0x40
		u8	encryption_key[0x10];
		u8	unknown4[0xA0];

		// address 0xF0
		u8	encryption;
		u8	unknown5[0x9];

		// address 0xFA
		u8	constant_id[6];

	}	*m_reg_ext;

	struct SpeakerReg
	{
		u16		unknown;
		u8		format;
		u16		sample_rate;
		u8		volume;
		u8		unk[4];

	}	*m_reg_speaker;

	wiimote_key		m_ext_key;
};

}

#endif
