#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_UseAnsi=N									;����
#AutoIt3Wrapper_Icon= nis									;ͼ��
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_OutFile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment=�������̻������				;ע��
#AutoIt3Wrapper_Res_Description=�������̻������,���ڽ����Һ���λ������������Ʋ���Ӧ������.
#AutoIt3Wrapper_Res_Fileversion=1.0							;�ļ��汾
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright= thesnow						;��Ȩ
#AutoIt3Wrapper_Change2CUI=N                   				;�޸�����ĳ���ΪCUI(����̨����)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;�Զ�����Դ��
;#AutoIt3Wrapper_Run_Tidy=                   				;�ű�����
;#AutoIt3Wrapper_Run_Obfuscator=      						;�����Ի�
;#AutoIt3Wrapper_Run_AU3Check= 								;�﷨���
;#AutoIt3Wrapper_Run_Before= 								;����ǰ
;#AutoIt3Wrapper_Run_After=									;���к�
#EndRegion AutoIt3Wrapper Ԥ��������������
#cs �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�

 Au3 �汾:
 �ű�����: 
	Email: 
	QQ/TM: 
 �ű��汾: 
 �ű�����: 

#ce �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣ߽ű���ʼ�ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
#NoTrayIcon
Global $ConfigFile		=	@ScriptDir & "\config.ini"
Global $NzConfigFile	=	IniRead($ConfigFile,"����","���������ļ���","")
Global $NowIpSection	=	IniRead($ConfigFile,"����","��ǰIP��ʼ�ֶ�","")
Global $TmpIpSection	=	IniRead($ConfigFile,"����","��ʱIP��ʼ�ֶ�","")
Global $NowIpOnSection	=	IniRead($ConfigFile,"����","��ǰIPÿ������","")
Global $TmpIpOnSection	=	IniRead($ConfigFile,"����","��ʱIPÿ������","")
Global $ReturnChar		=	IniRead($ConfigFile,"����","�����ļ����з�","")
If FileExists($ConfigFile) <> 1 Then 
	MsgBox(32,"�����ļ�������","�����ļ�������,�����Զ��ͷ�һ�������ļ����ӵ�����Ŀ¼." & @CRLF & "�������˳�!")
	FileInstall("configs.ini",$ConfigFile,1)
	Exit
EndIf
If $NzConfigFile="" Or $NowIpSection="" Or $NowIpOnSection="" Or $TmpIpSection="" Or $TmpIpOnSection="" Or $ReturnChar="" Then
	MsgBox(32,"�����ļ����ò���ȫ,�����˳�!","�����ļ����ò���ȫ,�����˳�!")
	Exit
EndIf
If $NowIpOnSection > 255 Or $NowIpOnSection <1 Or $TmpIpOnSection >255 Or $TmpIpOnSection < 1 Then
	MsgBox(32,"ERROR","ÿ��IP��ֻ����1~255")
	Exit
EndIf
Global $NowIpSectionS		=	StringSplit($NowIpSection,".")
Global $TmpIpSectionS		=	StringSplit($TmpIpSection,".")
If $NowIpSectionS[0] <> 3 Or $TmpIpSectionS[0] <> 3 Then
	MsgBox(32,$TmpIpSection[0],"IP��ʽ����")
	Exit
EndIf
If FileExists($NzConfigFile) Then
	FileCopy($NzConfigFile,@ScriptDir & "\New.ini",1)
	$NzConfigFile=@ScriptDir & "\New.ini"	
Else
	MsgBox(32,"ERROR","���ڵ����ļ�" & $NzConfigFile & "������,�����˳�!")
	Exit
EndIf
FileDelete(@ScriptDir & "\LOG.log")
$PcName=IniReadSection($ConfigFile,"�������")
For $i = 1 to $PcName[0][0]
	$OldName=$PcName[$i][0]
	If $OldName = "" Then ContinueLoop
	$NewName=$PcName[$i][1]
	If $NewName = "" Then ContinueLoop
	If IniRead($NzConfigFile,$OldName,"IP","") = "" Then
		If IniRead($NzConfigFile,$OldName & "BACK","IP","") = "" Then 
		FileWriteLine(@ScriptDir & '\LOG.log', _
		'��������	' & $OldName & "	�޸�Ϊ�»�����:	" & $NewName & "ʱû���ҵ���Ϊ	" & $OldName	& _
		"	�Ļ���.���β���ȡ��.")			
			ContinueLoop
		Else
		FileWriteLine(@ScriptDir & '\LOG.log', _
		'��������	' & $OldName & "	�޸�Ϊ�»�����:	" & $NewName & "ʱû���ҵ���Ϊ	" & $OldName	& _
		"	�Ļ���.���Ƿ�������ǰ��һ������������(" & $OldName & "BACK),���򽫲��ñ��ݵĻ�����.")				
			$OldName=IniRead($NzConfigFile,$OldName & "BACK","IP","")
		EndIf
	EndIf

	ToolTip("��������(" & $i & "/" & $PcName[0][0] & "):" & $OldName & ">" & $NewName,0,0)
	$NewIp=CalcIP($NowIpSectionS,$NowIpOnSection,$NewName)
 	$TmpIp=CalcIP($TmpIpSectionS,$TmpIpOnSection,$NewName)
	$NewPcName=FindIP($NewIp,$TmpIp)
	If $NewPcName <> "" Then
	ToolTip("�����Ѵ��ڼ����(" & $i & "/" & $PcName[0][0] & "):" & $NewPcName & ">" & $NewPcName & "BACK",0,0)		
		IniWrite($NzConfigFile,$NewPcName,"IP",$TmpIp[1] & "." & $TmpIp[2] & "." & $TmpIp[3] & "." & $TmpIp[4]) 
		IniRenameSection($NzConfigFile,$NewPcName,$NewPcName & "BACK",1)
		FileWriteLine(@ScriptDir & '\LOG.log', _
		'��IP��ͻ�Ļ�����	' & $NewPcName & "(IP:" & $NewIp[1] & "." & $NewIp[2] & "." & $NewIp[3] & "." & $NewIp[4] & _
		")	�޸�Ϊ���ݻ�����:	" & $NewPcName & "BACK(IP:" & $TmpIp &$TmpIp[1] & "." & $TmpIp[2] & "." & $TmpIp[3] & "." & $TmpIp[4] & ")")
	EndIf
	IniWrite($NzConfigFile,$OldName,"IP",$NewIp[1] & "." & $NewIp[2] & "." & $NewIp[3] & "." & $NewIp[4])
	IniRenameSection($NzConfigFile,$OldName,$NewIp[5],1)
	FileWriteLine(@ScriptDir & '\LOG.log',"��ԭ������	" & $OldName & "	�޸�Ϊ�»�����	" & _
	$NewIp[5] & "	������IPΪ:	" & $NewIp[1] & "." & $NewIp[2] & "." & $NewIp[3] & "." & $NewIp[4])
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