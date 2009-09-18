#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon= 										;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_Outfile=									;输出文件名
#AutoIt3Wrapper_Outfile_Type=exe							;文件类型
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
#EndRegion AutoIt3Wrapper 预编译参数(常用参数)
#cs ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
	
	Au3 版本:
	脚本作者:
	Email:
	QQ/TM:
	脚本版本:
	脚本功能:
	
#ce ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿脚本开始＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿

#include <ButtonConstants.au3>
#include <GUIConstantsEx.au3>
#include <ListViewConstants.au3>
#include <StaticConstants.au3>
#include <TabConstants.au3>
#include <WindowsConstants.au3>
#Region ### START Koda GUI section ### Form=
$Form1 = GUICreate("ACN Install Manager", 635, 427, 344, 122)
$LogoPic = GUICtrlCreatePic("", 0, 0, 635, 70, BitOR($SS_NOTIFY, $WS_GROUP, $WS_CLIPSIBLINGS))
GUICtrlSetImage ($LogoPic,@ScriptDir & "\logo.jpg")
$Tab1 = GUICtrlCreateTab(0, 72, 530, 345)
GUICtrlSetResizing(-1, $GUI_DOCKWIDTH + $GUI_DOCKHEIGHT)
$TabSheet1 = GUICtrlCreateTabItem("软件00")
$ListView1 = GUICtrlCreateListView("名称:      |版本|模块大小", 16, 112, 505, 161, BitOR($LVS_REPORT, $LVS_SHOWSELALWAYS), BitOR($WS_EX_CLIENTEDGE, $LVS_EX_CHECKBOXES,$LVS_EX_FULLROWSELECT,$LVS_SMALLICON))
GUICtrlSetFont(-1, 9, 400, 0, "宋体")
$ListView1_0 = GUICtrlCreateListViewItem("Winamp|1.0|333 kb", $ListView1)
$ListView1_0 = GUICtrlCreateListViewItem("test|2.0|333 mb", $ListView1)
$TabSheet2 = GUICtrlCreateTabItem("软件01")
$TabSheet3 = GUICtrlCreateTabItem("软件02")
GUICtrlCreateTabItem("")
$Group1 = GUICtrlCreateGroup("详细信息:", 16, 296, 500, 105)
;~ GUICtrlSetFont(-1, 9, 400, 0, "宋体")
GUICtrlCreateGroup("", -99, -99, 1, 1)
$Button1 = GUICtrlCreateButton("安装", 538, 110, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "宋体")
$Button2 = GUICtrlCreateButton("设置", 538, 142, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "宋体")
$Button3 = GUICtrlCreateButton("全选", 538, 175, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "宋体")
$Button4 = GUICtrlCreateButton("不选", 538, 208, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "宋体")
$Button5 = GUICtrlCreateButton("退出", 538, 241, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "宋体")
GUISetState(@SW_SHOW)
#EndRegion ### END Koda GUI section ###

While 1
	$nMsg = GUIGetMsg()
	Switch $nMsg
		Case $GUI_EVENT_CLOSE
			Exit

	EndSwitch
WEnd


;~ ;参数表:
;~ /install 安装
;~ /about	关于
;~ /setting	设置
;~ 