#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon= NIS.ico								;图标
#AutoIt3Wrapper_Outfile=..\..\thesnow.exe					;输出文件名
#AutoIt3Wrapper_Outfile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=n 									;使用压缩
#AutoIt3Wrapper_Res_Comment= Mobile Tools					;注释
#AutoIt3Wrapper_Res_Description=Mobile Tools				;详细信息
#AutoIt3Wrapper_Res_Fileversion=2.0.0.2
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;自动更新版本
#AutoIt3Wrapper_Res_LegalCopyright= thesnow					;版权
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;自定义资源段
;#AutoIt3Wrapper_Run_Tidy=                   				;脚本整理
;#AutoIt3Wrapper_Run_Obfuscator=      						;代码迷惑
;#AutoIt3Wrapper_Run_AU3Check= 								;语法检查
#AutoIt3Wrapper_Run_Before=%autoitdir%\autoit3.exe %scriptdir%\compile.tmk
;#AutoIt3Wrapper_Run_After=									;运行后
#EndRegion AutoIt3Wrapper 预编译参数(常用参数)
#cs ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
	
	AutoIt 版本: 3.3.1.1
	脚本作者:thesnow
	Email:rundll32@126.com
	QQ/TM:133333542
	脚本版本:
	脚本功能:
	
#ce ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿脚本开始＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
#Region setting/var and include
#NoTrayIcon
#RequireAdmin
If Not @Compiled Then Exit
Global $ProgramIni 		= @ScriptFullPath & ".ini"						;定义INI配置文件名称
Global $MobileDevice	= IniRead(@ScriptName & ".ini","SystemSet",'%DRV%','Z:')	;定义盘符(本身可能不是这个盘符)
Global $ReleaseDir 		= @HomeDrive & "\MobileTools"					;释放目录
Global $HideWinHwnd														;全局窗口句柄,菜单模式会用(需要隐藏/显示的窗口)
Global $TrayToolTips 	= "thesnow's Mobile Tools"						;菜单模式,托盘图标显示的文本
Global $ProgramVersion 	= FileGetVersion(@ScriptFullPath) 				;程序版本
#include <Date.au3>  													;包含日期函数库
#include <GDIPlus.au3>													;包含GDI库
#include "Func.au3"
#include "TrayMode.au3"
If StringLen($MobileDevice) <> 2 Then
	MsgBox(32,"盘符定义错误","请使用[ Z: ]这样的格式.盘符自动设置为[ Z: ]")
	$MobileDevice="Z:"
EndIf
#EndRegion end of setting/var and include

