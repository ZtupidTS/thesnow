# Microsoft Developer Studio Project File - Name="AppShare" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=AppShare - Win32 TightVNC
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AppShare.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AppShare.mak" CFG="AppShare - Win32 TightVNC"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AppShare - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "AppShare - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "AppShare - Win32 TightVNC" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AppShare - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "AppShare___Win32_HorizonLive"
# PROP BASE Intermediate_Dir "AppShare___Win32_HorizonLive"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./HorizonLive"
# PROP Intermediate_Dir "./HorizonLive"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /I "..\.." /I "$(WINVNC_HOME)\VNCHooks" /I "$(WINVNC_HOME)\libjpeg" /I "$(WINVNC_HOME)\omnithread" /I "$(WINVNC_HOME)\zlib" /D "__WIN32__" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_OMNITHREAD_DLL" /FD /GZ /c
# SUBTRACT BASE CPP /X /u /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /I "./lib" /I ".." /I "../.." /I "../omnithread" /D WINVER=0x0500 /D _WIN32_WINNT=0x0500 /D "__WIN32__" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_OMNITHREAD_DLL" /D "HORIZONLIVE" /D "USE_ALT_SOCKETS" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib wsock32.lib omnithread_rtd.lib libjpeg.lib VNCHooks.lib zlib.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /pdbtype:sept /libpath:"." /libpath:".." /libpath:"$(WINVNC_HOME)" /libpath:"$(WINVNC_HOME)\Debug"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 comctl32.lib htmlhelp.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib wsock32.lib shlwapi.lib omnithread_rt.lib libjpeg.a libz.a wininet.lib VNCHOOKS.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"C:\Documents and Settings\Administrator\Application Data\HorizonWimba\JSecureDoor\appshare_0.0.1\data\Release-AppShare.exe" /libpath:"./lib/libjpeg" /libpath:"./lib/zlib" /libpath:"./HorizonLive"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "AppShare - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "AppShare___Win32_HorizonLiveDebug"
# PROP BASE Intermediate_Dir "AppShare___Win32_HorizonLiveDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./HorizonLiveDebug"
# PROP Intermediate_Dir "./HorizonLiveDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Od /I ".." /I "../.." /I "../VNCHooks" /I "../libjpeg" /I "../omnithread" /I "../zlib" /D "__WIN32__" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_OMNITHREAD_DLL" /D "ZLIB_EXPORTS" /D "_ZLIB_DLL" /D "ZLIB_DLL" /D "HORIZONLIVE" /FD /GZ /c
# SUBTRACT BASE CPP /X /u /YX
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "./lib" /I ".." /I "../.." /I "../omnithread" /D WINVER=0x0500 /D _WIN32_WINNT=0x0500 /D "__WIN32__" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_OMNITHREAD_DLL" /D "HORIZONLIVE" /D "USE_ALT_SOCKETS" /FR /FD /GZ /c
# SUBTRACT CPP /Gy /X /u /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib wsock32.lib shlwapi.lib omnithread_rt.lib libjpeg.lib VNCHooks.lib zlib.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:"./HorizonLive/LiveShare.exe" /pdbtype:sept /libpath:"./HorizonLive"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 comctl32.lib htmlhelp.lib kernel32.lib shell32.lib user32.lib gdi32.lib advapi32.lib ole32.lib wsock32.lib shlwapi.lib omnithread_rt.lib libjpeg.a libz.a VNCHOOKS.lib wininet.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:"C:\Documents and Settings\Administrator\Application Data\HorizonWimba\JSecureDoor\appshare_0.0.1\data\AppShare.exe" /pdbtype:sept /libpath:"./lib/libjpeg" /libpath:"./lib/zlib" /libpath:"./HorizonLiveDebug"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "AppShare - Win32 TightVNC"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "AppShare___Win32_TightVNC"
# PROP BASE Intermediate_Dir "AppShare___Win32_TightVNC"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TightVNCRelease"
# PROP Intermediate_Dir "TightVNCRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Od /Ob2 /I "./lib" /I ".." /I "../.." /I "../omnithread" /I "$(PATH_TO_VNC_1_2)/winvnc" /I "C:\Program Files\HTML Help Workshop\include" /D WINVER=0x0500 /D "__WIN32__" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_OMNITHREAD_DLL" /D "HORIZONLIVE" /FD /GZ /c
# SUBTRACT BASE CPP /X /u /YX
# ADD CPP /nologo /MT /W3 /GX /Od /Ob2 /I "./lib" /I ".." /I "../.." /I "../omnithread" /I "C:\Program Files\HTML Help Workshop\include" /D WINVER=0x0500 /D "__WIN32__" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_OMNITHREAD_DLL" /FD /GZ /c
# SUBTRACT CPP /X /u /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib htmlhelp.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib wsock32.lib shlwapi.lib omnithread_rt.lib libjpeg.a libz.a VNCHOOKS.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"C:\Documents and Settings\Administrator\Application Data\HorizonLive\SecureDoor\HZSDControl\2.0.0.0\Doors\appshare_1_8_3\data\Release-AppShare.exe" /libpath:"./lib/libjpeg" /libpath:"./lib/zlib" /libpath:"./HorizonLive" /libpath:"C:\Program Files\HTML Help Workshop\lib"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 comctl32.lib htmlhelp.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib wsock32.lib shlwapi.lib omnithread_rt.lib libjpeg.a libz.a VNCHOOKS.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"C:\Documents and Settings\grolloj\Application Data\HorizonLive\SecureDoor\HZSDControl\2.0.0.0\Doors\appshare_1_8_8\data\AppShare.exe" /libpath:"./lib/libjpeg" /libpath:"./lib/zlib" /libpath:"./HorizonLive" /libpath:"C:\Program Files\HTML Help Workshop\lib"
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "AppShare - Win32 Release"
# Name "AppShare - Win32 Debug"
# Name "AppShare - Win32 TightVNC"
# Begin Group "tightvnc-cpp"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\CPUUsage.cpp
# End Source File
# Begin Source File

