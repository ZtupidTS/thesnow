#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_UseAnsi=n									;����
#AutoIt3Wrapper_Icon= NIS.ico								;ͼ��
#AutoIt3Wrapper_Outfile=..\..\thesnow.exe						;����ļ���
#AutoIt3Wrapper_Outfile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=n 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= Mobile Tools					;ע��
#AutoIt3Wrapper_Res_Description=Mobile Tools				;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=1.0.0.27
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;�Զ����°汾
#AutoIt3Wrapper_Res_LegalCopyright= thesnow					;��Ȩ
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;�Զ�����Դ��
;#AutoIt3Wrapper_Run_Tidy=                   				;�ű�����
;#AutoIt3Wrapper_Run_Obfuscator=      						;�����Ի�
;#AutoIt3Wrapper_Run_AU3Check= 								;�﷨���
;#AutoIt3Wrapper_Run_Before= 								;����ǰ
;#AutoIt3Wrapper_Run_After=									;���к�
#EndRegion AutoIt3Wrapper Ԥ�������(���ò���)
#cs �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
	
	AutoIt �汾: 3.2.13.6 (��һ��)
	�ű�����:thesnow
	Email:rundll32@126.com
	QQ/TM:133333542
	�ű��汾:
	�ű�����:
	
#ce �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣ߽ű���ʼ�ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
#NoTrayIcon
#RequireAdmin

