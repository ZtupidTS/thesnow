#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon=nis 										;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment=更新IE书签(收藏夹)图标 								;注释
#AutoIt3Wrapper_Res_Description=更新IE书签(收藏夹)图标							;详细信息
#AutoIt3Wrapper_Res_Fileversion=0.0.0.1							;文件版本
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright=thesnoW 						;版权
#AutoIt3Wrapper_Change2CUI=y                   				;修改输出的程序为CUI(控制台程序)
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
#Include <Array.au3>
#NoTrayIcon

Global $BookMarkFile[1]
LOOP(@FavoritesDir)
Global $UrlTotal=UBound($BookMarkFile)-1
Global $UrlNow=0
AdlibRegister('Now')
For $i = 1 To $UrlTotal
	DllCall("Kernel32.dll","bool","SetConsoleTitleW","wstr","正在检查第[" & $i & "/" & $UrlTotal & "]个书签")
	$UrlNow=$i
	CheckIcon($BookMarkFile[$i])
Next

Func LOOP($dir)
	Local $file
	; 显示当前目录中所有文件的文件名
	$search = FileFindFirstFile($dir & "\*.*")  
	; 检查搜索是否成功
	If $search = -1 Then Return
	While 1
		$file = FileFindNextFile($search) 
		If @error Then ExitLoop
		If @extended Then 
			LOOP($dir & "\" & $file)
		Else
			_ArrayAdd($BookMarkFile,$dir & "\" & $file)
		EndIf
	WEnd
	; 关闭搜索句柄
	FileClose($search)
EndFunc

Func CheckIcon($BookMark)
	Local $url=IniRead($BookMark,'InternetShortcut','url','')
	Local $url2=''
	If $url='' Or StringInStr($url,'ftp:') Or StringInStr($url,'javascript:') Then Return
;~ 	Local $IconFile=IniRead($BookMark,'InternetShortcut','IconFile','')
	If StringInStr($url,'//') Then
		$url2=StringTrimRight($url,StringLen($url)-StringInStr($url,'/',Default,3))
	Else
		$url2=StringTrimRight($url,StringLen($url)-StringInStr($url,'/'))
	EndIf
	If $url2='' Then 
		$url2=$url & "/favicon.ico"
	Else
		$url2&= "favicon.ico"
	EndIf
;~ 	ConsoleWrite($url2 & @CRLF)
	If InetGet($url2,@TempDir & "\temp.ico",1) >= 100 Then
		FileDelete(@TempDir & "\temp.ico")
		IniWrite($BookMark,'InternetShortcut','IconFile',$url2)
		IniWrite($BookMark,'InternetShortcut','IconIndex',1)
	EndIf
EndFunc

Func Now()
	For $k= 1 To 210
		ConsoleWrite(ChrW(0x8))
	Next
		ConsoleWrite('[')
	For $j = 1 To $UrlNow/$UrlTotal*50
		ConsoleWrite('*')
	Next
	ConsoleWrite('>')
	For $j = 1 To ($UrlTotal-$UrlNow)/$UrlTotal*50
		ConsoleWrite('#')
	Next
		ConsoleWrite(']	' & Round($UrlNow/$UrlTotal*100,3) & ' %')
EndFunc