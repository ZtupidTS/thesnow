#cs �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
	
	AutoIt �汾: 3.3.1.1
	�ű�����:thesnow
	Email:rundll32@126.com
	QQ/TM:133333542
	�ű��汾:
	�ű�����:���̲˵�
	
#ce �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣ߽ű���ʼ�ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
#include-once

Func TrayMode()		;�ȴ�U�̣��ƶ�Ӳ�̰γ�. ���̲˵�ģʽ.
	Sleep(1000)
	Opt('TrayMenuMode', 1)
	Opt('TrayIconHide', 0)
	TraySetClick(16)
	TraySetToolTip("thesnoW's Mobile Tools")
	;�ȼ�����----------------------------------------------------------\

	HotKeySet("!{F12}","CleanMem")						;�ڴ�����
	HotKeySet("#/","VOLUME_MUTE")						;����
	HotKeySet("#{LEFT}","MEDIA_NEXT")					;��һ��
	HotKeySet("#{RIGHT}","MEDIA_PREV")					;��һ��
	HotKeySet("#[","MEDIA_STOP")						;ֹͣ
	HotKeySet("#p","MEDIA_PLAY_PAUSE")					;��ͣ
	HotKeySet("#]","LAUNCH_MEDIA")						;����Ĭ�ϲ�����
	HotKeySet("^#!s","ShutdownOff")						;�ػ�
	HotKeySet("^#!l","ShutdownLogoff")					;ע��
	HotKeySet("^#!r","ShutdownReboot")					;������
	HotKeySet("^#!h","ShutdownHibernate")				;����
	HotKeySet("{home}","AntiStranger")					;��ֹİ����
	HotKeySet("^!0","NowTime")							;��ǰʱ��
	;�ȼ����ý���---------------------------------------------------------/
