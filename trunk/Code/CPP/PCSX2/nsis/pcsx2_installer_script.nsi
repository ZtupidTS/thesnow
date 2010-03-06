; PCSX2 NSIS installer script
; loosely based on a collection of examples and on information from the wikipedia

; Application version, changed for each release to match the verision

; ----------------------------------------
; Determine the revision numbers of the various components

!system 'SubWCRev.exe ..\pcsx2 templates\svnrev_pcsx2.nsh svnrev_pcsx2.nsh'

!ifdef INC_PLUGINS
  !system 'SubWCRev.exe ..\plugins\gsdx       templates\svnrev_gsdx.nsh     svnrev_gsdx.nsh'
  !system 'SubWCRev.exe ..\plugins\spu2-x     templates\svnrev_spu2x.nsh    svnrev_spu2x.nsh'
  !system 'SubWCRev.exe ..\plugins\cdvdiso    templates\svnrev_cdvdiso.nsh  svnrev_cdvdiso.nsh'
  !system 'SubWCRev.exe ..\plugins\lilypad    templates\svnrev_lilypad.nsh  svnrev_lilypad.nsh'
  !system 'SubWCRev.exe ..\plugins\zerogs\dx  templates\svnrev_zerogs.nsh   svnrev_zerogs.nsh'
  !system 'SubWCRev.exe ..\plugins\zerospu2   templates\svnrev_zerospu2.nsh svnrev_zerospu2.nsh'
!endif

!include "svnrev_pcsx2.nsh"

!ifdef INC_PLUGINS
  !include "svnrev_gsdx.nsh"
  !include "svnrev_spu2x.nsh"
  !include "svnrev_cdvdiso.nsh"
  !include "svnrev_lilypad.nsh"
  !include "svnrev_zerogs.nsh"
  !include "svnrev_zerospu2.nsh"
!endif

; ----------------------------------------

!define APP_NAME "PCSX2 0.9.7.r${SVNREV}"
!define APP_FILENAME "pcsx2-0.9.7.r${SVNREV}"

!define INSTDIR_REG_ROOT "HKLM"
!define INSTDIR_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"


; ----------------------------------------
; Include Modern UI 2 and advanced log uninstaller.

!include "MUI2.nsh"
!include "AdvUninstLog.nsh"

; -------------------------------------
; Test if Visual Studio Redistributables 2008 SP1 installed
; Returns -1 if there is no VC redistributables intstalled
;
Function CheckVCRedist

   Push $R0
   ClearErrors
   ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{9A25302D-30C0-39D9-BD6F-21E6EC160475}" "Version"

   ; if VS 2008+ redist SP1 not installed, install it
   IfErrors 0 VSRedistInstalled
   StrCpy $R0 "-1"

VSRedistInstalled:
   Exch $R0
   
FunctionEnd

; -------------------------------------
; Safe directory deletion code. :)
; 
Function un.DeleteDirIfEmpty
  FindFirst $R0 $R1 "$0\*.*"
  strcmp $R1 "." 0 NoDelete
   FindNext $R0 $R1
   strcmp $R1 ".." 0 NoDelete
    ClearErrors
    FindNext $R0 $R1
    IfErrors 0 NoDelete
     FindClose $R0
     Sleep 1000
     RMDir "$0"
  NoDelete:
   FindClose $R0
FunctionEnd

; ----------------------------------------
; The name of the installer
Name "${APP_NAME}"

OutFile "${APP_FILENAME}-setup.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\PCSX2 0.9.7 beta"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\pcsx2" "Install_Dir"

