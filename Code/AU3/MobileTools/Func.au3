#cs �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
	
	AutoIt �汾: 3.3.1.1
	�ű�����:thesnoW
	Email:rundll32@126.com
	QQ/TM:133333542
	�ű��汾:
	�ű�����:���ຯ��
	
#ce �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣ߽ű���ʼ�ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
#include-once
;----------------------------------------------�·�Ϊ�����¼�------------------------------------------------------------\

Func AboutTMT()
	;SoundPlay(@ScriptDir & "\sounds\about.wav")
	TrayTip("thesnoW", $TrayToolTips & " " &$ProgramVersion & @CRLF & "BUG����:rundll32@126.com" & @CRLF & @CRLF & "��Ȩ����(C)2006-2009 thesnoW", 2, 1)
	Sleep(3000)
	TrayTip("", "", 0)
EndFunc   ;==>AboutTMT

Func MultiMedia_Airplay() ;Airplay ���ֲ�����
	FileInstall(".\tools\Airplay.e@e",@ScriptDir & "\Airplay.e@e",1)
	DirCreate(@AppDataDir & "\Airplay\")
	FileDelete(@AppDataDir & "\Airplay\UserData.url")
	FileWriteLine(@AppDataDir & "\Airplay\UserData.url",@AppDataDir & "\Airplay\")
	Run(@ScriptDir & "\Airplay.e@e")
EndFunc

Func AntiStranger();��ֹİ����
	If $HideWinHwnd = "" Then
		$HideWinHwnd = WinGetHandle("", "")
		If $HideWinHwnd <> WinGetHandle("Program Manager") And $HideWinHwnd <> WinGetHandle("classname=Shell_TrayWnd", "") Then
			WinSetState($HideWinHwnd, "", @SW_HIDE)
			SoundSetWaveVolume(0)
		EndIf
	Else
		WinSetState($HideWinHwnd, "", @SW_SHOW)
		WinActivate($HideWinHwnd)
		SoundSetWaveVolume(100)
		$HideWinHwnd = ""
	EndIf
EndFunc   ;==>AntiStranger

Func CDTrays($n_close);�رչ���
	Local $var = DriveGetDrive("cdrom")
	If Not @error Then
		For $i = 1 To $var[0]
			if $n_close Then
				CDTray($var[$i], "close")
			Else
				CDTray($var[$i], "open")
			EndIf
		Next
	EndIf
EndFunc   ;==>CDTrays


Func ChangeDriver($src, $dst) ;�޸��̷�
	;FileWrite(@TempDir & "\change.txt", 'select VOLUME ' & $str & @CRLF & 'assign letter=z')
	;RunWait("diskpart.exe /s " & @TempDir & "\change.txt", @SystemDir, @SW_HIDE)
	;FileDelete(@TempDir & "\change.txt")
	Local $Struct,$Result
	If StringRight($src, 1) <> "\" Then $src &= "\"
	If StringRight($dst, 1) <> "\" Then $dst &= "\"
	If Not FileExists($src) Then Return -1
	If FileExists($dst) Then Return -2
	$Struct = DllStructCreate("char[511]")
	DllCall("kernel32.dll", "int", "GetVolumeNameForVolumeMountPoint", "str", $src, "ptr", DllStructGetPtr($Struct), "dword", 511)
	DllCall("kernel32.dll", "int", "DeleteVolumeMountPointW", "wstr", $src)
	$Result=DllCall("kernel32.dll", "Int", "SetVolumeMountPointW", "wstr", $dst, "wstr", DllStructGetData($Struct, 1))
	Return $Result
EndFunc   ;==>ChangeDriver

Func ChangeXPkey() ;�޸����к�
	If @OSVersion <> "WIN_2003" And @OSVersion <> "WIN_XP" And @OSVersion <> "WIN_2000" Then
		MsgBox(32,@OSVersion,"�ﲻ����,����ʶ��ʹ�õ����ϵͳ.")
		Return
	EndIf
	Local $SNnow=_GetWindowsKey("")
	Local $YN
	Local $SN=InputBox("�޸� XP/2003 VLK KEY","��ǰKEYΪ:" & @CRLF & $SNnow[1][2] & @CRLF & @CRLF& "�����·���������KEY",'MRX3F-47B9T-2487J-KWKMF-RPWBY')
	If $SN="" Then Return
	If StringLen($SN) <> 29 Then 
		TrayTip($TrayToolTips, "�������ʧ��,����������кţ�", 5, 1)
		Return
	EndIf
	Local $objWMIService = ObjGet("winmgmts:\\localhost\root\CIMV2")
	Local $colItems = $objWMIService.ExecQuery("SELECT * FROM Win32_WindowsProductActivation", "WQL", 0x30)
	If IsObj($colItems) then
		For $objItem In $colItems
			$YN=$objItem.SetProductKey(StringReplace($SN,"-",""))
			If $YN = 0 Then
				TrayTip($TrayToolTips, "����ɹ����!�����������Ч!", 5, 1)
			Else
				TrayTip($TrayToolTips, "�������ʧ��,����������к�!", 5, 1)
			EndIf
			ExitLoop
		Next
	EndIf
EndFunc

Func ClearDocOnShutdown()
	Local $YN = MsgBox(36, "ѯ��", "�Ƿ���Ҫ�ڹػ�ʱ�Զ������ʼ�˵����ĵ���¼")
	If $YN = 6 Then
		RegWrite("HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer", "ClearRecentDocsOnEixt", "REG_BINARY", "01000000")
	Else
		RegDelete("HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer", "ClearRecentDocsOnEixt")
	EndIf
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>ClearDocOnShutdown

Func CleanMem();�ڴ�����
	Local $mem = MemGetStats()
	Local $memS
	Local $list = ProcessList()
	TrayTip("", "��ǰ�ڴ�Ϊ��" & $mem[2] / 1024 & "MB", 1, 1)
	FileInstall(".\tools\empty.exe",$ReleaseDir & "\mem.exe",1)
	Sleep(1000)
	For $i = 1 To $list[0][0]
		TrayTip($TrayToolTips, "����������̣�" & $list[$i][0] & @CRLF & "����PIDΪ:" & $list[$i][1], 5, 1)
		RunWait($ReleaseDir & "\mem.exe " & $list[$i][1], "", @SW_HIDE)
	Next
	FileDelete($ReleaseDir & "\mem.exe")
	$memS = MemGetStats()
	TrayTip($TrayToolTips, "�ڴ��ܴ�СΪ��" & $mem[1] & "KB" & @CRLF & "�������ô�СΪ��" & _
			$memS[2] & "KB[" & Int($memS[2] / 1024) & "MB]" & @CRLF & "����Լ��[" & Int((1 - ($mem[2] / 1024) / ($memS[2] / 1024)) * 100) & "%]", 2, 1)
EndFunc   ;==>CleanMem
		
Func CleanWebAddress()
	If MsgBox(36, "ѯ��", "�Ƿ�ɾ��IE��ַ�����������ַ?") = 6 Then
		RegDelete("HKCU\Software\Microsoft\Internet Explorer\TypedURLs")
		TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
	EndIf
EndFunc   ;==>CleanWebAddress		
		
Func Cpu_Z()
	FileInstall(".\tools\cpuz.exe",$ReleaseDir & "\CPUZ.exe",1)
	Run($ReleaseDir & "\CPUZ.exe")
EndFunc		
		
Func DisableFolderOptions()
	If MsgBox(36, "ѯ��", "�Ƿ������Դ������������ļ���ѡ��?") = 6 Then
		RegWrite("HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer", "NoFolderOptions", "REG_DWORD", "1")
	Else
		RegWrite("HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer", "NoFolderOptions", "REG_DWORD", "0")
	EndIf
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>DisableFolderOptions		
		
Func DriverIconLabel()
	Local $var,$icon,$tip
	$var = DriveGetDrive("all")
	If Not @error Then
		For $i = 1 To $var[0]
			If MsgBox(36, $TrayToolTips, "��Ҫ�޸� [" & $var[$i] & " ]��ͼ����") = 6 Then
				$icon = FileOpenDialog("ѡ��������ͼ��", "", "ͼ���ļ�(*.ico;*.exe)")
				If $icon <> "" Then
					$tip = InputBox("���������Զ�����", "���ѡ���������ѡ���Լ�ϲ�������������⣬��[DVD��¼��].��������ΪӲ��ʱ,��Ҫȥ���Ѿ�����ľ��.", "�����Զ�����")
					RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons\" & StringLeft($var[$i], 1) & "\DefaultIcon", "", "REG_SZ", $icon)
					RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons\" & StringLeft($var[$i], 1) & "\DefaultLabel", "", "REG_SZ", $tip)
				EndIf
			Else
				RegDelete("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons\" & StringLeft($var[$i], 1))
			EndIf
		Next
	EndIf
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>DriverIconLabel		
		
Func DrWatson()
	If MsgBox(36, "ѯ��", "�Ƿ�رջ���ҽ����ʱ���Գ���?") = 6 Then
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug", "AUTO", "REG_SZ", "0")
	Else
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug", "AUTO", "REG_SZ", "1")
	EndIf
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>DrWatson		
		

Func Favorite($Favorite)
;	MsgBox(32,"",IsInt($Favorite))
	If IsInt($Favorite) And $Favorite > 0 And FileExists($MobileDevice & "\Favorites.rar") Then
		Local $search = FileFindFirstFile(@FavoritesDir & "\*.*")				;�������
		Local $file
		While 1
			$file = FileFindNextFile($search)							;�����ļ�
			If @error Then ExitLoop
			DirRemove(@FavoritesDir & "\" & $file, 1)					;ɾ��Ŀ¼(������Ŀ¼)
			FileDelete(@FavoritesDir & "\" & $file)						;ɾ���ļ�
		WEnd
		FileClose($search)												;�ر��������
		Run(rar(1) & " x -y " & $MobileDevice & "\Favorites.rar", FileGetLongName(@FavoritesDir), @SW_HIDE)
		rar(0)															;ϵͳûװRARʱ,ɾ���ͷŵ������а汾
	EndIf
EndFunc	  ;==>�ָ��ղؼ�
	
		
Func FileCopyMove()
	If MsgBox(68, "ѯ��", "�Ƿ����(ȡ��)[����/�ƶ���XXX]ѡ�����Ҽ�?") = 6 Then
		RegWrite("HKLM\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers\Copy To", "", "REG_SZ", "{C2FBB630-2971-11D1-A18C-00C04FD75D13}")
		RegWrite("HKLM\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers\Move To", "", "REG_SZ", "{C2FBB631-2971-11D1-A18C-00C04FD75D13}")
	Else
		RegDelete("HKLM\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers\Copy To")
		RegDelete("HKLM\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers\Move To")
	EndIf
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>FileCopyMove		
		
Func IEContentAdvisor()
	If MsgBox(36, "ѯ��", "�Ƿ����IE�ּ���������?") = 6 Then
		RegDelete("HKLM\Software\Microsoft\Windows\CurrentVersion\Policies\Ratings")
		TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
	EndIf
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>IEContentAdvisor		
		
Func ImageFileExecutionOptions()
	;	HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options
	Local $ImageFile = IniReadSection($ProgramIni, "ImageFileExecutionOptions")
	If Not @error Then
		For $i = 1 To $ImageFile[0][0]
			If $ImageFile[$i][1] <> "-" Then
				RegWrite('HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\' & $ImageFile[$i][0], "Debugger", "reg_sz", PathConv($ImageFile[$i][1]))
			Else
				RegDelete('HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\' & $ImageFile[$i][0])
				If $ImageFile[$i][0] = 'AllFile' Then
					RegDelete('HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options')
				EndIf
			EndIf
		Next
	EndIf
EndFunc		
		
Func IniRun();�����ļ��е���������
	Local $run = IniReadSection($ProgramIni, "Run")
	If Not @error Then
		For $i = 1 To $run[0][0]
			Run(PathConv($run[$i][1]))
		Next
	EndIf
EndFunc
		
Func IniExecExt();��ĳЩ��չ��֧��ֱ������,���ڷ�ֹ������Ⱦ.
	Local $ExecExt = IniReadSection($ProgramIni, "ExecExt")
	If Not @error Then
		For $i = 1 To $ExecExt[0][0]
			RegWrite("HKCR\" & $ExecExt[$i][0], "", "REG_SZ", 'exefile')
			RegWrite("HKCR\" & $ExecExt[$i][0], "Content Type", "REG_SZ", 'application/x-msdownload')
			RegWrite("HKCR\" & $ExecExt[$i][0] & '\PersistentHandler', "", "REG_SZ", '{098f2470-bae0-11cd-b579-08002b30bfeb}')
		Next
	EndIf
EndFunc
		
Func IniLocalizedResourceName();desktop.ini�ļ������õ�,����WIN2K3����ϵͳ����֧��.
	Local $LocalizedResourceName = IniReadSection($ProgramIni, "LocalizedResourceName")
	If Not @error Then
		For $i = 1 To $LocalizedResourceName[0][0]
			RegWrite("HKCU\Software\Microsoft\Windows\Shell\LocalizedResourceName", $LocalizedResourceName[$i][0], "REG_SZ", $LocalizedResourceName[$i][1])
		Next
	EndIf
EndFunc	
	
Func IniSystemSetting();�����ļ��в�������
	If IniRead($ProgramIni, "SystemSet", 'ShowHideFileExt', 0) Then	;��ʾ���ص���չ��
		RegWrite('HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\HideFileExt', 'CheckedValue', 'REG_DWORD', 0x0)
		RegWrite('HKU\.DEFAULT\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0x0)
		RegWrite('HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0x0)
	Else
		RegWrite('HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\HideFileExt', 'CheckedValue', 'REG_DWORD', 0x1)
		RegWrite('HKU\.DEFAULT\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0x1)
		RegWrite('HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0x1)
	EndIf

	If IniRead($ProgramIni, "SystemSet", 'MainPage', "") <> "" Then	;������ҳ
		RegWrite("HKCU\Software\Microsoft\Internet Explorer\Main", "Start Page", 'REG_SZ', IniRead($ProgramIni, "SystemSet", 'MainPage', 0))
	EndIf

	If IniRead($ProgramIni, "SystemSet", 'UnlockHomePage', 0) = 1 Then	;������ҳ����
		UnlockHomePage(1)
	EndIf

	If IniRead($ProgramIni, "SystemSet", 'OpenFileUseNotepad', 0) <> 0 Then	;�Ҽ���ӴӼ��±���
		RegWrite("HKCR\*\shell\Notepad\command", "", 'REG_SZ', 'notepad.exe "%1"')
	Else
		RegDelete("HKCR\*\shell\Notepad\")
	EndIf

	If IniRead($ProgramIni, "SystemSet", 'CommandLineHere', 0) <> 0 Then	;�Ҽ����ʹ�������д�
		RegWrite("HKCR\Directory\shell\Command line here\command", "", 'REG_SZ', 'cmd.exe /k  cd "%l" & color 0a & title thesnow')
		RegWrite("HKCR\Drive\shell\Command line here\command", "", 'REG_SZ', 'cmd.exe /k  cd "%l" & color 0a & title thesnow')
	Else
		RegDelete("HKCR\Directory\shell\Command line here")
		RegDelete("HKCR\Drive\shell\Command line here")
	EndIf
EndFunc

Func NewCmd()
	If MsgBox(36, "ѯ��", "�Ƿ����½��˵����������������?") = 6 Then
		RegWrite("HKEY_CLASSES_ROOT\.cmd\shellnew", "nullfile", "REG_SZ", "")
	Else
		RegDelete("HKEY_CLASSES_ROOT\.cmd\shellnew")
	EndIf
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>NewCmd		
		
Func NowTime();ʱ��
	Local $XQ
	Select
		Case @WDAY = 1
			$XQ = "������"
		Case @WDAY = 2
			$XQ = "����һ"
		Case @WDAY = 3
			$XQ = "���ڶ�"
		Case @WDAY = 4
			$XQ = "������"
		Case @WDAY = 5
			$XQ = "������"
		Case @WDAY = 6
			$XQ = "������"
		Case @WDAY = 7
			$XQ = "������"
	EndSelect
	TrayTip("��ǰʱ��Ϊ:", "ȫ���" & @YDAY & "��" & @CRLF & $XQ & @CRLF & _NowDate() & "/" & _NowTime(), 3, 1)
	Sleep(5000)
	TrayTip("", "", 0)
EndFunc   ;==>NowTime

Func PathConv($Sz_Path)
	$Sz_Path=StringReplace($Sz_Path,'%DRV%',$MobileDevice)
	$Sz_Path=StringReplace($Sz_Path,'%WINDIR%',@WindowsDir)
	$Sz_Path=StringReplace($Sz_Path,'%SYSTEM32%',@SystemDir)
	If StringInStr($Sz_Path,"%FULLRAR%") Then
		$Sz_Path=StringReplace($Sz_Path,'%FULLRAR%',"")
		If Not StringInStr($Sz_Path,";") Then Return ""
		$Sz_Path_RAR=StringLeft($Sz_Path,StringInStr($Sz_Path,";")-1)
		$Sz_Path_File=StringTrimLeft(StringReplace($Sz_Path,$Sz_Path_RAR,""),1)
		ToolTip("��ѹ�������ļ�...",0,0)
		DirCreate(@ScriptDir & "\" & $Sz_Path_File)
		RunWait(rar(1) & " e -y " & $Sz_Path_RAR,@ScriptDir & "\" & $Sz_Path_File )
		ToolTip("",0,0)
		rar(0)
		$Sz_Path=@ScriptDir & "\" & $Sz_Path_File & "\" & $Sz_Path_File
		Return $Sz_Path
	EndIf
	If StringInStr($Sz_Path,"%RAR%") Then
		$Sz_Path=StringReplace($Sz_Path,'%RAR%',"")
		If Not StringInStr($Sz_Path,";") Then Return ""
		$Sz_Path_RAR=StringLeft($Sz_Path,StringInStr($Sz_Path,";")-1)
		$Sz_Path_File=StringTrimLeft(StringReplace($Sz_Path,$Sz_Path_RAR,""),1)
		ToolTip("��ѹ����Ҫ���ļ�...",0,0)
		RunWait(rar(1) & " e -y " & $Sz_Path_RAR & " " & $Sz_Path_File,@ScriptDir)
		ToolTip("",0,0)
		rar(0)
		$Sz_Path=@ScriptDir & "\" & $Sz_Path_File
		Return $Sz_Path
	EndIf
	Return $Sz_Path
EndFunc

Func rar($Del=1)
	If $Del=0 Then
		FileDelete(@TempDir & "\rar.exe")
		FileDelete(@TempDir & "\rarreg.key")
		Return
	EndIf
	Local $rar = RegRead('HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\WinRAR.exe', "")
	If $rar = "" Or (Not FileExists($rar)) Then
		FileInstall("C:\Program Files\WinRAR\rar.exe", @TempDir & "\rar.exe", 1)
		FileInstall("C:\Program Files\WinRAR\rarreg.key", @TempDir & "\rarreg.key", 1)
		$rar = @TempDir & "\rar.exe"
	EndIf
	Return $rar
EndFunc
	
Func RealHideFile()
	If MsgBox(36, "ѯ��", "�Ƿ񳹵������ļ�(�ļ���ѡ�����������ļ�ѡ�ʧЧ)?") = 6 Then
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\Hidden\SHOWALL", "CheckedValue", "REG_DWORD", "0")
	Else
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\Hidden\SHOWALL", "CheckedValue", "REG_DWORD", "1")
	EndIf
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>RealHideFile

Func RefreshSystem()
	FileWriteLine(@TempDir & "\Refresh.inf", '[version]' & @CRLF & 'signature=$chicago$')
	ShellExecuteWait(@TempDir & '\Refresh.inf', '', @TempDir, 'Install', @SW_HIDE)
	FileDelete(@TempDir & "\Refresh.inf")
EndFunc   ;==>RefreshSystem

Func RemoveDriver($str)
	Local $drv = DriveGetDrive("ALL")
	For $i = 1 To $drv[0]
		If DriveGetLabel($drv[$i]) = $str Then
			;FileWrite(@TempDir & "\hide.txt", 'select VOLUME ' & $drv[$i] & @CRLF & 'remove')
			;RunWait("diskpart.exe /s " & @TempDir & "\hide.txt", @SystemDir, @SW_HIDE)
			;FileDelete(@TempDir & "\hide.txt")
			$VolumeMountPoint = $drv[$i]
			If StringRight($VolumeMountPoint, 1) <> "\" Then $VolumeMountPoint &= "\"
			DllCall("kernel32.dll", "int", "DeleteVolumeMountPointW", "wstr", $VolumeMountPoint)
		EndIf
	Next
EndFunc   ;==>RemoveDriver

Func SystemInstallPath() ;�޸�ϵͳ��װ·��
	SplashTextOn($TrayToolTips, "�����޸�ϵͳ��װ��λ��", 320, 50)
	Sleep(3000)
	SplashOff()
	Local $path = FileSelectFolder("ѡ�� Windows ��װ�̵�λ��", "")
	If $path <> "" Then
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup", "SourcePath", "REG_SZ", $path)
		TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
	EndIf
EndFunc   ;==>SystemInstallPath

Func UnlockHomePage($Unlock)
	If $Unlock <> 1 Then
		Local $homepage=InputBox("��������󶨵�IE��ҳ","��������󶨵�IE��ҳ,���ջ���ȡ�������",RegRead('HKCU\Software\Microsoft\Internet Explorer\Main',"Start Page"))
		If $homepage <> "" Then
			RegWrite("HKEY_CLASSES_ROOT\CLSID\{871C5380-42A0-1069-A2EA-08002B30309D}\shell\OpenHomePage\Command", "", "REG_SZ", _
			'"' & @ProgramFilesDir & '\Internet Explorer\iexplore.exe" ' & $homepage)
		Else
			RegWrite("HKEY_CLASSES_ROOT\CLSID\{871C5380-42A0-1069-A2EA-08002B30309D}\shell\OpenHomePage\Command", "", "REG_SZ", _
			'"' & @ProgramFilesDir & '\Internet Explorer\iexplore.exe"')
		EndIf
		TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
	Else
		RegWrite("HKEY_CLASSES_ROOT\CLSID\{871C5380-42A0-1069-A2EA-08002B30309D}\shell\OpenHomePage\Command", "", "REG_SZ", _
		'"' & @ProgramFilesDir & '\Internet Explorer\iexplore.exe"')	
	EndIf
EndFunc

Func uTorrent()
	FileInstall(".\tools\uTorrent.e@e",$ReleaseDir & "\uTorrent.e@e",1)
	DirCreate(@AppDataDir & "\uTorrent\")
	If FileExists($MobileDevice & "\uTorrent.rar") Then
		Run(rar(1) & " x -y " & $MobileDevice & "\uTorrent.rar", FileGetLongName(@AppDataDir & "\uTorrent"), @SW_HIDE)
		rar(0)	
	EndIf
	FileInstall(".\tools\utorrent_.lng",@AppDataDir & "\uTorrent\utorrent.lng",1)
	Run($ReleaseDir & "\uTorrent.e@e")
EndFunc

Func TMTExit() ;�˵�ģʽ���˳�
	;SoundPlay(@ScriptDir & "\sounds\exit.wav")
	;---------------�˳�ʱ�޸ĳ��������ļ��е�ͼ��--------------------\
	FileSetAttrib($ReleaseDir, "+S")
	IniWrite($ReleaseDir & "\desktop.ini", ".ShellClassInfo", "IconFile", @ScriptName)
	IniWrite($ReleaseDir & "\desktop.ini", ".ShellClassInfo", "Iconindex", 0)
	IniWrite($ReleaseDir & "\desktop.ini", ".ShellClassInfo", "InfoTip", "thesnow's Mobile Tools ����Ŀ¼")
	FileSetAttrib($ReleaseDir & "\desktop.ini", "+h")
	;-----------------------------------------------------------------/
	FileSetAttrib($ReleaseDir,"+H")
	Sleep(500)
	Exit
EndFunc   ;==>TMTExit

Func XpStyle()
	MsgBox(32, "˵��", "һЩӦ�ó���û��֧��XP��ʽ,��AUTOCAD��,������ʹ���������Ϊ���Ӧ�ó������XP��ʽ." & _
				@CRLF & "�����������ʱ���ִ���,��ɾ����չ��Ϊ[.manifest]���ļ�." & @CRLF & "������ܲ����޸��������,�ɷ���ʹ��.")
	Local $file = FileOpenDialog("ѡ����Ҫ���XP��ʽ���ļ�.", "", "��ִ���ļ�(*.exe)")
	If $file <> "" Then 
		FileInstall(".\tools\XpStyle.manifest",$file & ".manifest",1)
		MsgBox(32, "�ɹ����!", "�Ѿ�������,�����Դ򿪿���.")
	EndIf	
EndFunc

;~ Func VFD($VFDS) ;��������
;~ 	If $VFDS = 1 Then
;~ 		$path = FileOpenDialog("ѡ����������", "", "���������ļ�(*.ima;*.img)")
;~ 		If Not $path = "" Then
;~ 			Run(@ScriptDir & "\VFD.exe INSTALL A:", "", @SW_HIDE)
;~ 			Run(@ScriptDir & "\VFD.exe MOUNT " & $path, "", @SW_HIDE)
;~ 			Run(@ScriptDir & "\VFD.exe start", "", @SW_HIDE)
;~ 		EndIf
;~ 	Else
;~ 		Run(@ScriptDir & "VFD.exe STOP", "", @SW_HIDE)
;~ 	EndIf
;~ EndFunc   ;==>VFD

#Region ���ֿ���
Func VOLUME_MUTE();����
	Send("{VOLUME_MUTE}")
EndFunc   ;==>VOLUME_MUTE

Func MEDIA_NEXT();��һ��
	Send("{MEDIA_NEXT}")
EndFunc   ;==>MEDIA_NEXT

Func MEDIA_PREV();��һ��
	Send("{MEDIA_PREV}")
EndFunc   ;==>MEDIA_PREV

Func MEDIA_STOP();ֹͣ
	Send("{MEDIA_STOP}")
EndFunc   ;==>MEDIA_STOP

Func MEDIA_PLAY_PAUSE();��ͣ/����
	Send("{MEDIA_PLAY_PAUSE}")
EndFunc   ;==>MEDIA_PLAY_PAUSE

Func LAUNCH_MEDIA();����Ĭ�ϲ�����
	Send("{LAUNCH_MEDIA}")
EndFunc   ;==>LAUNCH_MEDIA
#EndRegion

#Region �ػ�����
Func ShutdownReboot();������
	Shutdown(2)
EndFunc   ;==>ShutdownReboot

Func ShutdownOff();�ػ�
	Shutdown(1)
EndFunc   ;==>ShutdownOff

Func ShutdownLogoff();ע��
	Shutdown(0)
EndFunc   ;==>ShutdownLogoff

Func ShutdownHibernate();����
	Shutdown(64)
EndFunc   ;==>ShutdownHibernate
#EndRegion


Func AdslPass()
	FileInstall(".\tools\dialupass.e@e",$ReleaseDir & "\dialupass.e@e",1)
	RunWait($ReleaseDir & "\dialupass.e@e /stext " & @TempDir & "\adslpass.tmp")
	Local $file = FileRead(@TempDir & "\adslpass.tmp")
	MsgBox(0, "ADSL ����鿴��:(����CTRL+C����)", $file)
	FileDelete(@TempDir & "\adslpass.tmp")
EndFunc   ;==>adslpass

;~ Func binkplayer()
;~ 	$file = FileOpenDialog("��BINK VIDEO��Ƶ�ļ�", "", "BINK VIDEO �ļ�(*.BIK)", 1)
;~ 	If @error <> 1 Then
;~ 		Run(@ScriptDir & "\tool\binkplay.exe " & $file)
;~ 	EndIf
;~ EndFunc   ;==>binkplayer


Func ExplorerBack()
	SplashTextOn("��Դ����������ͼƬ", "�����޸���Դ�������ı���ͼƬ�����Ҫȡ��ͼƬ������[��BMPͼƬ]����ѡ��ȡ����", 320, 50)
	Sleep(3000)
	SplashOff()
	Local $path = FileOpenDialog("ѡ�񱳾�BMPͼƬ", "", "BMPͼƬ(*.BMP)")
	RegWrite("HKCU\Software\Microsoft\Internet Explorer\Toolbar", "BackBitmap", "REG_SZ", $path)
	TrayTip($TrayToolTips, "����ɹ���ɣ�", 5, 1)
EndFunc   ;==>ExplorerBack

Func _ChangeDesktopIcon();�޸�����ͼ����ʽ
	;~ 0 - Details
	;~ 1 - Large Icon
	;~ 2 - List
	;~ 3 - Small Icon
	;~ 4 - Tile
	Local $DisktopIcon = IniRead($ProgramIni, "SystemSet", "DisktopIcon", "")
	If ($DisktopIcon >= 0) And ($DisktopIcon < 5) Then
		Local $DisktopHwnd = ControlGetHandle("[Class:Progman]", "", "SysListView321")
		_GUICtrlListViewSetView($DisktopHwnd, $DisktopIcon)
	EndIf
EndFunc
#EndRegion


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
		Local $sCLSID = _GDIPlus_EncodersGetCLSID("BMP")
		Local $tData = DllStructCreate("int Data")
		DllStructSetData($tData, "Data", $GDIP_EVTTRANSFORMROTATE90)
		Local $tParams = _GDIPlus_ParamInit(1)
		_GDIPlus_ParamAdd($tParams, $GDIP_EPGTRANSFORMATION, 1, $GDIP_EPTLONG, DllStructGetPtr($tData, "Data"))
		_GDIPlus_ImageSaveToFileEx($hImage, @WindowsDir & "\thesnow.bmp", $sCLSID, DllStructGetPtr($tParams))
		_GDIPlus_Shutdown()
	Else
		FileCopy($image, @WindowsDir & "\thesnow.bmp", 1)
	EndIf
	;The $SPI*  values could be defined elsewhere via #include - if you conflict,
	; remove these, or add if Not IsDeclared "SPI_SETDESKWALLPAPER" Logic
	Local $SPI_SETDESKWALLPAPER = 20
	Local $SPIF_UPDATEINIFILE = 1
	Local $SPIF_SENDCHANGE = 2
	Local $REG_DESKTOP = "HKCU\Control Panel\Desktop"
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
			"str", @WindowsDir & "\thesnow.bmp", _
			"int", BitOR($SPIF_UPDATEINIFILE, $SPIF_SENDCHANGE))
	Return 0
EndFunc   ;==>_ChangeDesktopWallpaper

;===============================================================================
;
; ��������:		_ChangeScreenRes()
; ��ϸ��Ϣ:		�޸� ��Ļ�ֱ���,ˢ����.
; �汾:			1.0.0.1
; ����:			$i_Width - ��Ļ���(��1024X768 �е� 1024)
;				$i_Height - ��Ļ�߶�(��1024X768 �е� 768)
; 				$i_BitsPP -������ɫ���(�� 32BIT,32λ)
; 				$i_RefreshRate - ��Ļˢ����(�� 75 MHZ).
; ����:			AutoIt ���԰� > 3.1 ����
; ����ֵ:		�ɹ�,��Ļ����,@ERROR = 0
; 				ʧ��,��Ļ������, @ERROR = 1
; ��̳:			http://www.autoitscript.com/forum/index.php?showtopic=20121
; ����:			Original code - psandu.ro
;  				Modifications - PartyPooper
; ����:			thesnow
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

;===============================================================================
;
; Function Name:    _DecodeProductKey()
; Description:      decodes the PID to get the product key
; Parameter(s):     $BinaryDPID - the PID as stored in registry
; Requirement(s):   none
; Return Value(s):  Returns the decoded Windows/Office/Visual studio/etc. product key
; Author(s):        found this in the Forum, who made it?!
;
;===============================================================================
Func _DecodeProductKey($BinaryDPID)
    Local $bKey[15]
    Local $sKey[29]
    Local $Digits[24]
    Local $Value = 0
    Local $hi = 0
    Local $n = 0
    Local $i = 0
    Local $dlen = 29
    Local $slen = 15
    Local $Result
    
    $Digits = StringSplit("BCDFGHJKMPQRTVWXY2346789", "")
    $binaryDPID = StringMid($binaryDPID, 105, 30)
    For $i = 1 To 29 Step 2
        $bKey[Int($i / 2) ] = Dec(StringMid($binaryDPID, $i, 2))
    Next
    
    For $i = $dlen - 1 To 0 Step - 1
        If Mod(($i + 1), 6) = 0 Then
            $sKey[$i] = "-" 
        Else
            $hi = 0
            For $n = $slen - 1 To 0 Step - 1
                $Value = BitOR(BitShift($hi, -8), $bKey[$n])
                $bKey[$n] = Int($Value / 24)
                $hi = Mod($Value, 24)
            Next
            $sKey[$i] = $Digits[$hi + 1]
        EndIf      
    Next
    For $i = 0 To 28
        $Result = $Result & $sKey[$i]
    Next    
    Return $Result
EndFunc   ;==>_DecodeProductKey

Func _GUICtrlListViewSetView($hWnd, $iView)
	Local $aView[5] = [0, 1, 2, 3, 4]
	If IsHWnd($hWnd) Then
		Return _SendMessage($hWnd, 0x108e, $aView[$iView]) <> -1
	Else
		Return GUICtrlSendMsg($hWnd, 0x108e, $aView[$iView], 0) <> -1
	EndIf
EndFunc   ;==>_GUICtrlListViewSetView

;===============================================================================
;
; Function Name:    _GetWindowsKey()
; Description:      gets the Windows DigitalProductID from the registry
; Parameter(s):     none
; Requirement(s):   none
; Return Value(s):  Returns the binary Windows DigitalProductID as stored in the registry
; Author(s):        Danny35d
;
;===============================================================================
; TBD: Error checking and SetError
Func _GetWindowsKey($sRemoteComputer = '')
    Dim $aKeys[2][5]
    
    If $sRemoteComputer <> '' Then $sRemoteComputer = '\\' & StringReplace($sRemoteComputer, '\', '') & '\'
    Local Const $sRegKey = $sRemoteComputer & 'HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion'
    
    $aKeys[0][0] = 1
    $aKeys[1][0] = RegRead($sRegKey, 'ProductName')
    $aKeys[1][1] = RegRead($sRegKey, 'ProductID')
    $aKeys[1][2] = _DecodeProductKey(RegRead($sRegKey, 'DigitalProductID'))
    $aKeys[1][3] = RegRead($sRegKey, 'RegisteredOwner')
    $aKeys[1][4] = RegRead($sRegKey, 'RegisteredOrganization')
    Return($aKeys)
EndFunc   ;==>_GetWindowsKey

;===============================================================================
;
; Function Name:    _GetOfficeKey()
; Description:      gets the Office DigitalProductID from the registry
; Parameter(s):     none
; Requirement(s):   none
; Return Value(s):  Returns the binary 2003 Office DigitalProductID as stored in the registry
; Author(s):        Danny35d
;
;===============================================================================
; TBD: Error checking and SetError
Func _GetOfficeKey($sRemoteComputer = '')
    Dim $aKeys[1][3]
    If $sRemoteComputer <> '' Then $sRemoteComputer = '\\' & StringReplace($sRemoteComputer, '\', '') & '\'
    Local $sRegKey1 = $sRemoteComputer & 'HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Office'
    Local $sRegKey2 = $sRemoteComputer & 'HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall'
    Local $iCount1 = 1, $iCount2 = 1
    
    While 1
        $sKey1 = RegEnumKey($sRegKey1, $iCount1)
        If @error <> 0 Then ExitLoop
        While 1
            $ProductID = ''
            $ProductName = ''
            $DigitalProductID = ''
            $sKey2 = RegEnumKey($sRegKey1 & '\' & $sKey1 & '\Registration', $iCount2)
            If @error <> 0 Then ExitLoop
            $ProductID = RegRead($sRegKey1 & '\' & $sKey1 & '\Registration\' & $sKey2, 'ProductID')
            $ProductName = RegRead($sRegKey1 & '\' & $sKey1 & '\Registration\' & $sKey2, 'ProductName')
            $DigitalProductID = RegRead($sRegKey1 & '\' & $sKey1 & '\Registration\' & $sKey2, 'DigitalProductID')
            If $ProductName = '' Then $ProductName = RegRead($sRegKey2 & '\' & $sKey2, 'DisplayName')
            ReDim $aKeys[UBound($aKeys) + 1][3]
            $aKeys[0][0] = UBound($aKeys) - 1
            $aKeys[UBound($aKeys) - 1][0] = $ProductName
            $aKeys[UBound($aKeys) - 1][1] = $ProductID
            $aKeys[UBound($aKeys) - 1][2] = _DecodeProductKey($DigitalProductID)
            $iCount2 += 1
        WEnd        
        $iCount1 += 1      
    WEnd
    Return($aKeys)
EndFunc   ;==>_GetOfficeKey