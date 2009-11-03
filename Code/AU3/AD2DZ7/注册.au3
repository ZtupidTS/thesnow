#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon= 										;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment= 								;注释
#AutoIt3Wrapper_Res_Description=							;详细信息
#AutoIt3Wrapper_Res_Fileversion=							;文件版本
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright= 						;版权
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

Func RegisterBbs($url)
	;$url = URL()
	If IniRead(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "已注册", 0) = 1 Then
		If MsgBox(36, "", "上次的报告说已经成功注册了,是否重新注册?") <> 6 Then Return
	EndIf
	$url2 = GetRegistrAddr()
	If Not StringInStr($url2, "http:") Then $url2 = $url & "/" & $url2
	;	$url2 = $url; & "/registerbbs.php"
	$ie = _IECreate($url2, 1, 1, 1, 1) ;MTV
	$oForm = _IEFormGetObjByName($ie, "registerform")
	If Not IsObj($oForm) Then
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "已注册", 0)
		MsgBox(32, "错误", "很明显,这个弱智的程序,找不到注册地址.",5)
		_IEQuit($ie)
		Return 0
	EndIf
	$oQuery = _IEGetObjById($oForm, "invitecode") ;用户名
	If $oQuery <> 0 Then
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "已注册", 0)
		MsgBox(32, "错误", "很明显,需要邀请码.帮不了你.")
		_IEQuit($ie)
		Return 0
	EndIf

	$oQuery = _IEGetObjById($oForm, "username") ;用户名
	_IEFormElementSetValue($oQuery, $UserName)
	$oQuery = _IEGetObjById($oForm, "email") ;email
	_IEFormElementSetValue($oQuery, '123@sb23456.com')
	$oQuery = _IEGetObjById($oForm, "password") ;password
	_IEFormElementSetValue($oQuery, $PassWord)
	$oQuery = _IEGetObjById($oForm, "password2") ;password
	_IEFormElementSetValue($oQuery, $PassWord)
	$oQuery = _IEGetObjById($oForm, "seccodeverify") ;验证
	$oQuery.click()
	_IEFormElementSetValue($oQuery, '')
	If MsgBox(36, "能不能注册?", "是不是注册成功了?") = 6 Then
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "已注册", 1)
		_IEQuit($ie)
		getBBSinfo()
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "username", $UserName)
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "password", $PassWord)
		$url = GUICtrlRead($InputURL)
		FileMove(@ScriptDir & "\site\" & $url,@ScriptDir & "\X\" & $url,1)
		Return 1
	Else
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "已注册", 0)
		_IEQuit($ie)
	EndIf
;~ 	$oQuery = _IEGetObjById ($oForm, "registerformsubmit")			;提交
;~ 	$oQuery.click()
;~ 	$oQuery = _IEGetObjById ($oForm, "checkseccodeverify")			;校检验证
;~ 	$oQuery = _IEFormElementGetValue ($oQuery)						;
EndFunc   ;==>RegisterBbs


Func GetRegistrAddr()
	ToolTip('检测论坛数据中,请等待.', 0, 0)
	$url = URL()
	$oIE = _IECreate($url, 1, 0, 1, 1) ;MTV
	$src = _IEBodyReadHTML($oIE)
	_IEQuit($oIE)
	$reg = _StringBetween($src, '<A class=lightlink', '</A>')
	If Not IsArray($reg) Then
		$reg = _StringBetween($src, '<li><a href="', '" onclick="')
		If Not IsArray($reg) Then
			$reg=''
		Else
			$reg = $reg[0]
		EndIf
	Else
		$reg = _StringBetween($reg[0], 'href="', '">')
		$reg = $reg[0]
	EndIf
	ToolTip("")
	Return $reg
EndFunc   ;==>GetRegistrAddr

Func getBBSinfo()
	ToolTip("正在得到信息,不要关闭IE窗口.", 0, 0)
	$url = URL()
	$oIE = _IECreate($url, 1, 1, 1, 1) ;MTV
	$src = _IEBodyReadHTML($oIE)
	$mini = _StringBetween($src, '<H2><A href="', '</H2>')
	For $j = 0 To UBound($mini) - 1
		$urla = StringTrimRight($mini[$j], (StringLen($mini[$j]) - StringInStr($mini[$j], '">')) + 1)
		$urlb = StringTrimRight($mini[$j], (StringLen($mini[$j]) - StringInStr($mini[$j], '</A>')) + 1)
		$urlb = StringTrimLeft($urlb, StringInStr($mini[$j], '">') + 1)
		GUICtrlCreateTreeViewItem($urlb, $TreeView1)
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", $urlb, "url", $urla)
	Next
	_IEQuit($oIE)
	ToolTip("")
EndFunc   ;==>getBBSinfo