SOURCE=..\CPUUsageWin32HRPC.cpp
# End Source File
# Begin Source File

SOURCE=..\CPUUsageWin32PDH.cpp
# End Source File
# Begin Source File

SOURCE=..\CPUUsageWin9x.cpp
# End Source File
# Begin Source File

SOURCE=..\d3des.c
# End Source File
# Begin Source File

SOURCE=..\FileTransferItemInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\GracePeriod.cpp
# End Source File
# Begin Source File

SOURCE=..\keysym2ucs.cpp
# End Source File
# Begin Source File

SOURCE=..\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\MatchWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\MinMax.cpp
# End Source File
# Begin Source File

SOURCE=..\PDHFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\PollControls.cpp
# End Source File
# Begin Source File

SOURCE=..\PollCycleControl.cpp
# End Source File
# Begin Source File

SOURCE=..\PollingQuadrant.cpp
# End Source File
# Begin Source File

SOURCE=..\PollingScanLines.cpp
# End Source File
# Begin Source File

SOURCE=..\PollingScanLinesAdjacent.cpp
# End Source File
# Begin Source File

SOURCE=..\RectList.cpp
# End Source File
# Begin Source File

SOURCE=..\stdhdrs.cpp
# End Source File
# Begin Source File

SOURCE=..\translate.cpp
# End Source File
# Begin Source File

SOURCE=..\VideoDriver.cpp
# End Source File
# Begin Source File

SOURCE=..\vncAbout.cpp
# End Source File
# Begin Source File

SOURCE=..\vncAcceptDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\vncauth.c
# End Source File
# Begin Source File

SOURCE=..\vncBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\vncClient.cpp
# End Source File
# Begin Source File

SOURCE=..\vncDesktop.cpp
# End Source File
# Begin Source File

SOURCE=..\vncEncodeCoRRE.cpp
# End Source File
# Begin Source File

SOURCE=..\vncEncodeHexT.cpp
# End Source File
# Begin Source File

