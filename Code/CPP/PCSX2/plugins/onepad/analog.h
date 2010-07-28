/*  OnePAD - author: arcum42(@gmail.com)
 *  Copyright (C) 2009
 *
 *  Based on ZeroPAD, author zerofrog@gmail.com
 *  Copyright (C) 2006-2007
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

 #define NUM_OF_PADS 2

 #include "onepad.h"
 namespace Analog
{
	extern void Init();
	extern u8 Pad(int pad, u8 index);
	extern void SetPad(u8 pad, int index, u8 value);
	extern void InvertPad(u8 pad, int key);
	extern bool ReversePad(u8 index);
	extern void ResetPad( u8 pad, int key);
	extern void ConfigurePad( u8 pad, int index, int value);
	extern int KeypadToPad(u8 keypress);
	extern int AnalogToPad(int index);
}
