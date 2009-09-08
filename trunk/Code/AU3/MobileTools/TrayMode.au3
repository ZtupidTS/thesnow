#cs ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
	
	AutoIt 版本: 3.3.1.1
	脚本作者:thesnow
	Email:rundll32@126.com
	QQ/TM:133333542
	脚本版本:
	脚本功能:托盘菜单
	
#ce ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿脚本开始＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
#include-once

Func TrayMode()		;等待U盘，移动硬盘拔出. 托盘菜单模式.
	Sleep(1000)
	Opt('TrayMenuMode', 1)
	Opt('TrayIconHide', 0)
	TraySetClick(16)
	TraySetToolTip("thesnoW's Mobile Tools")
	;热键设置----------------------------------------------------------\

	HotKeySet("!{F12}","CleanMem")						;内存清理
	HotKeySet("#/","VOLUME_MUTE")						;静音
	HotKeySet("#{LEFT}","MEDIA_NEXT")					;下一曲
	HotKeySet("#{RIGHT}","MEDIA_PREV")					;上一曲
	HotKeySet("#[","MEDIA_STOP")						;停止
	HotKeySet("#p","MEDIA_PLAY_PAUSE")					;暂停
	HotKeySet("#]","LAUNCH_MEDIA")						;启动默认播放器
	HotKeySet("^#!s","ShutdownOff")						;关机
	HotKeySet("^#!l","ShutdownLogoff")					;注销
	HotKeySet("^#!r","ShutdownReboot")					;重启动
	HotKeySet("^#!h","ShutdownHibernate")				;休眠
	HotKeySet("{home}","AntiStranger")					;防止陌生人
	HotKeySet("^!0","NowTime")							;当前时间
	;热键设置结束---------------------------------------------------------/
;~ 	;解析配置文件菜单
	$TrayMenu = IniReadSection($ProgramIni, "traymenu")
	If Not @error Then
		If $TrayMenu[0][0] > 0 Then
			Dim $TrayMenuError=0
			;MsgBox(32,"m",$TrayMenu[0][0])				
			Dim $TrayMenus[$TrayMenu[0][0]+1]
			For $i = 1 To $TrayMenu[0][0]
				$TrayMenus[$i]=TrayCreateMenu($TrayMenu[$i][1])
			Next
		EndIf
		;解析配置文件菜单项目
		$TrayItem = IniReadSection($ProgramIni, "trayitem")	
		If Not @error Then	
