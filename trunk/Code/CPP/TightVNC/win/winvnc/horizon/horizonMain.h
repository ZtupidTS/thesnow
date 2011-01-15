
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonMain.h 2009 2005-05-05 19:01:23Z jeffo3 $

#ifndef __APPSHAREMAIN_H
#define __APPSHAREMAIN_H

#pragma warning( disable : 4786 )

// stl first
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

using std::ofstream ;
using std::string ;
using std::vector ;

#include "stdhdrs.h"
#include "resource.h"

#include "VSocket.h"
#include "Log.h"

#include "vncInstHandler.h"
#include "vncServer.h"
#include "vncService.h"

#include "horizonMenu.h"
#include "horizonDefaults.h"
#include "horizonProperties.h"
#include "horizonGlobals.h"

//
// functions declarations
//

int AppShareMain( const string& args ) ;

bool ConnectToServer( const string& args ) ; 

void AppShareUsage( const string& args ) ;


#endif // __APPSHAREMAIN_H
