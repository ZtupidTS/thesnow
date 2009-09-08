#cs ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
	
	AutoIt 版本: 3.3.1.1
	脚本作者:thesnoW
	Email:rundll32@126.com
	QQ/TM:133333542
	脚本版本:
	脚本功能:各类函数
	
#ce ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿脚本开始＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
#include-once
;----------------------------------------------下方为函数事件------------------------------------------------------------\

Func AboutTMT()
	;SoundPlay(@ScriptDir & "\sounds\about.wav")
	TrayTip("thesnoW", $TrayToolTips & " " &$ProgramVersion & @CRLF & "BUG反馈:rundll32@126.com" & @CRLF & @CRLF & "版权所有(C)2006-2009 thesnoW", 2, 1)
	Sleep(3000)
	TrayTip("", "", 0)
EndFunc   ;==>AboutTMT

Func MultiMedia_Airplay() ;Airplay 音乐播放器
	FileInstall(".\tools\Airplay.e@e",@ScriptDir & "\Airplay.e@e",1)
	DirCreate(@AppDataDir & "\Airplay\")
	FileDelete(@AppDataDir & "\Airplay\UserData.url")
	FileWriteLine(@AppDataDir & "\Airplay\UserData.url",@AppDataDir & "\Airplay\")
	Run(@ScriptDir & "\Airplay.e@e")
EndFunc

Func AntiStranger();防止陌生人
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

Func CDTrays($n_close);关闭光驱
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


Func ChangeDriver($src, $dst) ;修改盘符
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

Func ChangeXPkey() ;修改序列号
	If @OSVersion <> "WIN_2003" And @OSVersion <> "WIN_XP" And @OSVersion <> "WIN_2000" Then
		MsgBox(32,@OSVersion,"帮不了你,不认识你使用的这个系统.")
		Return
	EndIf
	Local $SNnow=_GetWindowsKey("")
	Local $YN
	Local $SN=InputBox("修改 XP/2003 VLK KEY","当前KEY为:" & @CRLF & $SNnow[1][2] & @CRLF & @CRLF& "请在下方输入您的KEY",'MRX3F-47B9T-2487J-KWKMF-RPWBY')
	If $SN="" Then Return
	If StringLen($SN) <> 29 Then 
		TrayTip($TrayToolTips, "命令完成失败,检查您的序列号！", 5, 1)
		Return
	EndIf
	Local $objWMIService = ObjGet("winmgmts:\\localhost\root\CIMV2")
	Local $colItems = $objWMIService.ExecQuery("SELECT * FROM Win32_WindowsProductActivation", "WQL", 0x30)
	If IsObj($colItems) then
		For $objItem In $colItems
			$YN=$objItem.SetProductKey(StringReplace($SN,"-",""))
			If $YN = 0 Then
				TrayTip($TrayToolTips, "命令成功完成!重启计算机生效!", 5, 1)
			Else
				TrayTip($TrayToolTips, "命令完成失败,检查您的序列号!", 5, 1)
			EndIf
			ExitLoop
		Next
	EndIf
EndFunc

Func ClearDocOnShutdown()
	Local $YN = MsgBox(36, "询问", "是否需要在关机时自动清除开始菜单的文档记录")
	If $YN = 6 Then
		RegWrite("HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer", "ClearRecentDocsOnEixt", "REG_BINARY", "01000000")
	Else
		RegDelete("HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer", "ClearRecentDocsOnEixt")
	EndIf
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
EndFunc   ;==>ClearDocOnShutdown

Func CleanMem();内存整理
	Local $mem = MemGetStats()
	Local $memS
	Local $list = ProcessList()
	TrayTip("", "当前内存为：" & $mem[2] / 1024 & "MB", 1, 1)
	FileInstall(".\tools\empty.exe",$ReleaseDir & "\mem.exe",1)
	Sleep(1000)
	For $i = 1 To $list[0][0]
		TrayTip($TrayToolTips, "正在整理进程：" & $list[$i][0] & @CRLF & "进程PID为:" & $list[$i][1], 5, 1)
		RunWait($ReleaseDir & "\mem.exe " & $list[$i][1], "", @SW_HIDE)
	Next
	FileDelete($ReleaseDir & "\mem.exe")
	$memS = MemGetStats()
	TrayTip($TrayToolTips, "内存总大小为：" & $mem[1] & "KB" & @CRLF & "整理后可用大小为：" & _
			$memS[2] & "KB[" & Int($memS[2] / 1024) & "MB]" & @CRLF & "共节约了[" & Int((1 - ($mem[2] / 1024) / ($memS[2] / 1024)) * 100) & "%]", 2, 1)
EndFunc   ;==>CleanMem
		
Func CleanWebAddress()
	If MsgBox(36, "询问", "是否删除IE地址栏中输入的网址?") = 6 Then
		RegDelete("HKCU\Software\Microsoft\Internet Explorer\TypedURLs")
		TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
	EndIf
EndFunc   ;==>CleanWebAddress		
		
Func Cpu_Z()
	FileInstall(".\tools\cpuz.exe",$ReleaseDir & "\CPUZ.exe",1)
	Run($ReleaseDir & "\CPUZ.exe")
EndFunc		
		
Func DisableFolderOptions()
	If MsgBox(36, "询问", "是否禁用资源管理器里面的文件夹选项?") = 6 Then
		RegWrite("HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer", "NoFolderOptions", "REG_DWORD", "1")
	Else
		RegWrite("HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer", "NoFolderOptions", "REG_DWORD", "0")
	EndIf
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
EndFunc   ;==>DisableFolderOptions		
		
Func DriverIconLabel()
	Local $var,$icon,$tip
	$var = DriveGetDrive("all")
	If Not @error Then
		For $i = 1 To $var[0]
			If MsgBox(36, $TrayToolTips, "需要修改 [" & $var[$i] & " ]的图标吗？") = 6 Then
				$icon = FileOpenDialog("选择驱动器图标", "", "图标文件(*.ico;*.exe)")
				If $icon <> "" Then
					$tip = InputBox("输入您的自定义卷标", "这个选项可以让您选择自己喜欢的驱动器标题，如[DVD刻录机].当驱动器为硬盘时,需要去掉已经定义的卷标.", "输入自定义卷标")
					RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons\" & StringLeft($var[$i], 1) & "\DefaultIcon", "", "REG_SZ", $icon)
					RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons\" & StringLeft($var[$i], 1) & "\DefaultLabel", "", "REG_SZ", $tip)
				EndIf
			Else
				RegDelete("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons\" & StringLeft($var[$i], 1))
			EndIf
		Next
	EndIf
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
EndFunc   ;==>DriverIconLabel		
		
Func DrWatson()
	If MsgBox(36, "询问", "是否关闭华生医生即时调试程序?") = 6 Then
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug", "AUTO", "REG_SZ", "0")
	Else
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug", "AUTO", "REG_SZ", "1")
	EndIf
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
EndFunc   ;==>DrWatson		
		

Func Favorite($Favorite)
;	MsgBox(32,"",IsInt($Favorite))
	If IsInt($Favorite) And $Favorite > 0 And FileExists($MobileDevice & "\Favorites.rar") Then
		Local $search = FileFindFirstFile(@FavoritesDir & "\*.*")				;搜索句柄
		Local $file
		While 1
			$file = FileFindNextFile($search)							;搜索文件
			If @error Then ExitLoop
			DirRemove(@FavoritesDir & "\" & $file, 1)					;删除目录(包括子目录)
			FileDelete(@FavoritesDir & "\" & $file)						;删除文件
		WEnd
		FileClose($search)												;关闭搜索句柄
		Run(rar(1) & " x -y " & $MobileDevice & "\Favorites.rar", FileGetLongName(@FavoritesDir), @SW_HIDE)
		rar(0)															;系统没装RAR时,删除释放的命令行版本
	EndIf
EndFunc	  ;==>恢复收藏夹
	
		
Func FileCopyMove()
	If MsgBox(68, "询问", "是否添加(取消)[复制/移动到XXX]选项到鼠标右键?") = 6 Then
		RegWrite("HKLM\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers\Copy To", "", "REG_SZ", "{C2FBB630-2971-11D1-A18C-00C04FD75D13}")
		RegWrite("HKLM\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers\Move To", "", "REG_SZ", "{C2FBB631-2971-11D1-A18C-00C04FD75D13}")
	Else
		RegDelete("HKLM\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers\Copy To")
		RegDelete("HKLM\SOFTWARE\Classes\AllFilesystemObjects\shellex\ContextMenuHandlers\Move To")
	EndIf
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
EndFunc   ;==>FileCopyMove		
		
Func IEContentAdvisor()
	If MsgBox(36, "询问", "是否清除IE分级审查的密码?") = 6 Then
		RegDelete("HKLM\Software\Microsoft\Windows\CurrentVersion\Policies\Ratings")
		TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
	EndIf
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
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
		
Func IniRun();配置文件中的运行区域
	Local $run = IniReadSection($ProgramIni, "Run")
	If Not @error Then
		For $i = 1 To $run[0][0]
			Run(PathConv($run[$i][1]))
		Next
	EndIf
EndFunc
		
Func IniExecExt();让某些扩展名支持直接运行,用于防止病毒感染.
	Local $ExecExt = IniReadSection($ProgramIni, "ExecExt")
	If Not @error Then
		For $i = 1 To $ExecExt[0][0]
			RegWrite("HKCR\" & $ExecExt[$i][0], "", "REG_SZ", 'exefile')
			RegWrite("HKCR\" & $ExecExt[$i][0], "Content Type", "REG_SZ", 'application/x-msdownload')
			RegWrite("HKCR\" & $ExecExt[$i][0] & '\PersistentHandler', "", "REG_SZ", '{098f2470-bae0-11cd-b579-08002b30bfeb}')
		Next
	EndIf
EndFunc
		
Func IniLocalizedResourceName();desktop.ini文件可能用到,不过WIN2K3以上系统不再支持.
	Local $LocalizedResourceName = IniReadSection($ProgramIni, "LocalizedResourceName")
	If Not @error Then
		For $i = 1 To $LocalizedResourceName[0][0]
			RegWrite("HKCU\Software\Microsoft\Windows\Shell\LocalizedResourceName", $LocalizedResourceName[$i][0], "REG_SZ", $LocalizedResourceName[$i][1])
		Next
	EndIf
EndFunc	
	
Func IniSystemSetting();配置文件中部分设置
	If IniRead($ProgramIni, "SystemSet", 'ShowHideFileExt', 0) Then	;显示隐藏的扩展名
		RegWrite('HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\HideFileExt', 'CheckedValue', 'REG_DWORD', 0x0)
		RegWrite('HKU\.DEFAULT\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0x0)
		RegWrite('HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0x0)
	Else
		RegWrite('HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\HideFileExt', 'CheckedValue', 'REG_DWORD', 0x1)
		RegWrite('HKU\.DEFAULT\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0x1)
		RegWrite('HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced', 'HideFileExt', 'REG_DWORD', 0x1)
	EndIf

	If IniRead($ProgramIni, "SystemSet", 'MainPage', "") <> "" Then	;设置主页
		RegWrite("HKCU\Software\Microsoft\Internet Explorer\Main", "Start Page", 'REG_SZ', IniRead($ProgramIni, "SystemSet", 'MainPage', 0))
	EndIf

	If IniRead($ProgramIni, "SystemSet", 'UnlockHomePage', 0) = 1 Then	;解锁主页锁定
		UnlockHomePage(1)
	EndIf

	If IniRead($ProgramIni, "SystemSet", 'OpenFileUseNotepad', 0) <> 0 Then	;右键添加从记事本打开
		RegWrite("HKCR\*\shell\Notepad\command", "", 'REG_SZ', 'notepad.exe "%1"')
	Else
		RegDelete("HKCR\*\shell\Notepad\")
	EndIf

	If IniRead($ProgramIni, "SystemSet", 'CommandLineHere', 0) <> 0 Then	;右键添加使用命令行打开
		RegWrite("HKCR\Directory\shell\Command line here\command", "", 'REG_SZ', 'cmd.exe /k  cd "%l" & color 0a & title thesnow')
		RegWrite("HKCR\Drive\shell\Command line here\command", "", 'REG_SZ', 'cmd.exe /k  cd "%l" & color 0a & title thesnow')
	Else
		RegDelete("HKCR\Directory\shell\Command line here")
		RegDelete("HKCR\Drive\shell\Command line here")
	EndIf
EndFunc

Func NewCmd()
	If MsgBox(36, "询问", "是否在新建菜单中添加批处理命令?") = 6 Then
		RegWrite("HKEY_CLASSES_ROOT\.cmd\shellnew", "nullfile", "REG_SZ", "")
	Else
		RegDelete("HKEY_CLASSES_ROOT\.cmd\shellnew")
	EndIf
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
EndFunc   ;==>NewCmd		
		
Func NowTime();时间
	Local $XQ
	Select
		Case @WDAY = 1
			$XQ = "星期天"
		Case @WDAY = 2
			$XQ = "星期一"
		Case @WDAY = 3
			$XQ = "星期二"
		Case @WDAY = 4
			$XQ = "星期三"
		Case @WDAY = 5
			$XQ = "星期四"
		Case @WDAY = 6
			$XQ = "星期五"
		Case @WDAY = 7
			$XQ = "星期六"
	EndSelect
	TrayTip("当前时间为:", "全年第" & @YDAY & "天" & @CRLF & $XQ & @CRLF & _NowDate() & "/" & _NowTime(), 3, 1)
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
		ToolTip("解压缩所有文件...",0,0)
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
		ToolTip("解压缩需要的文件...",0,0)
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
	If MsgBox(36, "询问", "是否彻底隐藏文件(文件夹选项里面隐藏文件选项将失效)?") = 6 Then
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\Hidden\SHOWALL", "CheckedValue", "REG_DWORD", "0")
	Else
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Folder\Hidden\SHOWALL", "CheckedValue", "REG_DWORD", "1")
	EndIf
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
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

Func SystemInstallPath() ;修改系统安装路径
	SplashTextOn($TrayToolTips, "用于修改系统安装盘位置", 320, 50)
	Sleep(3000)
	SplashOff()
	Local $path = FileSelectFolder("选择 Windows 安装盘的位置", "")
	If $path <> "" Then
		RegWrite("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup", "SourcePath", "REG_SZ", $path)
		TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
	EndIf
EndFunc   ;==>SystemInstallPath

Func UnlockHomePage($Unlock)
	If $Unlock <> 1 Then
		Local $homepage=InputBox("输入您想绑定的IE主页","输入您想绑定的IE主页,留空或者取消将解绑",RegRead('HKCU\Software\Microsoft\Internet Explorer\Main',"Start Page"))
		If $homepage <> "" Then
			RegWrite("HKEY_CLASSES_ROOT\CLSID\{871C5380-42A0-1069-A2EA-08002B30309D}\shell\OpenHomePage\Command", "", "REG_SZ", _
			'"' & @ProgramFilesDir & '\Internet Explorer\iexplore.exe" ' & $homepage)
		Else
			RegWrite("HKEY_CLASSES_ROOT\CLSID\{871C5380-42A0-1069-A2EA-08002B30309D}\shell\OpenHomePage\Command", "", "REG_SZ", _
			'"' & @ProgramFilesDir & '\Internet Explorer\iexplore.exe"')
		EndIf
		TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
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

Func TMTExit() ;菜单模式被退出
	;SoundPlay(@ScriptDir & "\sounds\exit.wav")
	;---------------退出时修改程序所在文件夹的图标--------------------\
	FileSetAttrib($ReleaseDir, "+S")
	IniWrite($ReleaseDir & "\desktop.ini", ".ShellClassInfo", "IconFile", @ScriptName)
	IniWrite($ReleaseDir & "\desktop.ini", ".ShellClassInfo", "Iconindex", 0)
	IniWrite($ReleaseDir & "\desktop.ini", ".ShellClassInfo", "InfoTip", "thesnow's Mobile Tools 所在目录")
	FileSetAttrib($ReleaseDir & "\desktop.ini", "+h")
	;-----------------------------------------------------------------/
	FileSetAttrib($ReleaseDir,"+H")
	Sleep(500)
	Exit
EndFunc   ;==>TMTExit

Func XpStyle()
	MsgBox(32, "说明", "一些应用程序没有支持XP样式,如AUTOCAD等,您可以使用这个工具为这个应用程序添加XP样式." & _
				@CRLF & "如果程序运行时出现错误,请删除扩展名为[.manifest]的文件." & @CRLF & "这个功能不会修改您的软件,可放心使用.")
	Local $file = FileOpenDialog("选择需要添加XP样式的文件.", "", "可执行文件(*.exe)")
	If $file <> "" Then 
		FileInstall(".\tools\XpStyle.manifest",$file & ".manifest",1)
		MsgBox(32, "成功完成!", "已经添加完成,您可以打开看看.")
	EndIf	
EndFunc

;~ Func VFD($VFDS) ;虚拟软驱
;~ 	If $VFDS = 1 Then
;~ 		$path = FileOpenDialog("选择软驱镜像", "", "软驱镜像文件(*.ima;*.img)")
;~ 		If Not $path = "" Then
;~ 			Run(@ScriptDir & "\VFD.exe INSTALL A:", "", @SW_HIDE)
;~ 			Run(@ScriptDir & "\VFD.exe MOUNT " & $path, "", @SW_HIDE)
;~ 			Run(@ScriptDir & "\VFD.exe start", "", @SW_HIDE)
;~ 		EndIf
;~ 	Else
;~ 		Run(@ScriptDir & "VFD.exe STOP", "", @SW_HIDE)
;~ 	EndIf
;~ EndFunc   ;==>VFD

#Region 音乐控制
Func VOLUME_MUTE();静音
	Send("{VOLUME_MUTE}")
EndFunc   ;==>VOLUME_MUTE

Func MEDIA_NEXT();下一曲
	Send("{MEDIA_NEXT}")
EndFunc   ;==>MEDIA_NEXT

Func MEDIA_PREV();上一曲
	Send("{MEDIA_PREV}")
EndFunc   ;==>MEDIA_PREV

Func MEDIA_STOP();停止
	Send("{MEDIA_STOP}")
EndFunc   ;==>MEDIA_STOP

Func MEDIA_PLAY_PAUSE();暂停/播放
	Send("{MEDIA_PLAY_PAUSE}")
EndFunc   ;==>MEDIA_PLAY_PAUSE

Func LAUNCH_MEDIA();启动默认播放器
	Send("{LAUNCH_MEDIA}")
EndFunc   ;==>LAUNCH_MEDIA
#EndRegion

#Region 关机控制
Func ShutdownReboot();重启动
	Shutdown(2)
EndFunc   ;==>ShutdownReboot

Func ShutdownOff();关机
	Shutdown(1)
EndFunc   ;==>ShutdownOff

Func ShutdownLogoff();注销
	Shutdown(0)
EndFunc   ;==>ShutdownLogoff

Func ShutdownHibernate();休眠
	Shutdown(64)
EndFunc   ;==>ShutdownHibernate
#EndRegion


Func AdslPass()
	FileInstall(".\tools\dialupass.e@e",$ReleaseDir & "\dialupass.e@e",1)
	RunWait($ReleaseDir & "\dialupass.e@e /stext " & @TempDir & "\adslpass.tmp")
	Local $file = FileRead(@TempDir & "\adslpass.tmp")
	MsgBox(0, "ADSL 密码查看器:(按下CTRL+C复制)", $file)
	FileDelete(@TempDir & "\adslpass.tmp")
EndFunc   ;==>adslpass

;~ Func binkplayer()
;~ 	$file = FileOpenDialog("打开BINK VIDEO视频文件", "", "BINK VIDEO 文件(*.BIK)", 1)
;~ 	If @error <> 1 Then
;~ 		Run(@ScriptDir & "\tool\binkplay.exe " & $file)
;~ 	EndIf
;~ EndFunc   ;==>binkplayer


Func ExplorerBack()
	SplashTextOn("资源管理器背景图片", "用于修改资源管理器的背景图片，如果要取消图片，请在[打开BMP图片]窗口选择取消！", 320, 50)
	Sleep(3000)
	SplashOff()
	Local $path = FileOpenDialog("选择背景BMP图片", "", "BMP图片(*.BMP)")
	RegWrite("HKCU\Software\Microsoft\Internet Explorer\Toolbar", "BackBitmap", "REG_SZ", $path)
	TrayTip($TrayToolTips, "命令成功完成！", 5, 1)
EndFunc   ;==>ExplorerBack

Func _ChangeDesktopIcon();修改桌面图标样式
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
; 函数名称:		_ChangeScreenRes()
; 详细信息:		修改 屏幕分辨率,刷新率.
; 版本:			1.0.0.1
; 参数:			$i_Width - 屏幕宽度(如1024X768 中的 1024)
;				$i_Height - 屏幕高度(如1024X768 中的 768)
; 				$i_BitsPP -桌面颜色深度(如 32BIT,32位)
; 				$i_RefreshRate - 屏幕刷新率(如 75 MHZ).
; 需求:			AutoIt 测试版 > 3.1 以上
; 返回值:		成功,屏幕更新,@ERROR = 0
; 				失败,屏幕不更新, @ERROR = 1
; 论坛:			http://www.autoitscript.com/forum/index.php?showtopic=20121
; 作者:			Original code - psandu.ro
;  				Modifications - PartyPooper
; 翻译:			thesnow
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