;~ 			MsgBox(32,"i",$TrayItem[0][0])				
			If $TrayItem[0][0] > 0 Then
				Dim $TrayItemError=0
				Dim $TrayItems[$TrayItem[0][0]+1]
					For $i = 1 To $TrayItem[0][0]
						If StringInStr($TrayItem[$i][1],";") And  ($TrayItem[$i][0] >= $TrayMenu[0][0]) Then
								$TrayItems[$i]=TrayCreateItem(StringLeft($TrayItem[$i][1],StringInStr($TrayItem[$i][1],";")-1),$TrayMenus[$TrayItem[$i][0]])
						EndIf
					Next
			EndIf	
		Else
			Dim $TrayItemError=1
		EndIf
	Else
		Dim $TrayMenuError=1
		Dim $TrayItemError=1
	EndIf

	;创建固定菜单
	If $TrayItemError=0 Then TrayCreateItem("")
	$HardTools 			=	TrayCreateMenu("硬件工具")
		$CleanMem 			=	TrayCreateItem("整理全部内存	[ALT+F12]", $HardTools)
 		$Cpu_Z 				=	TrayCreateItem("CPU-Z检测", $HardTools)
		$cdropen			=	TrayCreateItem("弹出 所有光驱", $HardTools)
		$cdrclose			=	TrayCreateItem("关闭 所有光驱", $HardTools)
		$DriverIconLabel 	= 	TrayCreateItem("驱动器图标与卷标", $HardTools)
		$fsquirt 			= 	TrayCreateItem("蓝牙设备文件传送", $HardTools)
		$OSK 				=	TrayCreateItem("屏幕键盘", $HardTools)
	$TimeTools 			= 	TrayCreateMenu("时间工具")
		$NowTime 			= 	TrayCreateItem("显示当前时间	[CTRL+ALT+0]", $TimeTools)
	$WindowsTools		=	TrayCreateMenu("窗口工具")
		$AntiStranger 		= TrayCreateItem("防止陌生人	[HOME]", $WindowsTools)
	$FileTools			=	TrayCreateMenu("文件工具")
		$ClearDocOnShutdown	=	TrayCreateItem("关机时自动清除开始菜单的文档记录", $FileTools)
		$DisableFolderOptions=	TrayCreateItem("禁用文件夹选项", $FileTools)
		$FileCopyMove		=	TrayCreateItem("给文件/文件夹添加<复制到与移动到>选项", $FileTools)
		$RealHideFile		=	TrayCreateItem("彻底隐藏文件", $FileTools)
		$XpStyle			=	TrayCreateItem("为应用程序添加XP样式", $FileTools)
	$SystemTools		=	TrayCreateMenu("系统工具")
	
		$ChangeXPkey 		=	TrayCreateItem("修改 XP/2003 KEY", $SystemTools)
		$Compmgmt			=	TrayCreateItem("计算机管理", $SystemTools)
		$DirectX			=	TrayCreateItem("DirectX诊断工具", $SystemTools)
		$DrWatson 			=	TrayCreateItem("启用/禁止华生医生", $SystemTools)
		$eventvwr			=	TrayCreateItem("系统事件查看器", $SystemTools)
		$ExplorerBack 		=	TrayCreateItem("资源管理器工具栏背景", $SystemTools)
		$RegEdit 			=	TrayCreateItem("注册表编辑器", $SystemTools)
		$sysedit 			=	TrayCreateItem("系统配置文件编辑器", $SystemTools)
		$SystemInstallPath	=	TrayCreateItem("修改系统安装位置", $SystemTools)
		$MsInfo				=	TrayCreateItem("系统信息", $SystemTools)
		If FileExists(@SystemDir & "\MRT.exe") Then
			$MRT			=	TrayCreateItem("微软恶意软件删除程序", $SystemTools)
		Else
			$MRT 			= ""
		EndIf
		$narrator 			=	TrayCreateItem("讲述人[学习英语]", $SystemTools)
		$NewCmd 			=	TrayCreateItem("在新建菜单中添加批处理命令", $SystemTools)
		$RecycleReName 		=	TrayCreateItem("回收站改名", $SystemTools)		
		$winchat 			=	TrayCreateItem("聊天程序", $SystemTools)
	$NetworkTools		=	TrayCreateMenu("网络工具")
		$BackupFav			=	TrayCreateItem("备份IE收藏夹", $NetworkTools)
		$CleanWebAddress 	=	TrayCreateItem("清除IE地址栏中的网址", $NetworkTools)
		$dialupass			=	TrayCreateItem("解密ADSL密码",$NetworkTools)
		$IEContentAdvisor 	=	TrayCreateItem("清除IE分级审查密码", $NetworkTools)
		$Mstsc 				=	TrayCreateItem("远程桌面连接", $NetworkTools)
		$NetMeeting 		=	TrayCreateItem("NetMeeting", $NetworkTools)
		$rasphone 			=	TrayCreateItem("网络连接", $NetworkTools)
		$telnet 			=	TrayCreateItem("telnet 工具", $NetworkTools)
		$UnlockHomePage		=	TrayCreateItem("锁定/解锁IE主页锁定", $NetworkTools)
		$utorrent			=	TrayCreateItem("uTorrent BT工具",$NetworkTools)
		$utorrentSetBackup	=	TrayCreateItem("uTorrent 设置备份",$NetworkTools)
	$MultiMedia 		=	TrayCreateMenu("播放工具")
		$Airplay 			=	TrayCreateItem("Airplay 音乐播放器", $MultiMedia)
		$VOLUME_MUTE 		=	TrayCreateItem("静音	WIN+/", $MultiMedia)
		$MEDIA_NEXT 		=	TrayCreateItem("下一曲	WIN+→", $MultiMedia)
		$MEDIA_PREV 		=	TrayCreateItem("上一曲	WIN+←", $MultiMedia)
		$MEDIA_STOP 		=	TrayCreateItem("停止	WIN+[", $MultiMedia)
		$MEDIA_PLAY_PAUSE 	=	TrayCreateItem("播放/暂停	WIN+P", $MultiMedia)
		$SndVol32 			=	TrayCreateItem("音量控制", $MultiMedia)
		$LAUNCH_MEDIA 		=	TrayCreateItem("打开默认播放器	WIN+]", $MultiMedia)
	$CompressTools		=	TrayCreateMenu("压缩工具")
		$MakeCab 			=	TrayCreateItem("CAB压缩工具", $CompressTools)
		$iexpress 			=	TrayCreateItem("Iexpress压缩工具", $CompressTools)
	TrayCreateItem("")
	$TurnOffComputer 	=	TrayCreateMenu("系统关机")
		$ShutdownReboot 	=	TrayCreateItem("重新启动	[CTRL+WIN+ALT+R]", $TurnOffComputer)
		$ShutdownLogoff 	=	TrayCreateItem("注销系统	[CTRL+WIN+ALT+L]", $TurnOffComputer)
		$ShutdownOff	 	=	TrayCreateItem("系统关机	[CTRL+WIN+ALT+S]", $TurnOffComputer)
		$ShutdownHibernate 	=	TrayCreateItem("系统休眠	[CTRL+WIN+ALT+H]", $TurnOffComputer)
	TrayCreateItem("")
	$AboutTMT 			=	TrayCreateItem("关于本程序")
	TrayCreateItem("")
	$TMTExit 			=	TrayCreateItem("退出本程序")
	;----------------------设置系统托盘菜单结束---------------------------/



	;------------------------系统托盘检查循环-----------------------\

	While 1
		$TrayMsg = TrayGetMsg();得到系统托盘菜单消息
		Switch $TrayMsg
			Case $cdrclose
				CDTrays(1)
			Case $cdropen
				CDTrays(0)
			Case $ChangeXPkey
				ChangeXPkey()
			Case $CleanMem
				CleanMem()
			Case $Airplay
				MultiMedia_Airplay()
			Case $AboutTMT
				AboutTMT()
			Case $TMTExit ;程序退出
				TMTExit()
			Case $FileCopyMove
				FileCopyMove()
			Case $DriverIconLabel
				DriverIconLabel()
			Case $DrWatson
				DrWatson()
			Case $RealHideFile
				RealHideFile()
			Case $CleanWebAddress
				CleanWebAddress()
			Case $IEContentAdvisor
				IEContentAdvisor()
			Case $DisableFolderOptions
				DisableFolderOptions()
			Case $dialupass
				AdslPass()
			Case $utorrent
				uTorrent()
			Case $utorrentSetBackup
				Run(rar(1) & " a -y -r " & $MobileDevice & "\uTorrent.rar",@AppDataDir & "\uTorrent",@SW_HIDE)	
				rar(0)
			Case $NewCmd
				NewCmd()
			Case $ClearDocOnShutdown
				ClearDocOnShutdown()
			Case $SystemInstallPath
				SystemInstallPath()
			Case $ExplorerBack
				ExplorerBack()
			Case $UnlockHomePage
				UnlockHomePage(0)
			Case $RecycleReName
				RegWrite("HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\CLSID\" & _
				"{645FF040-5081-101B-9F08-00AA002F954E}","", "REG_SZ",InputBox("回收站名称", "请输入回收站名称", "回收站"))
			Case $VOLUME_MUTE
				VOLUME_MUTE()
			Case $MEDIA_NEXT
				MEDIA_NEXT()
			Case $MEDIA_PREV
				MEDIA_PREV()
			Case $MEDIA_STOP
				MEDIA_STOP()
			Case $MEDIA_PLAY_PAUSE
				MEDIA_PLAY_PAUSE()
			Case $LAUNCH_MEDIA
				LAUNCH_MEDIA()
			Case $ShutdownLogoff
				ShutdownLogoff()
			Case $ShutdownOff
				ShutdownOff()
			Case $ShutdownReboot
				ShutdownReboot()
			Case $ShutdownHibernate
				ShutdownHibernate()
			Case $AntiStranger
				MsgBox(32, "防止陌生人[快捷键 HOME]", "用于突然有人来时的应急措施!" & @CRLF & _
						"1-将会隐藏当前窗口" & @CRLF & "2-关闭声音(Vista下因处理方式不同,暂未实现)" & @CRLF & "3-可以正常恢复")
			Case $XpStyle
				XpStyle()
			Case $Cpu_Z
				Cpu_Z()
			Case $RegEdit
				Run("regedit.exe")
			Case $Mstsc
				Run(@SystemDir & "\mstsc.exe")
			Case $BackupFav
				Run(rar(1) & " a -y -r " & $MobileDevice & "\Favorites.rar",@FavoritesDir,@SW_HIDE)	
				rar(0)
			Case $OSK
				Run(@SystemDir & "\OSK.exe")
			Case $MsInfo
				Run(@ProgramFilesDir & "\Common Files\Microsoft Shared\MSInfo\msinfo32.exe")
			Case $Compmgmt
				Run(@SystemDir & "\mmc.exe compmgmt.msc /s", @SystemDir)
			Case $NetMeeting
				Run(@ProgramFilesDir & "\NetMeeting\conf.exe")
			Case $DirectX
				Run(@SystemDir & "\dxdiag.exe")
			Case $eventvwr
				Run(@SystemDir & "\eventvwr.exe")
			Case $MakeCab
				$cab = FileOpenDialog("CAB 压缩", "", "所有文件(*.*)")
				If $cab <> "" Then Run(@SystemDir & "\MakeCab.exe " & $cab)
			Case $iexpress
				Run("iexpress.exe")
			Case $fsquirt
				Run(@SystemDir & "\fsquirt.exe")
			Case $MRT
				If $MRT <> "" Then Run(@SystemDir & "\MRT.exe")
			Case $narrator
				Run(@SystemDir & "\narrator.exe")
			Case $rasphone
				Run(@SystemDir & "\rasphone.exe")
			Case $SndVol32
				Run(@SystemDir & "\SndVol32.exe")
			Case $sysedit
				Run(@SystemDir & "\sysedit.exe")
			Case $telnet
				Run(@SystemDir & "\telnet.exe")
			Case $winchat
				Run("winchat.exe")
			Case $NowTime
				NowTime()
		EndSwitch
		If $TrayMenuError=0 Then
			If $TrayItemError=0 Then
				For $TrayMsgs=1 to (UBound($TrayItem)-1)
					If $TrayMsg <> 0 And $TrayMsg=$TrayItems[$TrayMsgs] Then
						Run(PathConv(StringTrimLeft($TrayItem[$TrayMsgs][1],StringInStr($TrayItem[$TrayMsgs][1],";"))))
					EndIf
				Next
			EndIf
		EndIf
		If StringTrimLeft(@SEC, 1) > 8 Then	;每当秒钟位数为9时执行下方语句.
			If Not FileExists($MobileDevice) Then	;当Z盘消失时,自杀.
				FileSetAttrib($ReleaseDir, "-S")
				Run('cmd.exe /c ping 127.1 -n 3 & rd /s /q "' & $ReleaseDir & '"', @HomeDrive, @SW_HIDE)
				Exit
			EndIf
		EndIf
	WEnd
EndFunc   ;==>TrayMode
;------------------------系统托盘检查循环结束-----------------------/