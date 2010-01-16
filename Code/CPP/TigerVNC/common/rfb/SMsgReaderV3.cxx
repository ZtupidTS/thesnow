/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2009 Pierre Ossman for Cendio AB
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#include <stdio.h>
#include <rfb/PixelFormat.h>
#include <rfb/msgTypes.h>
#include <rfb/Exception.h>
#include <rdr/InStream.h>
#include <rfb/SMsgReaderV3.h>
#include <rfb/SMsgHandler.h>
#include <rfb/ScreenSet.h>

using namespace rfb;

SMsgReaderV3::SMsgReaderV3(SMsgHandler* handler, rdr::InStream* is)
  : SMsgReader(handler, is)
{
}

SMsgReaderV3::~SMsgReaderV3()
{
}

void SMsgReaderV3::readClientInit()
{
  bool shared = is->readU8();
  handler->clientInit(shared);
}

void SMsgReaderV3::readMsg()
{
  int msgType = is->readU8();
  switch (msgType) {
  case msgTypeSetPixelFormat:           readSetPixelFormat(); break;
  case msgTypeSetEncodings:             readSetEncodings(); break;
  case msgTypeFramebufferUpdateRequest: readFramebufferUpdateRequest(); break;
  case msgTypeKeyEvent:                 readKeyEvent(); break;
  case msgTypePointerEvent:             readPointerEvent(); break;
  case msgTypeClientCutText:            readClientCutText(); break;
  case msgTypeSetDesktopSize:           readSetDesktopSize(); break;

  default:
    fprintf(stderr, "unknown message type %d\n", msgType);
    throw Exception("unknown message type");
  }
}

void SMsgReaderV3::readSetDesktopSize()
{
  int width, height;
  int screens, i;
  rdr::U32 id, flags;
  int sx, sy, sw, sh;
  ScreenSet layout;

  is->skip(1);

  width = is->readU16();
  height = is->readU16();

  screens = is->readU8();
  is->skip(1);

  for (i = 0;i < screens;i++) {
    id = is->readU32();
    sx = is->readU16();
    sy = is->readU16();
    sw = is->readU16();
    sh = is->readU16();
    flags = is->readU32();

    layout.add_screen(Screen(id, sx, sy, sw, sh, flags));
  }

  handler->setDesktopSize(width, height, layout);
}

