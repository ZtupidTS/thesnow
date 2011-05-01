
; PCSX2 Web-based Install Package!
; (a NSIS installer script)
;
; Copyright 2009-2011  PCSX2 Dev Team
;
; The installer generated by this script will download all relevant components for 
; PCSX2 from a variety of mirror hosts.  Packages are only downloaded on an as-needed
; basis; this most importantly applies to the very bulky VS 2008 and VS2010 packages.

!ifndef INC_ZZOGL
  ; Includes ZZOGL and CG Toolkit (via web install).  Currently not supported (work in progress)
  !define INC_ZZOGL	    0
!endif

!define OUTFILE_POSTFIX "websetup"
!include "SharedBase.nsh"

!insertmacro MUI_PAGE_COMPONENTS 
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
  
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

!include "ApplyExeProps.nsh"
!include "SharedRedtape.nsh"

; =======================================================================
;                            Installer Sections
; =======================================================================

; -----------------------------------------------------------------------
; Basic section (emulation proper)
Section "!${APP_NAME} (required)" SEC_CORE

  SectionIn RO

!include "SectionCoreReqs.nsh"

  ; ------------------------------------------
  ;          -- Plugins Section --
  ; ------------------------------------------

!if ${INC_PLUGINS} > 0

  ; [TODO]  :  Eventually the 'latest' plugin packages should be downloaded from one
  ;   of our mirrors.  For now plugins are included in the web installer.

  SetOutPath "$INSTDIR\Plugins"
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL

    File /nonfatal /oname=gsdx-sse2-r${SVNREV_GSDX}.dll    ..\bin\Plugins\gsdx-sse2.dll
    File /nonfatal /oname=gsdx-ssse3-r${SVNREV_GSDX}.dll   ..\bin\Plugins\gsdx-ssse3.dll 
    File /nonfatal /oname=gsdx-sse4-r${SVNREV_GSDX}.dll    ..\bin\Plugins\gsdx-sse4.dll
    File /nonfatal /oname=zerogs-r${SVNREV_ZEROGS}.dll     ..\bin\Plugins\zerogs.dll
  
    File /nonfatal /oname=spu2-x-r${SVNREV_SPU2X}.dll      ..\bin\Plugins\spu2-x.dll
    File /nonfatal /oname=zerospu2-r${SVNREV_ZEROSPU2}.dll ..\bin\Plugins\zerospu2.dll
  
    File /nonfatal /oname=cdvdiso-r${SVNREV_CDVDISO}.dll   ..\bin\Plugins\cdvdiso.dll
    File                                                   ..\bin\Plugins\cdvdGigaherz.dll
  
    File /nonfatal /oname=lilypad-r${SVNREV_LILYPAD}.dll   ..\bin\Plugins\lilypad.dll
    File                                                   ..\bin\Plugins\PadSSSPSX.dll

  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL

!endif

SectionEnd

!include "SectionShortcuts.nsh"

!if ${INC_ZZOGL} > 0
Section "ZZogl Plugin (requires OpenGL)"

  SetOutPath "$INSTDIR\Plugins"
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
    File /oname=zzogl-pg-r${SVNREV_ZZOGL}.dll     ..\bin\Plugins\zzogl-pg.dll
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL

SectionEnd

Section "Nvidia's CG Toolkit"

  ; This section is required by anything using OpenGL, typically.
  ; It should be automatically checked when ZZogl is enabled.

 ; CG Toolkit would be downloaded from here:
 ; http://developer.download.nvidia.com/cg/Cg_2.2/Cg-2.2_February2010_Setup.exe
  
SectionEnd
!endif