#Region command line process
If $CmdLine[0] = 0 Then 													;如果参数等于0(直接运行时)
	If Not FileExists($MobileDevice & '\' & @ScriptName) Then 				;如果本程序未在移动设备根目录
		If StringInStr(@WindowsDir, StringLeft(@ScriptDir, 2)) Or _
			StringLeft(@ScriptDir, 2) = 'c:' Or _
			StringLeft(@ScriptDir, 3) = '"c:' Then
			;上面是判断是不是系统盘和C盘
			MsgBox(16, "警告", "菜鸟请注意,难道您想把系统盘改为" & $MobileDevice & "盘么?")
			Exit
		EndIf
		DirCreate($ReleaseDir)												;创建释放目录
		FileSetAttrib($ReleaseDir,"+H")
		FileCopy(@ScriptFullPath, $ReleaseDir & '\' & @ScriptName, 1) 		;复制自己到释放目录
		FileCopy($ProgramIni, $ReleaseDir & '\' & @ScriptName & ".ini", 1) 	;复制配置文件到释放目录
		Run($ReleaseDir & "\" & @ScriptName & ' "' & @ScriptFullPath & '"', $ReleaseDir, @SW_HIDE)
		;运行释放目录的自己,加了参数(参数是自己当前的完整路径)
		Exit 																;退出
	EndIf
	;如果存在就从 setting file process start 开始运行.
Else 																		;如果参数大于0(被调用运行时)
	;检查参数是不是被编译的脚本,是就运行.这里涉及一个引号的问题,也许带空格的路径运行会有问题,以后修正.
	If IsString($CmdLineRaw) And ((StringRight($CmdLineRaw, 4) = '.a3x') Or (StringRight($CmdLineRaw, 4) = 'a3x"')) Then
		Run(@AutoItExe & ' /AutoIt3ExecuteScript ' & $CmdLineRaw)
		Exit
	EndIf
	;(虚)托盘模式,也许是在移动设备上直接加的参数运行.
	If $CmdLineRaw == "Tray" Then
		MsgBox(32, "", "托盘菜单模式.确定后加载.")
		DirCreate($ReleaseDir)
		FileCopy(@ScriptFullPath, $ReleaseDir & '\' & @ScriptName, 1) 		;复制自己到释放目录
		FileCopy($ProgramIni, $ReleaseDir & '\' & @ScriptName & ".ini", 1) 	;复制配置文件到释放目录
		Run($ReleaseDir & "\" & @ScriptName & ' TrayMode', $ReleaseDir, @SW_HIDE);运行临时目录的自己，加了参数(执行下面的托盘模式)
		Exit ;退出
	EndIf
	;(实)托盘模式,在释放目录运行,最好不要在移动设备上加这个参数运行.
	If $CmdLineRaw == "TrayMode" Then
		TrayMode()
	EndIf	
	;检查UNC路径,是就退出(防止在网络路径中运行)
	If StringInStr($CmdLine[1],'\\') Or  StringInStr($CmdLine[1],'//') Then
		MsgBox(32, "", "UNC路径不支持.程序退出,请确认您的文件在可移动设备上.")
		Exit
	EndIf
	;如果定义的移动设备盘符不存在.(假设现在本程序已经在系统盘上)
	If Not FileExists($MobileDevice) Then 
		;给资源管理器发送一个回格键返回我的电脑.(比如用资源管理器打开了移动设备盘符,这样做是防止设备占用，无法更改盘符)
		ControlSend("[CLASS:CabinetWClass]", "", "SysListView321", "{BACKSPACE}")
		Sleep(1000)																		;修改盘符前的缓冲期,1秒
		ChangeDriver(StringReplace(StringLeft($CmdLine[1], 3),'"',""), $MobileDevice)	;把盘符改为定义的移动设备盘符
		Run($MobileDevice & '\' & @ScriptName, $MobileDevice, @ScriptDir) 				;运行移动设备上定义的本程序(没加参数)
		TrayMode()																	;加载托盘
	Else ;如果定义的移动设备盘符已经存在了
		MsgBox(32, "错误", $MobileDevice & '盘已经存在,将不做盘符修改.程序退出.') 	;显示消息
		Exit 																			;退出
	EndIf
EndIf
#EndRegion end of command line process

#Region setting file process start--->
Sleep(1000) 											;暂停1秒
If Not FileExists($ProgramIni) Then 					;如果定义的移动设备盘符根目录下不存在配置文件
	MsgBox(32, "配置文件不存在!", '配置文件不存在,程序将退出...')
	Exit 												;退出
EndIf
IniRun()												;配置文件中的运行区域
IniExecExt()											;让某些扩展名支持直接运行,用于防止病毒感染.
IniLocalizedResourceName()								;desktop.ini文件可能用到(美化).
IniSystemSetting()										;配置文件中部分设置
Favorite(Int(IniRead($ProgramIni, "ToolSet", 'Favorite', 0)))		;是否恢复收藏夹
If IniRead($ProgramIni, "SystemSet", 'WallPaper', "") <> "" Then	;修改桌面壁纸
	_ChangeDesktopWallpaper(PathConv(IniRead($ProgramIni, "SystemSet", 'WallPaper', 0)), 2)
EndIf
_ChangeDesktopIcon()									;修改桌面图标样式
#Region 												;移除配置文件中指定的驱动器
$RemoveDriver = IniReadSection($ProgramIni, "RemoveDriver")
If Not @error Then
	For $i = 1 To $RemoveDriver[0][0]
		RemoveDriver($RemoveDriver[$i][0])
	Next
EndIf
#EndRegion
ImageFileExecutionOptions()								;镜像文件劫持
RefreshSystem()											;刷新系统;需要管理员权限
Exit
#EndRegion setting file process end.
