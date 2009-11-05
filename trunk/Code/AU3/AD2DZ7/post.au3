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
If $cmdline[0] <> 1 Then Exit
Global $Name = IniRead('POST.ini','POST','NAME',"")
Global $CFG = IniRead('POST.ini','POST','CFG',"")
Global $URL = IniRead('POST.ini','POST','URL',"")
Global $PostUrl = IniRead('POST.ini','POST','POSTURL',"")
Global $BOARD = IniRead('POST.ini','POST','POSTBOARD',"")
Global $BBSVER=	IniRead('POST.ini','POST','BBSVER',"DZ7")	
Global $PostText = FileRead("文本.txt")
Global $PostTitle = FileRead("标题.txt")

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
		MsgBox(32,"",'未知论坛版本')
		Exit -1
EndSwitch

Func PostFileDZ7()
	;检测版块FID
	Local $PostUrlFid = IniRead(@ScriptDir & '\forum\' & $CFG, $BOARD, "URL", '')
	If $PostUrlFid = '' Then
		MsgBox(32, "", "请先得到(修改)论坛版块信息")
		Exit -2
	EndIf
	$PostUrlFid = StringReplace($PostUrlFid, 'forumdisplay.php?fid=', '')
	If Int($PostUrlFid)=0 Then
		Local $tmp = _StringBetween($PostUrlFid, '-', '-')
		If IsArray($tmp) Then
			$PostUrlFid = $tmp[0]
		Else
			MsgBox(32, "错误FID", $PostUrlFid)
			Exit -2
		EndIf
	EndIf
	;放入剪切板
	ClipPut($PostText)
	;发帖URL
	Local $URLS = $URL & '/' & $PostUrl & '?action=newthread&fid=' & $PostUrlFid
;~ 	$time = TimerInit()
	;创建IE
	$oIE = _IECreate($URLS, 0, 1, 1, 1)
	;得到发帖内容窗口
	$oForm = _IEFormGetObjByName($oIE, "postform")
	If IsObj($oForm) Then
		;选中源码
		_IEFormElementCheckBoxSelect($oForm, '0', 'checkbox', 1)

		;设置标题
		$oQuery = _IEGetObjById($oForm, "subject") ;标题
		If IsObj($oQuery) Then
			_IEFormElementSetValue($oQuery, $PostTitle)
		EndIf
		;设置内容
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
			If MsgBox(36,"","您并没有登录论坛...,登录后按[确定]重新发帖,按[取消]退出") =6 Then
				_IEQuit($oIE)
				Return
			Else
				_IEQuit($oIE)
				Exit -4
			EndIf
			
		Else
			MsgBox(32, "", "未发现DZ7发帖界面,可能是页面不对(等级不够,禁止发帖等)!")
		EndIf
		_IEQuit($oIE)
		Exit -3
	EndIf
	If MsgBox(36, 0, "请在完成后点击,是否成功发帖?") = 6 Then
		FileWriteLine("log.txt",Date() & "	成功在(" & $Name & ")发布了一篇名为(" & $PostTitle & ")的文章")
		_IEQuit($oIE)
	Else
		FileWriteLine("log.txt",Date() & "	没能在(" & $Name & ")发布一篇名为(" & $PostTitle & ")的文章")
		_IEQuit($oIE)
	EndIf
	Exit 0
EndFunc   ;==>postfile

Func Date()
	Local $DATE='[' & @YEAR & @MON & @MDAY & '/' & @HOUR & ":" & @MIN & ":" & @SEC & ']'
	Return $DATE
EndFunc   ;==>date