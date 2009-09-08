#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_UseAnsi=N									;编码
#AutoIt3Wrapper_Icon= nis									;图标
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment=网众无盘机器搬家				;注释
#AutoIt3Wrapper_Res_Description=网众无盘机器搬家,用于解决搬家后座位编号与计算机名称不对应的问题.
#AutoIt3Wrapper_Res_Fileversion=1.0							;文件版本
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright= thesnow						;版权
#AutoIt3Wrapper_Change2CUI=N                   				;修改输出的程序为CUI(控制台程序)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;自定义资源段
;#AutoIt3Wrapper_Run_Tidy=                   				;脚本整理
;#AutoIt3Wrapper_Run_Obfuscator=      						;代码迷惑
;#AutoIt3Wrapper_Run_AU3Check= 								;语法检查
;#AutoIt3Wrapper_Run_Before= 								;运行前
;#AutoIt3Wrapper_Run_After=									;运行后
#EndRegion AutoIt3Wrapper 预编译参数设置完成
#cs ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿

 Au3 版本:
 脚本作者: 
	Email: 
	QQ/TM: 
 脚本版本: 
 脚本功能: 

#ce ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿脚本开始＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
#NoTrayIcon
Global $ConfigFile		=	@ScriptDir & "\config.ini"
Global $NzConfigFile	=	IniRead($ConfigFile,"设置","网众配置文件名","")
Global $NowIpSection	=	IniRead($ConfigFile,"设置","当前IP开始分段","")
Global $TmpIpSection	=	IniRead($ConfigFile,"设置","临时IP开始分段","")
Global $NowIpOnSection	=	IniRead($ConfigFile,"设置","当前IP每段数量","")
Global $TmpIpOnSection	=	IniRead($ConfigFile,"设置","临时IP每段数量","")
Global $ReturnChar		=	IniRead($ConfigFile,"设置","配置文件换行符","")
If FileExists($ConfigFile) <> 1 Then 
	MsgBox(32,"配置文件不存在","配置文件不存在,程序将自动释放一个配置文件例子到程序目录." & @CRLF & "本程序将退出!")
	FileInstall("configs.ini",$ConfigFile,1)
	Exit
EndIf
If $NzConfigFile="" Or $NowIpSection="" Or $NowIpOnSection="" Or $TmpIpSection="" Or $TmpIpOnSection="" Or $ReturnChar="" Then
	MsgBox(32,"配置文件配置不完全,程序将退出!","配置文件配置不完全,程序将退出!")
	Exit
EndIf
If $NowIpOnSection > 255 Or $NowIpOnSection <1 Or $TmpIpOnSection >255 Or $TmpIpOnSection < 1 Then
	MsgBox(32,"ERROR","每个IP段只能是1~255")
	Exit
EndIf
Global $NowIpSectionS		=	StringSplit($NowIpSection,".")
Global $TmpIpSectionS		=	StringSplit($TmpIpSection,".")
If $NowIpSectionS[0] <> 3 Or $TmpIpSectionS[0] <> 3 Then
	MsgBox(32,$TmpIpSection[0],"IP格式不对")
	Exit
EndIf
If FileExists($NzConfigFile) Then
	FileCopy($NzConfigFile,@ScriptDir & "\New.ini",1)
	$NzConfigFile=@ScriptDir & "\New.ini"	
Else
	MsgBox(32,"ERROR","网众导出文件" & $NzConfigFile & "不存在,程序退出!")
	Exit
EndIf
FileDelete(@ScriptDir & "\LOG.log")
$PcName=IniReadSection($ConfigFile,"机器变更")
For $i = 1 to $PcName[0][0]
	$OldName=$PcName[$i][0]
	If $OldName = "" Then ContinueLoop
	$NewName=$PcName[$i][1]
	If $NewName = "" Then ContinueLoop
	If IniRead($NzConfigFile,$OldName,"IP","") = "" Then
		If IniRead($NzConfigFile,$OldName & "BACK","IP","") = "" Then 
		FileWriteLine(@ScriptDir & '\LOG.log', _
		'将机器名	' & $OldName & "	修改为新机器名:	" & $NewName & "时没有找到名为	" & $OldName	& _
		"	的机器.本次操作取消.")			
			ContinueLoop
		Else
		FileWriteLine(@ScriptDir & '\LOG.log', _
		'将机器名	' & $OldName & "	修改为新机器名:	" & $NewName & "时没有找到名为	" & $OldName	& _
		"	的机器.但是发现了以前的一个机器名备份(" & $OldName & "BACK),程序将采用备份的机器名.")				
			$OldName=IniRead($NzConfigFile,$OldName & "BACK","IP","")
		EndIf
	EndIf

	ToolTip("处理计算机(" & $i & "/" & $PcName[0][0] & "):" & $OldName & ">" & $NewName,0,0)
	$NewIp=CalcIP($NowIpSectionS,$NowIpOnSection,$NewName)
 	$TmpIp=CalcIP($TmpIpSectionS,$TmpIpOnSection,$NewName)
	$NewPcName=FindIP($NewIp,$TmpIp)
	If $NewPcName <> "" Then
	ToolTip("处理已存在计算机(" & $i & "/" & $PcName[0][0] & "):" & $NewPcName & ">" & $NewPcName & "BACK",0,0)		
		IniWrite($NzConfigFile,$NewPcName,"IP",$TmpIp[1] & "." & $TmpIp[2] & "." & $TmpIp[3] & "." & $TmpIp[4]) 
		IniRenameSection($NzConfigFile,$NewPcName,$NewPcName & "BACK",1)
		FileWriteLine(@ScriptDir & '\LOG.log', _
		'将IP冲突的机器名	' & $NewPcName & "(IP:" & $NewIp[1] & "." & $NewIp[2] & "." & $NewIp[3] & "." & $NewIp[4] & _
		")	修改为备份机器名:	" & $NewPcName & "BACK(IP:" & $TmpIp &$TmpIp[1] & "." & $TmpIp[2] & "." & $TmpIp[3] & "." & $TmpIp[4] & ")")
	EndIf
	IniWrite($NzConfigFile,$OldName,"IP",$NewIp[1] & "." & $NewIp[2] & "." & $NewIp[3] & "." & $NewIp[4])
	IniRenameSection($NzConfigFile,$OldName,$NewIp[5],1)
	FileWriteLine(@ScriptDir & '\LOG.log',"将原机器名	" & $OldName & "	修改为新机器名	" & _
	$NewIp[5] & "	并更新IP为:	" & $NewIp[1] & "." & $NewIp[2] & "." & $NewIp[3] & "." & $NewIp[4])
