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
If $cmdline[0] <> 5 Then Exit
postfile($cmdline[5])
Func postfile($BOARD)
	;��������
	Local $Name = $cmdline[1]
	Local $CFG = $cmdline[2]
	Local $URL = $cmdline[3]
	Local $PostUrl = $cmdline[4]
	Local $PostUrlFid = IniRead(@ScriptDir & '\forum\' & $CFG, $BOARD, "URL", '')
	If $PostUrlFid = '' Then
		MsgBox(32, "", "���ȵõ���̳�����Ϣ")
		Return
	EndIf
	$PostUrlFid = StringReplace($PostUrlFid, 'forumdisplay.php?fid=', '')
;	GUICtrlSetData($Label9, StringInStr($PostUrl, '.htm'))
	If Int($PostUrlFid)=0 Then
		Local $tmp = _StringBetween($PostUrlFid, '-', '-')
		If IsArray($tmp) Then
			$PostUrlFid = $tmp[0]
		Else
			MsgBox(32, "����FID", $PostUrlFid)
			Return
		EndIf
	EndIf
	Local $PostText = FileRead("�ı�.txt")
	Local $PostTitle = FileRead("����.txt")
	ClipPut($PostText)
	$url = $url & '/' & $PostUrl & '?action=newthread&fid=' & $PostUrlFid
;~ 	$time = TimerInit()
;~ 	MsgBox(32,"","b")
	$oIE = _IECreate($url, 0, 1, 1, 1) ;MTV
;~ 	If IsObj($oIE) Then MsgBox(32,0,1)
;~ 	ConsoleWrite('IE' & @CRLF & TimerDiff($time) & @CRLF)

	$oForm = _IEFormGetObjByName($oIE, "postform")
	If IsObj($oForm) Then
;~ 		ConsoleWrite('postform' & @CRLF & TimerDiff($time) & @CRLF)

		_IEFormElementCheckBoxSelect($oForm, '0', 'checkbox', 1)
;~ 		ConsoleWrite('checkbox' & @CRLF & TimerDiff($time) & @CRLF)

		$oQuery = _IEGetObjById($oForm, "subject") ;����
		If IsObj($oQuery) Then
			_IEFormElementSetValue($oQuery, $PostTitle)
		EndIf
;~ 		ConsoleWrite('subject' & @CRLF & TimerDiff($time) & @CRLF)

;~ 		$Message = _IEFormElementGetObjByName($oForm, 'newediter')
;~ 		If IsObj($Message) Then ConsoleWrite('newediter' & @CRLF)
;~ 		ConsoleWrite('newediter' & @CRLF & TimerDiff($time) & @CRLF)

		$Message = _IEFormElementGetObjByName($oForm, 'message') ;7.1	status
		If IsObj($Message) Then 
;~ 			ConsoleWrite('message' & @CRLF)
			_IEFormElementSetValue($Message, $PostText)
		EndIf

;~ 		$Message = _IEFormElementGetObjByName($oForm, 'e_iframe')
;~ 		If IsObj($Message) Then 
;~ 			ConsoleWrite('e_iframe' & @CRLF)
;~ 		EndIf

;~ 		$Message = _IEFormElementGetObjByName($oForm, 'wysiwyg') ;7.1	status
;~ 		If IsObj($Message) Then ConsoleWrite('wysiwyg' & @CRLF)
;~ 		_IEFormElementSetValue($Message, $PostText)

	Else
		MsgBox(32, "", "δ����DZ7��������,������δ��¼��ɵ�!")
		_IEQuit($oIE)
		Return 0
	EndIf
;~ 	ConsoleWrite('End' & @CRLF & TimerDiff($time) & @CRLF)
	If MsgBox(36, 0, "������ɺ���,�Ƿ�ɹ�����?") = 6 Then
		FileWriteLine("log.txt", "�ɹ���(" & $Name & ")������һƪ��Ϊ(" & $PostTitle & ")������");	' & date())
		_IEQuit($oIE)
;~ 		MsgBox(32,"","q")
	Else
		FileWriteLine("log.txt", "û����(" & $Name & ")����һƪ��Ϊ(" & $PostTitle & ")������");	' & date())
		_IEQuit($oIE)
	EndIf
;~ 	$t=_IEFormElementGetCollection($oFORM)
;~ 	For $j=1 To @extended
;~ 		_IEFormElementSetValue($t,$j)
;~ 	Next



;~ 	$oFORM = _IEFormGetObjByName($IE, 'FORM')
;~ 	$oINPUT = _IEFormElementGetObjByName($oForm, 'wysiwyg')
;~ 	_IEFormElementSetValue($oINPUT, "asdasdasd")
;~ ;_IEFormSubmit ($oForm)
	;AdlibRegister("LoadData")
	;$attach = _IEGetObjById($ie,"attachnew_1")					;��Ӹ���
	;$attach.click
	;AdlibUnRegister("LoadData")
	Return 1
EndFunc   ;==>postfile

Func date()
	Return '[' & @YEAR & @MON & @MDAY & '/' & @HOUR & ":" & @MIN & ":" & @SEC & ']'
EndFunc   ;==>date