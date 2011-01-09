#ifndef NUNCHUCK_H
#define NUNCHUCK_H

#include "Attachment.h"

class UDPWrapper;

namespace WiimoteEmu
{

class Nunchuk : public Attachment
{
public:
	Nunchuk(UDPWrapper * wrp);

	virtual void GetState( u8* const data, const bool focus );

	enum
	{
		BUTTON_C = 0x02,
		BUTTON_Z = 0x01,
	};

private:
	Tilt*			m_tilt;
	Force*			m_swing;

	Buttons*		m_shake;

	Buttons*		m_buttons;
	AnalogStick*	m_stick;

	u8	m_shake_step[3];
	
	UDPWrapper* const m_udpWrap;
};

}

#endif