;~ 	;���������ļ��˵�
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
		;���������ļ��˵���Ŀ
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

	;�����̶��˵�
	If $TrayItemError=0 Then TrayCreateItem("")
	$HardTools 			=	TrayCreateMenu("Ӳ������")
		$CleanMem 			=	TrayCreateItem("����ȫ���ڴ�	[ALT+F12]", $HardTools)
 		$Cpu_Z 				=	TrayCreateItem("CPU-Z���", $HardTools)
		$cdropen			=	TrayCreateItem("���� ���й���", $HardTools)
		$cdrclose			=	TrayCreateItem("�ر� ���й���", $HardTools)
		$DriverIconLabel 	= 	TrayCreateItem("������ͼ������", $HardTools)
		$fsquirt 			= 	TrayCreateItem("�����豸�ļ�����", $HardTools)
		$OSK 				=	TrayCreateItem("��Ļ����", $HardTools)
	$TimeTools 			= 	TrayCreateMenu("ʱ�乤��")
		$NowTime 			= 	TrayCreateItem("��ʾ��ǰʱ��	[CTRL+ALT+0]", $TimeTools)
	$WindowsTools		=	TrayCreateMenu("���ڹ���")
		$AntiStranger 		= TrayCreateItem("��ֹİ����	[HOME]", $WindowsTools)
	$FileTools			=	TrayCreateMenu("�ļ�����")
		$ClearDocOnShutdown	=	TrayCreateItem("�ػ�ʱ�Զ������ʼ�˵����ĵ���¼", $FileTools)
		$DisableFolderOptions=	TrayCreateItem("�����ļ���ѡ��", $FileTools)
		$FileCopyMove		=	TrayCreateItem("���ļ�/�ļ������<���Ƶ����ƶ���>ѡ��", $FileTools)
		$RealHideFile		=	TrayCreateItem("���������ļ�", $FileTools)
		$XpStyle			=	TrayCreateItem("ΪӦ�ó������XP��ʽ", $FileTools)
	$SystemTools		=	TrayCreateMenu("ϵͳ����")
	
		$ChangeXPkey 		=	TrayCreateItem("�޸� XP/2003 KEY", $SystemTools)
		$Compmgmt			=	TrayCreateItem("���������", $SystemTools)
		$DirectX			=	TrayCreateItem("DirectX��Ϲ���", $SystemTools)
		$DrWatson 			=	TrayCreateItem("����/��ֹ����ҽ��", $SystemTools)
		$eventvwr			=	TrayCreateItem("ϵͳ�¼��鿴��", $SystemTools)
		$ExplorerBack 		=	TrayCreateItem("��Դ����������������", $SystemTools)
		$RegEdit 			=	TrayCreateItem("ע���༭��", $SystemTools)
		$sysedit 			=	TrayCreateItem("ϵͳ�����ļ��༭��", $SystemTools)
		$SystemInstallPath	=	TrayCreateItem("�޸�ϵͳ��װλ��", $SystemTools)
		$MsInfo				=	TrayCreateItem("ϵͳ��Ϣ", $SystemTools)
		If FileExists(@SystemDir & "\MRT.exe") Then
			$MRT			=	TrayCreateItem("΢��������ɾ������", $SystemTools)
		Else
			$MRT 			= ""
		EndIf
		$narrator 			=	TrayCreateItem("������[ѧϰӢ��]", $SystemTools)
		$NewCmd 			=	TrayCreateItem("���½��˵����������������", $SystemTools)
		$RecycleReName 		=	TrayCreateItem("����վ����", $SystemTools)		
		$winchat 			=	TrayCreateItem("�������", $SystemTools)
	$NetworkTools		=	TrayCreateMenu("���繤��")
		$BackupFav			=	TrayCreateItem("����IE�ղؼ�", $NetworkTools)
		$CleanWebAddress 	=	TrayCreateItem("���IE��ַ���е���ַ", $NetworkTools)
		$dialupass			=	TrayCreateItem("����ADSL����",$NetworkTools)
		$IEContentAdvisor 	=	TrayCreateItem("���IE�ּ��������", $NetworkTools)
		$Mstsc 				=	TrayCreateItem("Զ����������", $NetworkTools)
		$NetMeeting 		=	TrayCreateItem("NetMeeting", $NetworkTools)
		$rasphone 			=	TrayCreateItem("��������", $NetworkTools)
		$telnet 			=	TrayCreateItem("telnet ����", $NetworkTools)
		$UnlockHomePage		=	TrayCreateItem("����/����IE��ҳ����", $NetworkTools)
		$utorrent			=	TrayCreateItem("uTorrent BT����",$NetworkTools)
		$utorrentSetBackup	=	TrayCreateItem("uTorrent ���ñ���",$NetworkTools)
	$MultiMedia 		=	TrayCreateMenu("���Ź���")
		$Airplay 			=	TrayCreateItem("Airplay ���ֲ�����", $MultiMedia)
		$VOLUME_MUTE 		=	TrayCreateItem("����	WIN+/", $MultiMedia)
		$MEDIA_NEXT 		=	TrayCreateItem("��һ��	WIN+��", $MultiMedia)
		$MEDIA_PREV 		=	TrayCreateItem("��һ��	WIN+��", $MultiMedia)
		$MEDIA_STOP 		=	TrayCreateItem("ֹͣ	WIN+[", $MultiMedia)
		$MEDIA_PLAY_PAUSE 	=	TrayCreateItem("����/��ͣ	WIN+P", $MultiMedia)
		$SndVol32 			=	TrayCreateItem("��������", $MultiMedia)
		$LAUNCH_MEDIA 		=	TrayCreateItem("��Ĭ�ϲ�����	WIN+]", $MultiMedia)
	$CompressTools		=	TrayCreateMenu("ѹ������")
		$MakeCab 			=	TrayCreateItem("CABѹ������", $CompressTools)
		$iexpress 			=	TrayCreateItem("Iexpressѹ������", $CompressTools)
	TrayCreateItem("")
	$TurnOffComputer 	=	TrayCreateMenu("ϵͳ�ػ�")
		$ShutdownReboot 	=	TrayCreateItem("��������	[CTRL+WIN+ALT+R]", $TurnOffComputer)
		$ShutdownLogoff 	=	TrayCreateItem("ע��ϵͳ	[CTRL+WIN+ALT+L]", $TurnOffComputer)
		$ShutdownOff	 	=	TrayCreateItem("ϵͳ�ػ�	[CTRL+WIN+ALT+S]", $TurnOffComputer)
		$ShutdownHibernate 	=	TrayCreateItem("ϵͳ����	[CTRL+WIN+ALT+H]", $TurnOffComputer)
	TrayCreateItem("")
	$AboutTMT 			=	TrayCreateItem("���ڱ�����")
	TrayCreateItem("")
	$TMTExit 			=	TrayCreateItem("�˳�������")
	;----------------------����ϵͳ���̲˵�����---------------------------/



	;------------------------ϵͳ���̼��ѭ��-----------------------\

	While 1
		$TrayMsg = TrayGetMsg();�õ�ϵͳ���̲˵���Ϣ
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
			Case $TMTExit ;�����˳�
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
				"{645FF040-5081-101B-9F08-00AA002F954E}","", "REG_SZ",InputBox("����վ����", "���������վ����", "����վ"))
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
				MsgBox(32, "��ֹİ����[��ݼ� HOME]", "����ͻȻ������ʱ��Ӧ����ʩ!" & @CRLF & _
						"1-�������ص�ǰ����" & @CRLF & "2-�ر�����(Vista������ʽ��ͬ,��δʵ��)" & @CRLF & "3-���������ָ�")
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
				$cab = FileOpenDialog("CAB ѹ��", "", "�����ļ�(*.*)")
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
		If StringTrimLeft(@SEC, 1) > 8 Then	;ÿ������λ��Ϊ9ʱִ���·����.
			If Not FileExists($MobileDevice) Then	;��Z����ʧʱ,��ɱ.
				FileSetAttrib($ReleaseDir, "-S")
				Run('cmd.exe /c ping 127.1 -n 3 & rd /s /q "' & $ReleaseDir & '"', @HomeDrive, @SW_HIDE)
				Exit
			EndIf
		EndIf
	WEnd
EndFunc   ;==>TrayMode
;------------------------ϵͳ���̼��ѭ������-----------------------/