// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/
//
// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/
//

#include <iostream> // System
#include <vector>
#include <cmath>
#include "Common.h" // Common

#if defined HAVE_WX && HAVE_WX
	#include <wx/wx.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace InputCommon
{

// Degree to radian and back
float Deg2Rad(float Deg)
{
	return Deg * (float)M_PI / 180.0f;
}
float Rad2Deg(float Rad)
{
	return Rad * 180.0f / (float)M_PI;
}

// Check if the pad is within the dead zone, we assume the range is 0x8000
float CoordinatesToRadius(int x, int y)
{
	return sqrt(pow((float)x, 2) +  pow((float)y, 2));
}

bool IsDeadZone(float DeadZone, int x, int y)
{
	// Get the distance from the center
	float Distance = CoordinatesToRadius(x, y) / 32767.0f;

	//Console::Print("%f\n", Distance);

	// Check if it's within the dead zone
	if (Distance <= DeadZone)
		return true;
	else
		return false;
}

// Scale down stick values from 0x8000 to 0x80
/*
   The value returned by SDL_JoystickGetAxis is a signed integer s16 (-32768 to
   32767). The value used for the gamecube controller is an unsigned char u8 (0
   to 255) with neutral at 0x80 (128), so that it's equivalent to a signed -128
   to 127.  */
// 
int Pad_Convert(int _val)
{

	/* If the limits on PadState[].axis[] actually is a u16 then we don't need
	   this but if it's not actually limited to that we need to apply these
	   limits */
	if(_val > 32767) _val = 32767; // upper limit
	if(_val < -32768) _val = -32768; // lower limit
		
	// Convert the range (-0x8000 to 0x7fff) to (0 to 0xffff)
	_val = 0x8000 +_val;

	// Convert the range (0 to 0xffff) to (0 to 0xff)
	_val = _val >> 8;

	//Console::Print("0x%04x  %06i\n\n", _val, _val);

	return _val;
}

// Adjust the radius
void RadiusAdjustment(s8 &_x, s8 &_y, int _Radius)
{
	// Get the radius setting
	float RadiusSetting = (float)_Radius / 100.0f;
	float x = (float)_x * RadiusSetting;
	float y = (float)_y * RadiusSetting;
	// Update values
	_x = (s8)x;
	_y = (s8)y;
}

/* Convert the stick raidus from a square or rounded box to a circular
   radius. I don't know what input values the actual GC controller produce for
   the GC, it may be a square, a circle or something in between. But one thing
   that is certain is that PC pads differ in their output (as shown in the list
   below), so it may be beneficiary to convert whatever radius they produce to
   the radius the GC games expect. This is the first implementation of this
   that convert a square radius to a circual radius. Use the advanced settings
   to enable and calibrate it.

   Observed diagonals:
	   Perfect circle: 71% = sin(45)
	   Logitech Dual Action: 100%
	   PS2 Dual Shock 2 (Original) with Super Dual Box Pro: 90%
	   XBox 360 Wireless: 85%
	   GameCube Controller (Third Party) with EMS Trio Linker Plus II: 60%
*/

/* Calculate the distance from the outer edges of the box to the outer edges of
   the circle inside the box at any angle from 0 to 360. The returned value is
   1 + Distance, for example at most sqrt(2) in the corners and at least 1.0 at
   the horizontal and vertical angles. */
float Square2CircleDistance(float deg)
{
	// See if we have to adjust the angle
	deg = abs(deg);
	if( (deg > 45 && deg < 135) ) deg = deg - 90;

	// Calculate distance from center
	float val = abs(cos(Deg2Rad(deg)));
	float Distance = 1 / val;

	//m_frame->m_pStatusBar2->SetLabel(wxString::Format("Deg:%f  Val:%f  Dist:%f", deg, val, dist));

	return Distance;
}

// Produce a perfect circle from an original square or rounded box
void Square2Circle(int &_x, int &_y, int _Diagonal, bool Circle2Square)
{
	// Do we need to do this check?
	if(_x >  32767) _x =  32767; if(_y >  32767) _y =  32767; // upper limit
	if(_x < -32768) _x = -32768; if(_y < -32768) _y = -32768; // lower limit

	// Convert to circle
	// Get the manually configured diagonal distance
	float Diagonal = (float)_Diagonal / 100.0f;

	// First make a perfect square in case we don't have one already
	float OrigDist = sqrt(  pow((float)_y, 2) + pow((float)_x, 2)  ); // Get current distance
	float deg = Rad2Deg(atan2((float)_y, (float)_x)); // Get current angle

	/* Calculate the actual distance between the maxium diagonal values, and
	   the outer edges of the square. A diagonal of 85% means a maximum
	   distance of 0.85 * sqrt(2) ~1.2 in the diagonals. */
	float corner_circle_dist = ( Diagonal / sin(Deg2Rad(45.0f)) );
	float SquareDist = Square2CircleDistance(deg);
	// The original-to-square distance adjustment
	float adj_ratio1;
	// The circle-to-square distance adjustment
	float adj_ratio2 = SquareDist;
	 // The resulting distance
	float result_dist;

	// Calculate the corner-to-square adjustment ratio
	if(corner_circle_dist < SquareDist) adj_ratio1 = SquareDist / corner_circle_dist;
		else adj_ratio1 = 1;

	// Calculate the resulting distance
	if(Circle2Square)
		result_dist = OrigDist * adj_ratio1;
	else
		result_dist = OrigDist * adj_ratio1 / adj_ratio2;
	
	// Calculate x and y and return it
	float x = result_dist * cos(Deg2Rad(deg));
	float y = result_dist * sin(Deg2Rad(deg));
	// Update coordinates
	_x = (int)floor(x);
	_y = (int)floor(y);
	// Boundaries
	if (_x < -32768) _x = -32768; if (_x > 32767) _x = 32767;
	if (_y < -32768) _y = -32768; if (_y > 32767) _y = 32767;

	// Debugging
	//Console::Print("%f  %f  %i", corner_circle_dist, Diagonal, Tmp));
}

// Windows Virtual Key Codes Names
#ifdef _WIN32
std::string VKToString(int keycode)
{
	// Default value
	char KeyStr[64] = {0};
	std::string KeyString;

	switch(keycode)
	{
		// Give it some help with a few keys
		case VK_END: return "END";
		case VK_INSERT: return "INS";
		case VK_DELETE: return "DEL";
		case VK_HOME: return "HOME";
		case VK_PRIOR: return "PGUP";
		case VK_NEXT: return "PGDN";

		case VK_UP: return "UP";
		case VK_DOWN: return "DOWN";
		case VK_LEFT: return "LEFT";
		case VK_RIGHT: return "RIGHT";

		case VK_LSHIFT: return "Left Shift";
		case VK_RSHIFT: return "Right Shift";
		case VK_LCONTROL: return "Left Ctrl";
		case VK_RCONTROL: return "Right Ctrl";
		case VK_LMENU: return "Left Alt";
		case VK_RMENU: return "Right Alt";

		case VK_NUMLOCK: return "Num Lock";
		case VK_MULTIPLY: return "Num *";
		case VK_ADD: return "Num +";
		case VK_SEPARATOR: case 0xC2: return "Num Separator";
		case VK_SUBTRACT: return "Num -";
		case VK_DECIMAL: return "Num Decimal";
		case VK_DIVIDE: return "Num /";

		case VK_OEM_PLUS: return "=";
		case VK_OEM_MINUS: return "-";
		case VK_OEM_COMMA: return ",";
		case VK_OEM_PERIOD: return ".";

		//default: return KeyString = KeyStr;
	}
	
	// TODO: Switch to unicode GetKeyNameText?
	if (keycode < 256)	// Keyboard
		GetKeyNameTextA(MapVirtualKey(keycode, MAPVK_VK_TO_VSC) << 16, KeyStr, 64);
	else	// Pad
		sprintf(KeyStr, "PAD: %d", keycode - 0x1000);
	
	return KeyString = KeyStr;
}
#endif


}

