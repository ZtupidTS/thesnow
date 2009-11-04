#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon= nis										;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=n 									;使用压缩
#AutoIt3Wrapper_Res_Comment= 发帖器(发帖功能)								;注释
#AutoIt3Wrapper_Res_Description=发帖器(发帖功能)							;详细信息
#AutoIt3Wrapper_Res_Fileversion=1.0							;文件版本
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright= thesnoW						;版权
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
#include <IE.au3>
#include <String.au3>
;~ MsgBox(32,"",$cmdline[0])
If $cmdline[0] <> 5 Then Exit
postfile($cmdline[5])
Func postfile($BOARD)
	;创建对象
	Local $Name = $cmdline[1]
	Local $CFG = $cmdline[2]
	Local $URL = $cmdline[3]
	Local $PostUrl = $cmdline[4]
	Local $PostUrlFid = IniRead(@ScriptDir & '\forum\' & $CFG, $BOARD, "URL", '')
	If $PostUrlFid = '' Then
		MsgBox(32, "", "请先得到论坛版块信息")
		Return
	EndIf
	$PostUrlFid = StringReplace($PostUrlFid, 'forumdisplay.php?fid=', '')
;	GUICtrlSetData($Label9, StringInStr($PostUrl, '.htm'))
	If Int($PostUrlFid)=0 Then
		Local $tmp = _StringBetween($PostUrlFid, '-', '-')
		If IsArray($tmp) Then
			$PostUrlFid = $tmp[0]
		Else
			MsgBox(32, "错误FID", $PostUrlFid)
			Return
		EndIf
	EndIf
	Local $PostText = FileRead("文本.txt")
	Local $PostTitle = FileRead("标题.txt")
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

		$oQuery = _IEGetObjById($oForm, "subject") ;标题
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
		MsgBox(32, "", "未发现DZ7发帖界面,可能是未登录造成的!")
		_IEQuit($oIE)
		Return 0
	EndIf
;~ 	ConsoleWrite('End' & @CRLF & TimerDiff($time) & @CRLF)
	If MsgBox(36, 0, "请在完成后点击,是否成功发帖?") = 6 Then
		FileWriteLine("log.txt", "成功在(" & $Name & ")发布了一篇名为(" & $PostTitle & ")的文章");	' & date())
		_IEQuit($oIE)
;~ 		MsgBox(32,"","q")
	Else
		FileWriteLine("log.txt", "没能在(" & $Name & ")发布一篇名为(" & $PostTitle & ")的文章");	' & date())
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
	;$attach = _IEGetObjById($ie,"attachnew_1")					;添加附件
	;$attach.click
	;AdlibUnRegister("LoadData")
	Return 1
EndFunc   ;==>postfile

Func date()
	Return '[' & @YEAR & @MON & @MDAY & '/' & @HOUR & ":" & @MIN & ":" & @SEC & ']'
EndFunc   ;==>date