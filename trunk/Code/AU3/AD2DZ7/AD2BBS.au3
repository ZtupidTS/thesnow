#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon= 	nis									;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_Outfile=e:\script\AD2BBS.EXE									;输出文件名
#AutoIt3Wrapper_UseX64=n
#AutoIt3Wrapper_Outfile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment= 								;注释
#AutoIt3Wrapper_Res_Description=thesnoW							;详细信息
#AutoIt3Wrapper_Res_Fileversion=0.0.1.1
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;自动更新版本
#AutoIt3Wrapper_Res_LegalCopyright=thesnoW 						;版权
#AutoIt3Wrapper_Change2CUI=N                   				;修改输出的程序为CUI(控制台程序)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;自定义资源段
;#AutoIt3Wrapper_Run_Tidy=                   				;脚本整理
;#AutoIt3Wrapper_Run_Obfuscator=      						;代码迷惑
;#AutoIt3Wrapper_Run_AU3Check= 								;语法检查
;#AutoIt3Wrapper_Run_Before= 								;运行前
;#AutoIt3Wrapper_Run_After=									;运行后
#EndRegion AutoIt3Wrapper 预编译参数(常用参数)
#cs ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
	
	Au3 版本:
	脚本作者:
	Email:
	QQ/TM:
	脚本版本:
	脚本功能:
	
#ce ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿脚本开始＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
#Region 	包含文件
#NoTrayIcon
#include <Array.au3>
#include <IE.au3>
#include <String.au3>
#include <ComboConstants.au3>
#include <GuiTreeView.au3>
#include <ButtonConstants.au3>
#include <EditConstants.au3>
#include <GUIConstantsEx.au3>
#include <StaticConstants.au3>
#include <TreeViewConstants.au3>
#include <WindowsConstants.au3>
#EndRegion 	包含文件





#Region 声明变量
;Global $board[1],$board2[1]
Global $UserName = 'KOKR2009'
Global $PassWord = '123456'
DirCreate("forum")
DirCreate("site")
DirCreate("X")
#EndRegion 声明变量

#Region ### START Koda GUI section ### Form=f:\workspace\svn\svnthesnow\trunk\code\au3\ad2dz7\main.kxf
$Main = GUICreate("AD2DZ7(如遇到IE慢,请手动在页面加载完成后点击IE的停止) v" & FileGetVersion(@ScriptFullPath), 522, 377, 304, 237)
$Button1 = GUICtrlCreateButton("注册论坛", 8, 344, 57, 25)
$Button2 = GUICtrlCreateButton("得到版块", 72, 344, 65, 25)
$Button3 = GUICtrlCreateButton("登录论坛", 144, 344, 73, 25)
$Button4 = GUICtrlCreateButton("开始发帖", 224, 344, 73, 25)
$Label1 = GUICtrlCreateLabel("配置文件:", 8, 8, 55, 17)
$Button5 = GUICtrlCreateButton("退出", 440, 344, 65, 25)
$TreeView1 = GUICtrlCreateTreeView(8, 40, 257, 289, BitOR($TVS_HASBUTTONS, $TVS_HASLINES, $TVS_LINESATROOT, $TVS_DISABLEDRAGDROP, $TVS_SHOWSELALWAYS, $TVS_CHECKBOXES, $WS_GROUP, $WS_TABSTOP))
$CFG_NAME = GUICtrlCreateCombo("", 64, 8, 161, 25)
$Button6 = GUICtrlCreateButton("载入", 232, 8, 33, 25)
$Group1 = GUICtrlCreateGroup("自定义信息:", 272, 8, 241, 321)
GUICtrlCreateLabel("用户名称:", 280, 48, 55, 17)
$GUI_USER = GUICtrlCreateInput("", 336, 48, 161, 21)
GUICtrlCreateLabel("账号密码:", 280, 72, 55, 17)
$GUI_PASS = GUICtrlCreateInput("", 336, 72, 161, 21)
GUICtrlCreateLabel("URL:", 280, 24, 29, 17)
$GUI_URL = GUICtrlCreateInput("", 312, 24, 185, 21)
GUICtrlCreateLabel("设置文件:", 280, 96, 55, 17)
$GUI_CFG = GUICtrlCreateInput("", 336, 96, 161, 21)
GUICtrlCreateLabel("论坛版本:", 280, 120, 55, 17)
$GUI_VER = GUICtrlCreateInput("", 336, 120, 129, 21)
GUICtrlCreateLabel("发帖地址:", 280, 144, 55, 17)
$GUI_POST = GUICtrlCreateInput("", 336, 144, 161, 21)
$Button7 = GUICtrlCreateButton("保存设置", 440, 296, 65, 25)
$Button8 = GUICtrlCreateButton("测", 472, 120, 25, 17)
GUICtrlCreateLabel("是否静态:", 280, 168, 55, 17)
$Label9 = GUICtrlCreateLabel("否", 336, 168, 16, 17)
GUICtrlCreateGroup("", -99, -99, 1, 1)
;~ AddFile2List()
AddSite2List()
GUISetState(@SW_SHOW)
WinSetOnTop($Main, "", 1)
#EndRegion ### END Koda GUI section ###






