#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_Icon= nis										;ͼ��,֧��EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_OutFile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=n 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= ������(��������)								;ע��
#AutoIt3Wrapper_Res_Description=������(��������)							;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=1.0							;�ļ��汾
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright= thesnoW						;��Ȩ
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
#include <IE.au3>
#include <String.au3>
;~ MsgBox(32,"",$cmdline[0])
If $cmdline[0] <> 1 Then Exit
Global $Name = IniRead('POST.ini','POST','NAME',"")
Global $CFG = IniRead('POST.ini','POST','CFG',"")
Global $URL = IniRead('POST.ini','POST','URL',"")
Global $PostUrl = IniRead('POST.ini','POST','POSTURL',"")
Global $BOARD = IniRead('POST.ini','POST','POSTBOARD',"")
Global $BBSVER=	IniRead('POST.ini','POST','BBSVER',"DZ7")	
Global $PostText = FileRead("�ı�.txt")
Global $PostTitle = FileRead("����.txt")

Switch $BBSVER
	Case 'DZ7'
		While 1
			PostFileDZ7()
		WEnd
	Case 'DZ6.1'
		While 1
			PostFileDZ7()
		WEnd
	Case 'DZ6'
		While 1
			PostFileDZ7()
		WEnd
	Case Else
		MsgBox(32,"",'δ֪��̳�汾')
		Exit -1
EndSwitch

Func PostFileDZ7()
	;�����FID
	Local $PostUrlFid = IniRead(@ScriptDir & '\forum\' & $CFG, $BOARD, "URL", '')
	If $PostUrlFid = '' Then
		MsgBox(32, "", "���ȵõ�(�޸�)��̳�����Ϣ")
		Exit -2
	EndIf
	$PostUrlFid = StringReplace($PostUrlFid, 'forumdisplay.php?fid=', '')
	If Int($PostUrlFid)=0 Then
		Local $tmp = _StringBetween($PostUrlFid, '-', '-')
		If IsArray($tmp) Then
			$PostUrlFid = $tmp[0]
		Else
			MsgBox(32, "����FID", $PostUrlFid)
			Exit -2
		EndIf
	EndIf
	;������а�
	ClipPut($PostText)
	;����URL
	Local $URLS = $URL & '/' & $PostUrl & '?action=newthread&fid=' & $PostUrlFid
;~ 	$time = TimerInit()
	;����IE
	$oIE = _IECreate($URLS, 0, 1, 1, 1)
	;�õ��������ݴ���
	$oForm = _IEFormGetObjByName($oIE, "postform")
	If IsObj($oForm) Then
		;ѡ��Դ��
		_IEFormElementCheckBoxSelect($oForm, '0', 'checkbox', 1)

		;���ñ���
		$oQuery = _IEGetObjById($oForm, "subject") ;����
		If IsObj($oQuery) Then
			_IEFormElementSetValue($oQuery, $PostTitle)
		EndIf
		;��������
		$Message = _IEFormElementGetObjByName($oForm, 'message') ;DZ7.1
		If IsObj($Message) Then 
			_IEFormElementSetValue($Message, $PostText)
		EndIf
	Else
		$oForm=_IEFormGetObjByName($oIE, "loginform")
		If IsObj($oForm) Then 
			$oQuery = _IEGetObjById($oForm, "username")
			_IEFormElementSetValue($oQuery, IniRead(@ScriptDir & "\site.ini",$Name,'username',""))
			$oQuery = _IEGetObjById($oForm, "password")
			_IEFormElementSetValue($oQuery, IniRead(@ScriptDir & "\site.ini",$Name,'password',""))
			_IEFormElementCheckBoxSelect($oForm, '0', '2592000', 1)
			;_IEFormElementCheckBoxSelect($oForm, '0', 'cookietime', 1)
			If MsgBox(36,"","����û�е�¼��̳...,��¼��[ȷ��]���·���,��[ȡ��]�˳�") =6 Then
				_IEQuit($oIE)
				Return
			Else
				_IEQuit($oIE)
				Exit -4
			EndIf
			
		Else
			MsgBox(32, "", "δ����DZ7��������,������ҳ�治��(�ȼ�����,��ֹ������)!")
		EndIf
		_IEQuit($oIE)
		Exit -3
	EndIf
	If MsgBox(36, 0, "������ɺ���,�Ƿ�ɹ�����?") = 6 Then
		FileWriteLine("log.txt",Date() & "	�ɹ���(" & $Name & ")������һƪ��Ϊ(" & $PostTitle & ")������")
		_IEQuit($oIE)
	Else
		FileWriteLine("log.txt",Date() & "	û����(" & $Name & ")����һƪ��Ϊ(" & $PostTitle & ")������")
		_IEQuit($oIE)
	EndIf
	Exit 0
EndFunc   ;==>postfile

Func Date()
	Local $DATE='[' & @YEAR & @MON & @MDAY & '/' & @HOUR & ":" & @MIN & ":" & @SEC & ']'
	Return $DATE
EndFunc   ;==>date