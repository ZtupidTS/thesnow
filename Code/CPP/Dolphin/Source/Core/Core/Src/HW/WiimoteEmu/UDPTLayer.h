//UDP Wiimote Translation Layer

#if (defined(USE_UDP_WIIMOTE) && !defined(UDPTLAYER_H))
#define UDPTLAYER_H

#include "UDPWiimote.h"
#include "WiimoteEmu.h"

using WiimoteEmu::Wiimote;

namespace UDPTLayer
{
	void GetButtons(UDPWrapper * m , wm_core * butt)
	{
		if (!(m->inst)) return;
		if (!(m->updButt)) return;
		u32 mask=m->inst->getButtons();
		*butt|=(mask&UDPWM_BA)?Wiimote::BUTTON_A:0;
		*butt|=(mask&UDPWM_BB)?Wiimote::BUTTON_B:0;
		*butt|=(mask&UDPWM_B1)?Wiimote::BUTTON_ONE:0;
		*butt|=(mask&UDPWM_B2)?Wiimote::BUTTON_TWO:0;
		*butt|=(mask&UDPWM_BP)?Wiimote::BUTTON_PLUS:0;
		*butt|=(mask&UDPWM_BM)?Wiimote::BUTTON_MINUS:0;
		*butt|=(mask&UDPWM_BH)?Wiimote::BUTTON_HOME:0;
		*butt|=(mask&UDPWM_BU)?Wiimote::PAD_UP:0;
		*butt|=(mask&UDPWM_BD)?Wiimote::PAD_DOWN:0;
		*butt|=(mask&UDPWM_BL)?Wiimote::PAD_LEFT:0;
		*butt|=(mask&UDPWM_BR)?Wiimote::PAD_RIGHT:0;
	}

	void GetAcceleration(UDPWrapper * m , WiimoteEmu::AccelData * const data)
	{
		if (!(m->inst)) return;
		if (!(m->updAccel)) return;
		float x,y,z;
		m->inst->getAccel(x,y,z);
		data->x=x;
		data->y=y;
		data->z=z;
	}

	void GetIR( UDPWrapper * m, float * x,  float * y,  float * z)
	{
		if (!(m->inst)) return;
		if (!(m->updIR)) return;
		if ((*x>=-0.999)&&(*x<=0.999)&&(*y>=-0.999)&&(*y<=0.999)) return; //the recieved values are used ONLY when the normal pointer is offscreen
		float _x,_y;
		m->inst->getIR(_x,_y);
		*x=_x*2-1;
		*y=-(_y*2-1);
		*z=0;
	}
	
}

#endif