; -----------------------------------------------------------------------
; MSVC Redistributable - required if the user does not already have it
; Note: if your NSIS generates an error here it means you need to download the latest
; visual studio redist package from microsoft.  Any redist 2008/SP1 or newer will do.
;
; IMPORTANT: Online references for how to detect the presence of the VS2008 redists LIE.
; None of the methods are reliable, because the registry keys placed by the MSI installer
; vary depending on operating system *and* MSI installer version (youch).
;
Section "Microsoft Visual C++ 2008 SP1 Redist (required)"  SEC_CRT2008

  ;SectionIn RO

  ; Downloaded from:
  ;  http://download.microsoft.com/download/d/d/9/dd9a82d0-52ef-40db-8dab-795376989c03/vcredist_x86.exe
 
  SetOutPath "$TEMP"

  DetailPrint "Downloading Visual C++ 2008 SP1 Redistributable Setup..."
  DetailPrint "Contacting Microsoft.com..."
  NSISdl::download /TIMEOUT=15000 "http://download.microsoft.com/download/d/d/9/dd9a82d0-52ef-40db-8dab-795376989c03/vcredist_x86.exe" "vcredist_2008_sp1_x86.exe"

  Pop $R0 ;Get the return value
  StrCmp $R0 "success" OnSuccess
  DetailPrint "Cound not contact Microsoft.com, or the file has been (re)moved!"
  DetailPrint "Contacting Googlecode.com..."
  NSISdl::download /TIMEOUT=20000 "http://pcsx2.googlecode.com/files/vcredist_2008_sp1_x86.exe" "vcredist_2008_sp1_x86.exe"

  ; [TODO] Provide a mirror for this file hosted from pcsx2.net .. ?  or emudev.net .. ?
  ;Pop $R0 ;Get the return value
  ;StrCmp $R0 "success" +2
  ;NSISdl::download /TIMEOUT=15000 "http://www.pcsx2.net/vcredist_x86.exe" "vcredist_2008_sp1_x86.exe"

  Pop $R0 ;Get the return value
  StrCmp $R0 "success" +2
    MessageBox MB_OK "Could not download Visual Studio 2008 Redist; none of the mirrors appear to be functional."
    Goto done

OnSuccess:
  DetailPrint "Running Visual C++ 2008 SP1 Redistributable Setup..."
  ExecWait '"$TEMP\vcredist_2008_sp1_x86.exe" /qb'
  DetailPrint "Finished Visual C++ 2008 SP1 Redistributable Setup"
  Delete "$TEMP\vcredist_2008_sp1_x86.exe"

done:
SectionEnd

Section "Microsoft Visual C++ 2010 Redist (required)" SEC_CRT2010

  ; Make this required on the web installer, since it has a fully reliable check to
  ; see if it needs to be downloaded and installed or not.
  SectionIn RO

  ; Detection made easy: Unlike previous redists, VC2010 now generates a platform
  ; independent key for checking availability.
  
  ; Downloaded from:
  ;   http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe

  ClearErrors
  ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86" "Installed"
  IfErrors 0 +2
  DetailPrint "Visual C++ 2010 Redistributable registry key was not found; assumed to be uninstalled."
  StrCmp $R0 "1" 0 +3
    DetailPrint "Visual C++ 2010 Redistributable is already installed; skipping!"
    Goto done

  SetOutPath "$TEMP"

  DetailPrint "Downloading Visual C++ 2010 Redistributable Setup..."
  DetailPrint "Contacting Microsoft.com..."
  NSISdl::download /TIMEOUT=15000 "http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe" "vcredist_2010_x86.exe"

  Pop $R0 ;Get the return value
  StrCmp $R0 "success" OnSuccess
  DetailPrint "Cound not contact Microsoft.com, or the file has been (re)moved!"
  DetailPrint "Contacting Googlecode.com..."
  NSISdl::download /TIMEOUT=20000 "http://pcsx2.googlecode.com/files/vcredist_2010_x86.exe" "vcredist_2010_x86.exe"

  ; [TODO] Provide a mirror for this file hosted from pcsx2.net .. ?  or emudev.net .. ?
  ;Pop $R0 ;Get the return value
  ;StrCmp $R0 "success" +2
  ;NSISdl::download /TIMEOUT=30000 "http://www.pcsx2.net/vcredist_x86.exe" "vcredist_2010_x86.exe"

  Pop $R0 ;Get the return value
  StrCmp $R0 "success" +2
    MessageBox MB_OK "Could not download Visual Studio 2010 Redist; none of the mirrors appear to be functional."
    Goto done