#include <GDIPlus.au3>													;����GDI��
If $CmdLine[0] <= 0 Then												;�������С�ڵ���0��ֱ������ʱ��
	If Not FileExists('z:\' & @ScriptName) Then							;���z:\thesnow.exe������
		If StringInStr(@WindowsDir,StringLeft(@ScriptDir, 2)) Or StringLeft(@ScriptDir, 2)="c:" Then
			MsgBox(32,"","������ע��,�ѵ������ϵͳ�̸�ΪZ��ô?")
			Exit
		EndIf
		FileCopy(@ScriptFullPath, @TempDir & '\' & @ScriptName, 1)		;�����Լ�����ʱĿ¼
		Run(@TempDir & "\" & @ScriptName & ' "' & @ScriptFullPath & '"', @TempDir, @SW_HIDE);������ʱĿ¼���Լ������˲������������Լ���ǰ������·����
		Exit															;�˳�
	EndIf
Else																	;�����������0������������ʱ��
	If IsString($CmdLine[1]) And (StringRight($CmdLine[1],4)='.a3x') Then
		Run(@AutoItExe & ' /AutoIt3ExecuteScript ' & $CmdLine[1])
		Exit
	EndIf
	If StringLeft($CmdLine[1], 2)="\\" Then
		MsgBox(32,"","UNC·����֧��.�����˳�,��ȷ�������ļ��ڿ��ƶ��豸��.")
		Exit
	EndIf
	If Not FileExists('z:') Then										;���Z�̲�����
		ControlSend("[CLASS:CabinetWClass]","","SysListView321","{BACKSPACE}")
		ChangeDriver(StringLeft($CmdLine[1], 2), 'z:')					;���̷���ΪZ��
		Run('z:\' & @ScriptName, 'z:', @ScriptDir)						;����Z:\THESNOW.EXE
		Exit															;�˳�
	Else																;���Z�̴�����
		MsgBox(32, "����", 'Z���Ѿ�����,�������̷��޸�.')				;��ʾ��Ϣ
		Exit															;�˳�
	EndIf
EndIf

Sleep(1000)																;��ͣ1��
FileDelete(@TempDir & "\" & @ScriptName)								;ɾ����ʱĿ¼���Լ�
If Not FileExists("thesnow.ini") Then									;���thesnow.ini��������Z�̸�Ŀ¼
	MsgBox(32, "�����ļ�������!", '�����ļ�������,�����˳�...')
	Exit																;�˳�
EndIf


;run
$run = IniReadSection("thesnow.ini", "Run")
If Not @error Then
	For $i = 1 To $run[0][0]
		Run($run[$i][1])
	Next
EndIf
;ExecExt
$ExecExt = IniReadSection("thesnow.ini", "ExecExt")
If Not @error Then
	For $i = 1 To $ExecExt[0][0]
		RegWrite("HKCR\" & $ExecExt[$i][0], "", "REG_SZ", 'exefile')
		RegWrite("HKCR\" & $ExecExt[$i][0], "Content Type", "REG_SZ", 'application/x-msdownload')
		RegWrite("HKCR\" & $ExecExt[$i][0] & '\PersistentHandler', "", "REG_SZ", '{098f2470-bae0-11cd-b579-08002b30bfeb}')
	Next
EndIf
;LocalizedResourceName
$LocalizedResourceName = IniReadSection("thesnow.ini", "LocalizedResourceName")
If Not @error Then
	For $i = 1 To $LocalizedResourceName[0][0]
		RegWrite("HKCU\Software\Microsoft\Windows\Shell\LocalizedResourceName", $LocalizedResourceName[$i][0], "REG_SZ", $LocalizedResourceName[$i][1])
	Next
EndIf
;system setting
If IniRead("thesnow.ini", "SystemSet", 'ShowHideFileExt', 0) Then
	RegWrite('HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\HideFileExt', 'CheckedValue', 'REG_DWORD', 0)
	RegWrite('HKU\.DEFAULT\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0)
	RegWrite('HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0)
Else
	RegWrite('HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\HideFileExt', 'CheckedValue', 'REG_DWORD', 1)
	RegWrite('HKU\.DEFAULT\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 1)
	RegWrite('HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 1)
EndIf
If IniRead("thesnow.ini", "SystemSet", 'MainPage', "") <> "" Then
	RegWrite("HKCU\Software\Microsoft\Internet Explorer\Main", "Start Page", 'REG_SZ', IniRead("thesnow.ini", "SystemSet", 'MainPage', 0))
EndIf
If IniRead("thesnow.ini", "SystemSet", 'OpenFileUseNotepad', 0) <> 0 Then
	RegWrite("HKCR\*\shell\Notepad\command", "", 'REG_SZ', 'notepad.exe "%1"')
EndIf
If IniRead("thesnow.ini", "SystemSet", 'CommandLineHere', 0) <> 0 Then
	RegWrite("HKCR\Directory\shell\Command line here\command", "", 'REG_SZ', 'cmd.exe /k  cd "%l" & color 0a & title thesnow')
	RegWrite("HKCR\Drive\shell\Command line here\command", "", 'REG_SZ', 'cmd.exe /k  cd "%l" & color 0a & title thesnow')
EndIf

If IniRead("thesnow.ini", "SystemSet", 'Favorite', 0) <> 0 And FileExists(@ScriptDir & "\Favorites.rar") Then
	; Shows the filenames of all files in the current directory.
	$search = FileFindFirstFile(@FavoritesDir & "\*.*")  

	; Check if the search was successful
	If $search = -1 Then
		;MsgBox(0, "Error", "No files/directories matched the search pattern")
		;Exit
	EndIf

	While 1
		$file = FileFindNextFile($search) 
		If @error Then ExitLoop
		DirRemove(@FavoritesDir & "\" & $file,1)	
		FileDelete(@FavoritesDir & "\" & $file)
	WEnd
	; Close the search handle
	FileClose($search)
	$rar=RegRead('HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\WinRAR.exe',"")
	If $rar = "" Or (Not FileExists($rar)) Then
		FileInstall("C:\Program Files\WinRAR\rar.exe",@TempDir & "\rar.exe",1)
		FileInstall("C:\Program Files\WinRAR\rarreg.key",@TempDir & "\rarreg.key",1)
		$rar=@TempDir & "\rar.exe"
		Run($rar & " x -y " & @ScriptDir & "\Favorites.rar",FileGetLongName(@FavoritesDir),@SW_HIDE)	
	Else
		Run($rar & " x -y " & @ScriptDir & "\Favorites.rar",FileGetLongName(@FavoritesDir),@SW_SHOW)
	EndIf
EndIf

If IniRead("thesnow.ini", "SystemSet", 'WallPaper', "") <> "" Then
	_ChangeDesktopWallpaper(IniRead("thesnow.ini", "SystemSet", 'WallPaper', 0), 2)
EndIf




;~ 0 - Details
;~ 1 - Large Icon
;~ 2 - List
;~ 3 - Small Icon
;~ 4 - Tile
$DisktopIcon=IniRead("thesnow.ini","SystemSet","DisktopIcon","")
If ($DisktopIcon >= 0) And ($DisktopIcon < 5) Then
	$DisktopHwnd=ControlGetHandle("[Class:Progman]","","SysListView321")
	_GUICtrlListViewSetView($DisktopHwnd,$DisktopIcon)
EndIf
$RemoveDriver = IniReadSection("thesnow.ini", "RemoveDriver")
If Not @error Then
	For $i = 1 To $RemoveDriver[0][0]
		RemoveDriver($RemoveDriver[$i][0])
	Next
EndIf

;Image File Execution Options
;	HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options
$ImageFile = IniReadSection("thesnow.ini", "ImageFileExecutionOptions")
If Not @error Then
	For $i = 1 To $ImageFile[0][0]
		if $ImageFile[$i][1] <> "-" Then
			RegWrite('HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\' & $ImageFile[$i][0],"Debugger","reg_sz",$ImageFile[$i][1])
		Else
			RegDelete('HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\' & $ImageFile[$i][0])
			If $ImageFile[$i][0] = 'AllFile' Then
				RegDelete('HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options')
			EndIf
		EndIf
	Next
EndIf


RefreshSystem()
Exit

Func WaitUnmount()
	
	
EndFunc

Func RefreshSystem()
	FileWriteLine(@TempDir & "\Refresh.inf", '[version]' & @CRLF & 'signature=$chicago$')
	ShellExecuteWait(@TempDir & '\Refresh.inf', '', @TempDir, 'Install', @SW_HIDE)
	FileDelete(@TempDir & "\Refresh.inf")
EndFunc   ;==>RefreshSystem

Func RemoveDriver($str)
	$drv = DriveGetDrive("ALL")
	For $i = 1 To $drv[0]
		If DriveGetLabel($drv[$i]) = $str Then
			;FileWrite(@TempDir & "\hide.txt", 'select VOLUME ' & $drv[$i] & @CRLF & 'remove')
			;RunWait("diskpart.exe /s " & @TempDir & "\hide.txt", @SystemDir, @SW_HIDE)
			;FileDelete(@TempDir & "\hide.txt")
			$VolumeMountPoint=$drv[$i]
			If StringRight($VolumeMountPoint,1)<>"\" Then $VolumeMountPoint &= "\"
			DllCall("kernel32.dll", "int", "DeleteVolumeMountPointW", "wstr",$VolumeMountPoint)
		EndIf
	Next
EndFunc   ;==>RemoveDriver

Func ChangeDriver($str, $dst)
	;FileWrite(@TempDir & "\change.txt", 'select VOLUME ' & $str & @CRLF & 'assign letter=z')
	;RunWait("diskpart.exe /s " & @TempDir & "\change.txt", @SystemDir, @SW_HIDE)
	;FileDelete(@TempDir & "\change.txt")
	Local $Struct
    If StringRight($str,1) <> "\" Then $str &= "\"
    If StringRight($dst,1) <> "\" Then $dst &= "\"
    If Not FileExists($str) Then Return -1
    If FileExists($dst) Then Return -1
    $Struct = DllStructCreate ("char[511]")
    DllCall("kernel32.dll", "int", "GetVolumeNameForVolumeMountPoint", "str", $str, "ptr", DllStructGetPtr($Struct), "dword",511)
    DllCall("kernel32.dll", "int", "DeleteVolumeMountPointW", "wstr",$str)
    DllCall("kernel32.dll", "Int", "SetVolumeMountPointW", "wstr", $dst, "wstr",DllStructGetData($Struct,1))  
EndFunc   ;==>ChangeDriver

Func _ChangeDesktopWallpaper($image, $style = 0)
	;===============================================================================
	;
	; Function Name:    _ChangeDesktopWallpaper
	; Description:       Update WallPaper Settings
	; Usage:              _ChangeDesktopWallpaper(@WindowsDir & '\' & 'zapotec.bmp',1)
	; Parameter(s):     $image - Full Path to BitMap File (*.bmp)
	;                              [$style] - 0 = Centered, 1 = Tiled, 2 = Stretched
	; Requirement(s):   None.
	; Return Value(s):  On Success - Returns 0
	;                   On Failure -   -1
	; Author(s):        FlyingBoz
	; Thanks:        Larry - DllCall Example - Tested and Working under XPHome and W2K Pro
	;                     Excalibur - Reawakening my interest in Getting This done.
	;
	;===============================================================================
	If Not FileExists($image) Then Return -1
	$image = FileGetLongName($image)
	If StringRight($image, "3") <> "bmp" Then
		_GDIPlus_Startup()
		$hImage = _GDIPlus_ImageLoadFromFile($image)
		$sCLSID = _GDIPlus_EncodersGetCLSID("BMP")
		$tData = DllStructCreate("int Data")
		DllStructSetData($tData, "Data", $GDIP_EVTTRANSFORMROTATE90)
		$tParams = _GDIPlus_ParamInit(1)
		_GDIPlus_ParamAdd($tParams, $GDIP_EPGTRANSFORMATION, 1, $GDIP_EPTLONG, DllStructGetPtr($tData, "Data"))
		_GDIPlus_ImageSaveToFileEx($hImage, "c:\thesnow.bmp", $sCLSID, DllStructGetPtr($tParams))
		_GDIPlus_Shutdown()
	Else
		FileCopy($image, "c:\thesnow.bmp", 1)
	EndIf
	;The $SPI*  values could be defined elsewhere via #include - if you conflict,
	; remove these, or add if Not IsDeclared "SPI_SETDESKWALLPAPER" Logic
	Local $SPI_SETDESKWALLPAPER = 20
	Local $SPIF_UPDATEINIFILE = 1
	Local $SPIF_SENDCHANGE = 2
	Local $REG_DESKTOP = "HKEY_CURRENT_USER\Control Panel\Desktop"
	If $style = 1 Then
		RegWrite($REG_DESKTOP, "TileWallPaper", "REG_SZ", 1)
		RegWrite($REG_DESKTOP, "WallpaperStyle", "REG_SZ", 0)
	Else
		RegWrite($REG_DESKTOP, "TileWallPaper", "REG_SZ", 0)
		RegWrite($REG_DESKTOP, "WallpaperStyle", "REG_SZ", $style)
	EndIf


	DllCall("user32.dll", "int", "SystemParametersInfo", _
			"int", $SPI_SETDESKWALLPAPER, _
			"int", 0, _
			"str", "c:\thesnow.bmp", _
			"int", BitOR($SPIF_UPDATEINIFILE, $SPIF_SENDCHANGE))
	Return 0
EndFunc   ;==>_ChangeDesktopWallpaper

;===============================================================================
;
; ��������:    _ChangeScreenRes()
; ��ϸ��Ϣ:    �޸� ��Ļ�ֱ���,ˢ����.
; �汾:          1.0.0.1
; ����:     $i_Width - ��Ļ���(��1024X768 �е� 1024)
;             $i_Height - ��Ļ�߶�(��1024X768 �е� 768)
;             $i_BitsPP -������ɫ���(�� 32BIT,32λ)
;             $i_RefreshRate - ��Ļˢ����(�� 75 MHZ).
; ����      AutoIt ���԰� > 3.1 ����
; ����ֵ  :      �ɹ�,��Ļ����,@ERROR = 0
;                   ʧ��,��Ļ������, @ERROR = 1
; ��̳:         http://www.autoitscript.com/forum/index.php?showtopic=20121
; ����:        Original code - psandu.ro
;                Modifications - PartyPooper
; ����:        thesnow
;
;===============================================================================
Func _ChangeScreenRes($i_Width = @DesktopWidth, $i_Height = @DesktopHeight, $i_BitsPP = @DesktopDepth, $i_RefreshRate = @DesktopRefresh)
	Local Const $DM_PELSWIDTH = 0x00080000
	Local Const $DM_PELSHEIGHT = 0x00100000
	Local Const $DM_BITSPERPEL = 0x00040000
	Local Const $DM_DISPLAYFREQUENCY = 0x00400000
	Local Const $CDS_TEST = 0x00000002
	Local Const $CDS_UPDATEREGISTRY = 0x00000001
	Local Const $DISP_CHANGE_RESTART = 1
	Local Const $DISP_CHANGE_SUCCESSFUL = 0
	Local Const $HWND_BROADCAST = 0xffff
	Local Const $WM_DISPLAYCHANGE = 0x007E
	If $i_Width = "" Or $i_Width = -1 Then $i_Width = @DesktopWidth ; default to current setting
	If $i_Height = "" Or $i_Height = -1 Then $i_Height = @DesktopHeight ; default to current setting
	If $i_BitsPP = "" Or $i_BitsPP = -1 Then $i_BitsPP = @DesktopDepth ; default to current setting
	If $i_RefreshRate = "" Or $i_RefreshRate = -1 Then $i_RefreshRate = @DesktopRefresh ; default to current setting
	Local $DEVMODE = DllStructCreate("byte[32];int[10];byte[32];int[6]")
	Local $B = DllCall("user32.dll", "int", "EnumDisplaySettings", "ptr", 0, "long", 0, "ptr", DllStructGetPtr($DEVMODE))
	If @error Then
		$B = 0
		SetError(1)
		Return $B
	Else
		$B = $B[0]
	EndIf
	If $B <> 0 Then
		DllStructSetData($DEVMODE, 2, BitOR($DM_PELSWIDTH, $DM_PELSHEIGHT, $DM_BITSPERPEL, $DM_DISPLAYFREQUENCY), 5)
		DllStructSetData($DEVMODE, 4, $i_Width, 2)
		DllStructSetData($DEVMODE, 4, $i_Height, 3)
		DllStructSetData($DEVMODE, 4, $i_BitsPP, 1)
		DllStructSetData($DEVMODE, 4, $i_RefreshRate, 5)
		$B = DllCall("user32.dll", "int", "ChangeDisplaySettings", "ptr", DllStructGetPtr($DEVMODE), "int", $CDS_TEST)
		If @error Then
			$B = -1
		Else
			$B = $B[0]
		EndIf
		Select
			Case $B = $DISP_CHANGE_RESTART
				$DEVMODE = ""
				Return 2
			Case $B = $DISP_CHANGE_SUCCESSFUL
				DllCall("user32.dll", "int", "ChangeDisplaySettings", "ptr", DllStructGetPtr($DEVMODE), "int", $CDS_UPDATEREGISTRY)
				DllCall("user32.dll", "int", "SendMessage", "hwnd", $HWND_BROADCAST, "int", $WM_DISPLAYCHANGE, _
						"int", $i_BitsPP, "int", $i_Height * 2 ^ 16 + $i_Width)
				$DEVMODE = ""
				Return 1
			Case Else
				$DEVMODE = ""
				SetError(1)
				Return $B
		EndSelect
	EndIf
EndFunc   ;==>_ChangeScreenRes

Func _GUICtrlListViewSetView($hWnd, $iView)
	Local $aView[5] = [0, 1, 2, 3, 4]
	If IsHWnd($hWnd) Then
		Return _SendMessage($hWnd, 0x108e, $aView[$iView]) <> -1
	Else
		Return GUICtrlSendMsg($hWnd, 0x108e, $aView[$iView], 0) <> -1
	EndIf
EndFunc   ;==>_GUICtrlListView_SetView