While 1
	$nMsg = GUIGetMsg()
	Switch $nMsg
		Case $GUI_EVENT_CLOSE
			Exit
		Case $Button1
;~ 			RegisterBbs()
		Case $Button2
			getBBSinfo()
		Case $Button3
			AutoLogin()
		Case $Button4
			Post()
		Case $Button5
			Exit
		Case $Button6
			;LoadFile()
			LoadSite()
		Case $Button7
			SaveSite()
	EndSwitch
WEnd

;~ While 1
;~ 	Sleep(5000)
;~ 	havefile()
;~ WEnd

;~ Func havefile()
;~ 	; 显示当前目录中所有文件的文件名
;~ 	$search = FileFindFirstFile(@ScriptDir & "\site\*.*")

;~ 	; 检查搜索是否成功
;~ 	If $search = -1 Then
;~     MsgBox(0, "错误", "没有文件/目录 匹配搜索")
;~ 		Return
;~ 	EndIf

;~ 	While 1
;~ 		$file = FileFindNextFile($search)
;~ 		If @error Then ExitLoop
;~
;~ 			$files = FileOpen(@ScriptDir & "\site\" & $file, 0)

;~ 			; 检查打开的文件是否可读
;~ 			If $files = -1 Then
;~ 				Return
;~ 			EndIf

;~ 			; 每次读取一行文本,直到文件结束.
;~ 			While 1
;~ 				$line = FileReadLine($files)
;~ 				If @error = -1 Then ExitLoop
;~ 				GUICtrlSetData($CFG_NAME, $line)
;~ 				;MsgBox(32,"",$line)
;~ 				If RegisterBbs($line) = 0 Then
;~ 					FileWriteLine("f.txt",$line)
;~ 				Else
;~ 					FileWriteLine("t.txt",$line)
;~ 				EndIf
;~ 			Wend

;~ 			FileClose($files)
;~ 			FileMove(@ScriptDir & "\site\" & $file,@ScriptDir & "\X\" & $file,1)
;~
;~ 		;  MsgBox(4096, "文件:", $file)
;~ 	WEnd

;~ 	; 关闭搜索句柄
;~ 	FileClose($search)


;~ EndFunc   ;==>havefile


Func LoadFile()
	Local $url = URL()
	Local $board = IniReadSectionNames(@ScriptDir & '\forum\' & $url & ".bbs")
	_GUICtrlTreeView_DeleteAll($TreeView1)
	For $i = 1 To $board[0]
		If $board[$i] <> 'forum' Then
			GUICtrlCreateTreeViewItem($board[$i], $TreeView1)
			If IniRead(@ScriptDir & '\forum\' & $url & ".bbs", $board[$i], "checked", "0") = 1 Then
				GUICtrlSetState(-1, $GUI_CHECKED)
			EndIf
		EndIf
	Next
EndFunc   ;==>LoadFile

Func LoadSite()
	Local $Name = GUICtrlRead($CFG_NAME)
	If $Name = '' Then Return
	Local $config = IniRead('site.ini', $Name, 'config', '')
	GUICtrlSetData($GUI_CFG, $config)
	GUICtrlSetData($GUI_PASS, IniRead('site.ini', $Name, 'password', ""))
	GUICtrlSetData($GUI_USER, IniRead('site.ini', $Name, 'username', ""))
	GUICtrlSetData($GUI_URL, IniRead('site.ini', $Name, 'URL', ""))
	GUICtrlSetData($GUI_VER, IniRead('site.ini', $Name, 'version', ""))
	GUICtrlSetData($GUI_POST, IniRead('site.ini', $Name, 'POST', ""))
	_GUICtrlTreeView_DeleteAll($TreeView1)
	Local $board = IniReadSectionNames(@ScriptDir & '\forum\' & $config)
	If IsArray($board) Then
		For $i = 1 To $board[0]
			GUICtrlCreateTreeViewItem($board[$i], $TreeView1)
			If IniRead(@ScriptDir & '\forum\' & $config, $board[$i], "checked", "0") = 1 Then
				GUICtrlSetState(-1, $GUI_CHECKED)
			EndIf
		Next
	EndIf
EndFunc   ;==>LoadSite

Func SaveSite()
	Local $Name = GUICtrlRead($CFG_NAME)
	If $Name = '' Then Return
	IniWrite('site.ini', $Name, 'URL', GUICtrlRead($GUI_URL))
	IniWrite('site.ini', $Name, 'config', GUICtrlRead($GUI_CFG))
	IniWrite('site.ini', $Name, 'password', GUICtrlRead($GUI_PASS))
	IniWrite('site.ini', $Name, 'username', GUICtrlRead($GUI_USER))
	IniWrite('site.ini', $Name, 'version', GUICtrlRead($GUI_VER))
	IniWrite('site.ini', $Name, 'POST', GUICtrlRead($GUI_POST))
	MsgBox(32, "", "OK!")
EndFunc   ;==>SaveSite

Func AddSite2List()
	Local $siteini = IniReadSectionNames(@ScriptDir & "\site.ini")
	Local $site = ''
	For $k = 1 To $siteini[0]
		$site = $siteini[$k] & "|" & $site
	Next
	GUICtrlSetData($CFG_NAME, $site)
EndFunc   ;==>AddSite2List

Func AutoLogin()
	Local $url = GUICtrlRead($GUI_URL)
	If $url = '' Then Return
	Local $UserName = GUICtrlRead($GUI_USER)
	If $UserName = '' Then Return
	Local $PassWord = GUICtrlRead($GUI_PASS)
	If $PassWord = '' Then Return
	;用户名密码改成你自己的
	$oIE = _IECreate($url & "/logging.php?action=login&" & _
			"loginfield=username&username=" & $UserName & "&password=" & $PassWord & "&questionid=0&answer=&cookietime=2592000" & _
			"&loginmode=&styleid=&loginsubmit=%CC%E1%BD%BB", 0, 0, 0)
	Sleep(3000)
	_IEQuit($oIE)
EndFunc   ;==>AutoLogin

Func URL()
	Local $url = GUICtrlRead($GUI_URL)
	Return $url
EndFunc   ;==>URL

Func getBBSinfo()
	ToolTip("正在得到信息,不要关闭IE窗口.", 0, 0)
	Local $CFG = GUICtrlRead($GUI_CFG)
	Local $url = GUICtrlRead($GUI_URL)
	Local $Name=GUICtrlRead($CFG_NAME)
	If $CFG = '' Or $url = '' Or $CFG_NAME Then 
		ToolTip("")
		Return
	EndIf
;~ 	Local $PostUrl=GUICtrlRead($GUI_POST)
	_GUICtrlTreeView_DeleteAll($TreeView1)
	$oIE = _IECreate($url, 1, 1, 1, 1) ;MTV
	$src = _IEBodyReadHTML($oIE)
	$mini = _StringBetween($src, '<H2><A href="', '</H2>')
	FileDelete(@ScriptDir & '\forum\' & $CFG)
	For $j = 0 To UBound($mini) - 1
		$urla = StringTrimRight($mini[$j], (StringLen($mini[$j]) - StringInStr($mini[$j], '">')) + 1)
		$urlb = StringTrimRight($mini[$j], (StringLen($mini[$j]) - StringInStr($mini[$j], '</A>')) + 1)
		$urlb = StringTrimLeft($urlb, StringInStr($mini[$j], '">') + 1)
		GUICtrlCreateTreeViewItem($urlb, $TreeView1)
		IniWrite(@ScriptDir & '\forum\' & $CFG, $urlb, "URL", $urla)
	Next
	_IEQuit($oIE)
	ToolTip("")
EndFunc   ;==>getBBSinfo


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
			$reg = ''
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

Func RegisterBbs()
	Local $url = GUICtrlRead($GUI_URL)
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
		MsgBox(32, "错误", "很明显,这个弱智的程序,找不到注册地址.", 5)
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
		$url = GUICtrlRead($CFG_NAME)
		FileMove(@ScriptDir & "\site\" & $url, @ScriptDir & "\X\" & $url, 1)
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

Func POST()
	Local $Name = GUICtrlRead($CFG_NAME)
	Local $CFG = GUICtrlRead($GUI_CFG)
	Local $url = GUICtrlRead($GUI_URL)
	Local $PostUrl = GUICtrlRead($GUI_POST)
	If $PostUrl = '' Then $PostUrl = 'post.php'
	
	GUISetState(@SW_HIDE, $Main)
	Local $CFG = GUICtrlRead($GUI_CFG)
	Local $count = _GUICtrlTreeView_GetCount($TreeView1)
	Local $first = _GUICtrlTreeView_GetFirstItem($TreeView1)
	Local $TreeViewText=_GUICtrlTreeView_GetText($TreeView1, $first)
	If _GUICtrlTreeView_GetChecked($TreeView1, $first) Then
		
		IniWrite(@ScriptDir & '\' & $CFG,$TreeViewText, 'checked', 1)
		RunWait("post.exe " & $Name & ' ' & $CFG & ' ' & $URL & ' ' & $PostUrl & ' ' & $TreeViewText)
;~ 		If postfile($TreeViewText)=0 Then
;~ 			GUISetState(@SW_SHOW, $Main)
;~ 			Return
;~ 		EndIf
	Else
		IniWrite(@ScriptDir & '\' & $CFG, $TreeViewText, 'checked', 0)
	EndIf
	While 1
		$first = _GUICtrlTreeView_GetNext($TreeView1, $first)
		If $first = 0 Then
			ExitLoop
		Else
			$TreeViewText=_GUICtrlTreeView_GetText($TreeView1, $first)
			If _GUICtrlTreeView_GetChecked($TreeView1, $first) Then
				
				IniWrite(@ScriptDir & '\' & $CFG, $TreeViewText, 'checked', 1)
;~ 				If postfile($TreeViewText) =0 Then 
;~ 					GUISetState(@SW_SHOW, $Main)
;~ 					Return
;~ 				EndIf
				RunWait("post.exe " & $Name & ' ' & $CFG & ' ' & $URL & ' ' & $PostUrl & ' ' & $TreeViewText)
			Else
				IniWrite(@ScriptDir & '\' & $CFG, $TreeViewText, 'checked', 0)
			EndIf
		EndIf
	WEnd
	GUISetState(@SW_SHOW, $Main)
	MsgBox(32, "", "所有已选版块已经发帖.")
EndFunc   ;==>Getchecked
Func postfile($BOARD)
	;创建对象
	Local $Name = GUICtrlRead($CFG_NAME)
	Local $CFG = GUICtrlRead($GUI_CFG)
	Local $url = GUICtrlRead($GUI_URL)
	Local $PostUrl = GUICtrlRead($GUI_POST)
	If $PostUrl = '' Then $PostUrl = 'post.php'
	Local $PostUrlFid = IniRead(@ScriptDir & '\forum\' & $CFG, $BOARD, "URL", '')
	If $PostUrlFid = '' Then
		MsgBox(32, "", "请先得到论坛版块信息")
		Return
	EndIf
	$PostUrlFid = StringReplace($PostUrlFid, 'forumdisplay.php?fid=', '')
	GUICtrlSetData($Label9, StringInStr($PostUrl, '.htm'))
	If Not IsInt($PostUrlFid) Then
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
	If IsObj($oIE) Then MsgBox(32,0,1)
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
		FileWriteLine("log.txt", "没能在(" & $Name & ")发布一篇名为(" & $PostTitle & ')的文章');	' & date())
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