OnSuccess:
  DetailPrint "Running Visual C++ 2010 SP1 Redistributable Setup..."
  ExecWait '"$TEMP\vcredist_2010_x86.exe" /qb'
  DetailPrint "Finished Visual C++ 2010 SP1 Redistributable Setup"
  
  Delete "$TEMP\vcredist_2010_x86.exe"

done:
SectionEnd

; -----------------------------------------------------------------------
; This section needs to be last, so that in case it fails, the rest of the program will
; be installed cleanly.
; 
; This section could be optional, but why not?  It's pretty painless to double-check that
; all the libraries are up-to-date.
;
Section "DirectX Web Setup (recommended)" SEC_DIRECTX
                                                                              
  ;SectionIn RO

  SetOutPath "$TEMP"
 
  DetailPrint "Downloading DirectX Web Setup..."
  DetailPrint "Contacting Microsoft.com..."
  NSISdl::download /TIMEOUT=15000 "http://download.microsoft.com/download/1/7/1/1718CCC4-6315-4D8E-9543-8E28A4E18C4C/dxwebsetup.exe" dxwebsetup.exe

  ; No mirrors provided for the dx web setup.  Either we get it from Microsoft, or we don't bother.
  ; (this is done because there's a good chance the dxwebsetup we provide won't work anyway, if Microsoft
  ;  has in fact re-arranged their website (again)).

  ;Pop $R0 ;Get the return value
  ;StrCmp $R0 "success" OnSuccess
  ;DetailPrint "Cound not contact Microsoft.com, or the file has been (re)moved!"
  ;DetailPrint "Contacting Googlecode.com..."
  ;NSISdl::download /TIMEOUT=20000 "http://code.google.com/dxwebsetup01.exe" "dxwebsetup.exe"

  Pop $R0 ;Get the return value
  StrCmp $R0 "success" OnSuccess
  DetailPrint "Cound not contact Microsoft.com, or the file has been (re)moved!"
  MessageBox MB_OK "Could not download the DirectX Web Setup.  Microsoft probably rearranged their website.  Please do an internet search for 'DirectX Setup' and download and install it yourself after this installer has finished."
  Goto done

  ;Pop $R0 ;Get the return value
  ;StrCmp $R0 "success" +2
  ;NSISdl::download /TIMEOUT=30000 "http://www.pcsx2.net/dxwebsetup.exe" "dxwebsetup.exe"

OnSuccess:
  DetailPrint "Running DirectX Web Setup..."
  ExecWait '"$TEMP\dxwebsetup.exe" /Q' $DirectXSetupError
  DetailPrint "Finished DirectX Web Setup"                                     
  Delete "$TEMP\dxwebsetup.exe"

done:

SectionEnd

!include "SectionUninstaller.nsh"

LangString DESC_CORE       ${LANG_ENGLISH} "Core components (binaries, plugins, languages, etc)."

LangString DESC_STARTMENU  ${LANG_ENGLISH} "Adds shortcuts for PCSX2 to the start menu (all users)."
LangString DESC_DESKTOP    ${LANG_ENGLISH} "Adds a shortcut for PCSX2 to the desktop (all users)."

LangString DESC_CRT2008    ${LANG_ENGLISH} "Required!  Only uncheck if you are certain this component is already installed."
LangString DESC_CRT2010    ${LANG_ENGLISH} "Will only be downloaded if you don't already have it installed."
LangString DESC_DIRECTX    ${LANG_ENGLISH} "Only uncheck this if you are quite certain your Direct3D runtimes are up to date."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_CORE}        $(DESC_CORE)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_STARTMENU}   $(DESC_STARTMENU)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_DESKTOP}     $(DESC_DESKTOP)

  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_CRT2008}     $(DESC_CRT2008)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_CRT2010}     $(DESC_CRT2010)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_DIRECTX}     $(DESC_DIRECTX)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