SOURCE=..\vncEncoder.cpp
# End Source File
# Begin Source File

SOURCE=..\vncEncodeRRE.cpp
# End Source File
# Begin Source File

SOURCE=..\vncEncodeTight.cpp
# End Source File
# Begin Source File

SOURCE=..\vncEncodeZlib.cpp
# End Source File
# Begin Source File

SOURCE=..\vncEncodeZlibHex.cpp
# End Source File
# Begin Source File

SOURCE=..\vncHTTPConnect.cpp
# End Source File
# Begin Source File

SOURCE=..\vncInstHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\vncKeymap.cpp
# End Source File
# Begin Source File

SOURCE=..\vncRegion.cpp
# End Source File
# Begin Source File

SOURCE=..\vncServer.cpp
# End Source File
# Begin Source File

SOURCE=..\vncServerSingleton.cpp
# End Source File
# Begin Source File

SOURCE=..\vncService.cpp
# End Source File
# Begin Source File

SOURCE=..\vncSockConnect.cpp
# End Source File
# Begin Source File

SOURCE=..\vncTimedMsgBox.cpp
# End Source File
# Begin Source File

SOURCE=..\VSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\VSocketAlt.cpp
# End Source File
# Begin Source File

SOURCE=..\VSocketSystemAlt.cpp
# End Source File
# Begin Source File

SOURCE=..\WallpaperUtils.cpp
# End Source File
# End Group
# Begin Group "tightvnc-hpp"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\CPUUsage.h
# End Source File
# Begin Source File

SOURCE=..\CPUUsageWin32HRPC.h
# End Source File
# Begin Source File

SOURCE=..\CPUUsageWin32PDH.h
# End Source File
# Begin Source File

SOURCE=..\CPUUsageWin9x.h
# End Source File
# Begin Source File

SOURCE=..\d3des.h
# End Source File
# Begin Source File

SOURCE=..\FileTransferItemInfo.h
# End Source File
# Begin Source File

SOURCE=..\GracePeriod.h
# End Source File
# Begin Source File

SOURCE=..\keysym2ucs.h
# End Source File
# Begin Source File

SOURCE=..\keysymdef.h
# End Source File
# Begin Source File

SOURCE=..\Log.h
# End Source File
# Begin Source File

SOURCE=..\MatchWindow.h
# End Source File
# Begin Source File

SOURCE=..\MinMax.h
# End Source File
# Begin Source File

SOURCE=..\PDHFunctions.h
# End Source File
# Begin Source File

SOURCE=..\PollControls.h
# End Source File
# Begin Source File

SOURCE=..\PollCycleControl.h
# End Source File
# Begin Source File

SOURCE=..\PollingBase.h
# End Source File
# Begin Source File

SOURCE=..\PollingQuadrant.h
# End Source File
# Begin Source File

SOURCE=..\PollingScanLines.h
# End Source File
# Begin Source File

SOURCE=..\PollingScanLinesAdjacent.h
# End Source File
# Begin Source File

SOURCE=..\RectList.h
# End Source File
# Begin Source File

SOURCE=..\rfb.h
# End Source File
# Begin Source File

SOURCE=..\stdhdrs.h
# End Source File
# Begin Source File

SOURCE=..\translate.h
# End Source File
# Begin Source File

SOURCE=..\VideoDriver.h
# End Source File
# Begin Source File

SOURCE=..\vncAbout.h
# End Source File
# Begin Source File

SOURCE=..\vncAcceptDialog.h
# End Source File
# Begin Source File

SOURCE=..\vncAdvancedProperties.h
# End Source File
# Begin Source File

SOURCE=..\vncauth.h
# End Source File
# Begin Source File

SOURCE=..\vncBuffer.h
# End Source File
# Begin Source File

SOURCE=..\vncClient.h
# End Source File
# Begin Source File

SOURCE=..\vncCorbaConnect.h
# End Source File
# Begin Source File

SOURCE=..\vncDesktop.h
# End Source File
# Begin Source File

SOURCE=..\vncEncodeCoRRE.h
# End Source File
# Begin Source File