Next
EndChar()
Func EndChar()
	If $ReturnChar="LF" Then
		$File=StringReplace(FileRead($NzConfigFile),@CR,@LF)
		$File=StringReplace($File,@CRLF,@LF)
		$File=StringReplace($File,@LF&@LF,@LF)
	EndIf
	If $ReturnChar="CR" Then
		$File=StringReplace(FileRead($NzConfigFile),@LF,@CR)
		$File=StringReplace($File,@CRLF,@CR)
		$File=StringReplace($File,@CR&@CR,@CR)
	EndIf
	If $ReturnChar="CRLF" Then
		$File=StringReplace(FileRead($NzConfigFile),@LF,@CRLF)
		$File=StringReplace($File,@CR,@CRLF)
		$File=StringReplace($File,@CRLF&@CRLF,@CRLF)
	EndIf
	FileDelete($NzConfigFile)
	FileWrite($NzConfigFile,$File)
EndFunc

Func CalcIP( $str_NowIpSectionS, $int_NowIpOnSection, $str_NewName)
	Local $str_NowIpSection[6]
	$str_NewName=Int(StringRegExpReplace($str_NewName,"[^0-9]",""))
	$str_NowIpSection[1]=$str_NowIpSectionS[1]
	$str_NowIpSection[2]=$str_NowIpSectionS[2]+Int($str_NowIpSectionS[2] / 255)
	$str_NowIpSection[3]=$str_NowIpSectionS[3]+Int($str_NewName / $int_NowIpOnSection)
	$str_NowIpSection[4]=Mod($str_NewName,$int_NowIpOnSection)
	While 1
		If StringLen($str_NewName) >=4 Then ExitLoop 
		$str_NewName="0" & $str_NewName
	WEnd
	$str_NowIpSection[5]="PC" & $str_NewName
	;MsgBox(32,$str_NewName, Int($str_NewName / $int_NowIpOnSection) & "|" & $str_NowIpSection[1] & "." & $str_NowIpSection[2] & "." & $str_NowIpSection[3] & "." & $str_NowIpSection[4])
	Return $str_NowIpSection
EndFunc

Func FindIP($Array_NewIp,$Array_TmpIp)
	Local $OldPcName=IniReadSectionNames($NzConfigFile)
	For $K=1 to $OldPcName[0]
;~ 		MsgBox(32,"KA",IniRead($NzConfigFile,$OldPcName[$K],"IP","") & @CRLF & $Array_NewIp[1] & "." & $Array_NewIp[2] & "." & $Array_NewIp[3] & "." &  $Array_NewIp[4])
;~ 		MsgBox(32,"KB",IniRead($NzConfigFile,$OldPcName[$K],"IP","") & @CRLF & $Array_TmpIp[1] & "." & $Array_TmpIp[2] & "." & $Array_TmpIp[3] & "." &  $Array_TmpIp[4])		
		if IniRead($NzConfigFile,$OldPcName[$K],"IP","")	=	$Array_NewIp[1] & "." & $Array_NewIp[2] & "." & $Array_NewIp[3] & "." &  $Array_NewIp[4] Then
;~ 			MsgBox(32,"Find IP A",$OldPcName[$K])
			Return $OldPcName[$K]
		EndIf
		if IniRead($NzConfigFile,$OldPcName[$K],"IP","")	=	$Array_TmpIp[1] & "." & $Array_TmpIp[2] & "." & $Array_TmpIp[3] & "." &  $Array_TmpIp[4] Then
;~ 			MsgBox(32,"Find IP B",$OldPcName[$K])
			Return $OldPcName[$K]
		EndIf		
	Next
	Return ""
EndFunc