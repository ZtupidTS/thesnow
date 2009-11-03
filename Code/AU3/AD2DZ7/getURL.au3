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
#Include <String.au3>
#include <IE.au3>
$page=InputBox("输入一个数字","输入页码","0")
$pages=($page +1 ) * 10
For $x= 1 To 100
	ToolTip("当前页:" & ($pages /10),0,0)
	$oIE = _IECreate ('http://www.google.cn/search?hl=zh-CN&q=%E6%B8%B8%E6%88%8F+%22powered+by+discuz%22&start=' & $pages & '&sa=N')
	$text=_IEDocReadHTML($oIE)
	_IEQuit($oIE)
	;FileDelete("temp")
	$url=_StringBetween($text,'<cite>','</cite>')
	If IsArray($url) Then
		For $i = 1 To UBound($url)-1
			FileWriteLine("URL.txt",$url[$i])
		Next
	Else
		MsgBox(32,0,0)
	EndIf
	$page+=1
	$pages=($page +1 ) * 10
	Sleep(3000)
Next