SOURCE=..\vncEncodeHexT.h
# End Source File
# Begin Source File

SOURCE=..\vncEncoder.h
# End Source File
# Begin Source File

SOURCE=..\vncEncodeRRE.h
# End Source File
# Begin Source File

SOURCE=..\vncEncodeTight.h
# End Source File
# Begin Source File

SOURCE=..\vncEncodeZlib.h
# End Source File
# Begin Source File

SOURCE=..\vncEncodeZlibHex.h
# End Source File
# Begin Source File

SOURCE=..\vncHTTPConnect.h
# End Source File
# Begin Source File

SOURCE=..\vncInstHandler.h
# End Source File
# Begin Source File

SOURCE=..\vncKeymap.h
# End Source File
# Begin Source File

SOURCE=..\vncPasswd.h
# End Source File
# Begin Source File

SOURCE=..\vncProperties.h
# End Source File
# Begin Source File

SOURCE=..\vncRegion.h
# End Source File
# Begin Source File

SOURCE=..\vncServer.h
# End Source File
# Begin Source File

SOURCE=..\vncServerSingleton.h
# End Source File
# Begin Source File

SOURCE=..\vncService.h
# End Source File
# Begin Source File

SOURCE=..\vncSockConnect.h
# End Source File
# Begin Source File

SOURCE=..\vncTimedMsgBox.h
# End Source File
# Begin Source File

SOURCE=..\VSocket.h
# End Source File
# Begin Source File

SOURCE=..\VSocketAlt.h
# End Source File
# Begin Source File

SOURCE=..\VSocketSystemAlt.h
# End Source File
# Begin Source File

SOURCE=..\VTypes.h
# End Source File
# Begin Source File

SOURCE=..\WallpaperUtils.h
# End Source File
# End Group
# Begin Group "horizon-cpp"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\horizonAdvancedSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonBasicSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonConnect.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonDefaults.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonDoorReader.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonDoorWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonMain.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonMainHZTC.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonNormalSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonPollControls.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonPollingAdapter.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonPollingType.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonSettingsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\horizonSharedArea.cpp
# End Source File
# End Group
# Begin Group "horizon-hpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\horizonAdvancedSettings.h
# End Source File
# Begin Source File

SOURCE=.\horizonBasicSettings.h
# End Source File
# Begin Source File

SOURCE=.\horizonConnect.h
# End Source File
# Begin Source File

SOURCE=.\horizonDefaults.h
# End Source File
# Begin Source File

SOURCE=.\horizonDoorReader.h
# End Source File
# Begin Source File

SOURCE=.\horizonDoorWriter.h
# End Source File
# Begin Source File

SOURCE=.\horizonMain.h
# End Source File
# Begin Source File

SOURCE=.\horizonMenu.h
# End Source File
# Begin Source File

SOURCE=.\horizonNormalSettings.h
# End Source File
# Begin Source File

SOURCE=.\horizonPollControls.h
# End Source File
# Begin Source File

SOURCE=.\horizonPollingAdapter.h
# End Source File
# Begin Source File

SOURCE=.\horizonPollingType.h
# End Source File
# Begin Source File

SOURCE=.\horizonProperties.h
# End Source File
# Begin Source File

SOURCE=.\horizonSettingsDialog.h
# End Source File
# Begin Source File

SOURCE=.\horizonSharedArea.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "horizon-res"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\horizon.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\appshare.bmp
# End Source File
# Begin Source File

SOURCE=.\res\appshare_mask.ico
# End Source File
# Begin Source File

SOURCE=.\res\appshare_off.ico
# End Source File
# Begin Source File

SOURCE=.\res\appshare_off_lr.ico
# End Source File
# Begin Source File

SOURCE=.\res\appshare_on.ico
# End Source File
# Begin Source File

SOURCE=.\res\appshare_on_lr.ico
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\LiveShare.ico
# End Source File
# Begin Source File

SOURCE=.\res\tightvnc.bmp
# End Source File
# End Target
# End Project