; Request application privileges for Windows Vista (shouldn't be needed anymore!  -- air)
;RequestExecutionLevel admin

; Pages

  !insertmacro UNATTENDED_UNINSTALL
  !define MUI_COMPONENTSPAGE_NODESC ;no decription is really necessary at this stage...
  !insertmacro MUI_PAGE_COMPONENTS 
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_COMPONENTS
  !insertmacro MUI_UNPAGE_INSTFILES


; ----------------------------------------
; Basic section (emulation proper)
Section "${APP_NAME} (required)"

  SectionIn RO
  
  ; Put file there. It's catched by the uninstaller script
  SetOutPath $INSTDIR
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File           /oname=pcsx2-r${SVNREV}.exe      ..\bin\pcsx2.exe
  File /nonfatal /oname=pcsx2-dev-r${SVNREV}.exe  ..\bin\pcsx2-dev.exe
  File ..\bin\w32pthreads.v2.dll
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL

  ; -- Languages and Patches --

  SetOutPath $INSTDIR\Langs
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /r ..\bin\Langs\*.mo
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL

  SetOutPath $INSTDIR\Patches
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /r ..\bin\Patches\*.pnach
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL


  ; NULL plugins are required, and really there should be more but we don't have working
  ; SPU2 or GS null plugins right now.  (note: no install logging performed here -- nulls
  ; are removed manually by name)
  
  SetOutPath $INSTDIR\Plugins
  File ..\bin\Plugins\USBnull.dll
  File ..\bin\Plugins\DEV9null.dll
  File ..\bin\Plugins\FWnull.dll
  File ..\bin\Plugins\CDVDnull.dll

  ; -- Other plugins --

!ifdef INC_PLUGINS

  SetOutPath $INSTDIR\Plugins
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL

  File /nonfatal /oname=gsdx-sse2-r${SVNREV_GSDX}.dll    ..\bin\Plugins\gsdx-sse2.dll
  File /nonfatal /oname=gsdx-ssse3-r${SVNREV_GSDX}.dll   ..\bin\Plugins\gsdx-ssse3.dll 
  File /nonfatal /oname=gsdx-sse4-r${SVNREV_GSDX}.dll    ..\bin\Plugins\gsdx-sse4.dll  
  File /nonfatal /oname=spu2-x-r${SVNREV_SPU2X}.dll      ..\bin\Plugins\spu2-x.dll     
  File /nonfatal /oname=cdvdiso-r${SVNREV_CDVDISO}.dll   ..\bin\Plugins\cdvdiso.dll    
  File /nonfatal /oname=lilypad-r${SVNREV_LILYPAD}.dll   ..\bin\Plugins\lilypad.dll

  File /nonfatal /oname=zerogs-r${SVNREV_ZEROGS}.dll     ..\bin\Plugins\zerogs.dll     
  File /nonfatal /oname=zerospu2-r${SVNREV_ZEROSPU2}.dll ..\bin\Plugins\zerospu2.dll   

  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif

  ; Write the installation path into the registry
  WriteRegStr HKLM Software\pcsx2 "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${INSTDIR_REG_KEY}" "DisplayName" "PCSX2 - Playstation 2 Emulator"
  WriteRegStr HKLM "${INSTDIR_REG_KEY}" "UninstallString" '"$INSTDIR\${UNINST_EXE}-r${SVNREV}.exe"'
  WriteRegDWORD HKLM "${INSTDIR_REG_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${INSTDIR_REG_KEY}" "NoRepair" 1
  WriteUninstaller "${UNINST_EXE}-r${SVNREV}.exe"

SectionEnd

; ----------------------------------------
; MSVC Redistributable - required if the user des not already have it
; Note: if your NSIS generates an error here it means you need to download the latest
; visual studio redist package from microsoft.  Any redist 2008/SP1 or newer will do.
Section "Microsoft Visual C++ 2008 SP1 Redist (required)"

  SectionIn RO
  SetOutPath $TEMP
  File "vcredist_x86.exe"
  Call CheckVCRedist
  StrCmp $R0 "-1" installRedist
    DetailPrint "Visual C++ 2008 SP1 Redistributable already installed, skipping..."
    Goto skipRedist

  installRedist:
    ExecWait "$TEMP\vcredist_x86.exe"

SkipRedist:  
SectionEnd

; ----------------------------------------
; Optional sections (can be disabled by the user)
Section "Start Menu Shortcuts"

  SetOutPath $INSTDIR
    
  CreateDirectory "$SMPROGRAMS\pcsx2"
  CreateShortCut "$SMPROGRAMS\pcsx2\${UNINST_EXE}-r${SVNREV}.lnk"  "$INSTDIR\${UNINST_EXE}-r${SVNREV}.exe"  "" "$INSTDIR\${UNINST_EXE}-r${SVNREV}.exe" 0
  CreateShortCut "$SMPROGRAMS\pcsx2\pcsx2-r${SVNREV}.lnk"          "$INSTDIR\pcsx2-r${SVNREV}.exe"          "" "$INSTDIR\pcsx2-r${SVNREV}.exe" 0

  IfFileExists ..\bin\pcsx2-dev.exe 0 +2
    CreateShortCut "$SMPROGRAMS\pcsx2\pcsx2-dev-r${SVNREV}.lnk"  "$INSTDIR\pcsx2-dev-r${SVNREV}.exe"  "" "$INSTDIR\pcsx2-dev-r${SVNREV}.exe" 0 "" "" \
      "PCSX2 Devel (has additional logging support)"

SectionEnd

;--------------------------------

Function .onInit

        ;prepare log always within .onInit function
        !insertmacro UNINSTALL.LOG_PREPARE_INSTALL

FunctionEnd


Function .onInstSuccess

         ;create/update log always within .onInstSuccess function
         !insertmacro UNINSTALL.LOG_UPDATE_INSTALL

FunctionEnd


; --------------------------------------
; Uninstaller

Function .removeShorties

    ; Remove shortcuts, if any
    Delete "$SMPROGRAMS\pcsx2\${UNINST_EXE}-r${SVNREV}.lnk"
    Delete "$SMPROGRAMS\pcsx2\pcsx2-r${SVNREV}.lnk"
    Delete "$SMPROGRAMS\pcsx2\pcsx2-dev-r${SVNREV}.lnk"
    
    StrCpy $0 "$SMPROGRAMS\pcsx2"
    Call un.DeleteDirIfEmpty

FunctionEnd

; Languages, patches, and null plugins should only be removed if all previous versions of 
; PCSX2 have been uninstalled.  And we know that's happened when the pcsx2\versions registry
; key is empty.
Function .removeSharedJunk
    !insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\Langs"
    !insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\Patches"

    Delete "$INSTDIR\Plugins\USBnull.dll"
    Delete "$INSTDIR\Plugins\DEV9null.dll"
    Delete "$INSTDIR\Plugins\FWnull.dll"
    Delete "$INSTDIR\Plugins\CDVDnull.dll"
FunctionEnd


Section "Un.Basic Removal (removes only files installed by this package) ${APP_NAME}"

  !insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR"

  MessageBox MB_YESNO "Also remove plugins that were installed with this package?" IDYES true IDNO false
  true:
    !insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\Plugins"
  false:

  !insertmacro UNINSTALL.LOG_END_UNINSTALL

  ; Remove registry keys
  DeleteRegKey HKLM ${INSTDIR_REG_KEY}

  Call .removeShorties

  DeleteRegKey ${INSTDIR_REG_ROOT} "${INSTDIR_REG_KEY}"

  ; And remove the install dir but only if it's clean of user content:
  StrCpy $0 "$INSTDIR"
  Call un.DeleteDirIfEmpty

SectionEnd

Section Un.Full Removal (completely removes all PCSX2 program files and folders)

  MessageBox MB_YESNO "WARNING!  This will remove *all* files in $INSTDIR -- are you sure you want to proceed?" IDYES true IDNO false
  true:

  RMDir /r "$INSTDIR"
  Call .removeShorties

  DeleteRegKey ${INSTDIR_REG_ROOT} "${INSTDIR_REG_KEY}"
  DeleteRegKey HKLM Software\pcsx2

  false:
SectionEnd

; --------------------------------------
Function UN.onInit

         ;begin uninstall, could be added on top of uninstall section instead
         !insertmacro UNINSTALL.LOG_BEGIN_UNINSTALL

FunctionEnd

