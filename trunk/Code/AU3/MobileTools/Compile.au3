#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon= 										;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=a3x							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment= thesnoW						;注释
#AutoIt3Wrapper_Res_Description=							;详细信息
#AutoIt3Wrapper_Res_Fileversion=0.0.0.0
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright= thesnow						;版权
#AutoIt3Wrapper_Change2CUI=y                   				;修改输出的程序为CUI(控制台程序)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;自定义资源段
;#AutoIt3Wrapper_Run_Tidy=                   				;脚本整理
;#AutoIt3Wrapper_Run_Obfuscator=      						;代码迷惑
;#AutoIt3Wrapper_Run_AU3Check= 								;语法检查
;#AutoIt3Wrapper_Run_Before= 								;运行前
#AutoIt3Wrapper_Run_After=del %scriptdir%\%scriptfile%.tmk
#AutoIt3Wrapper_Run_After=ren %scriptdir%\%scriptfile%.a3x %scriptfile%.tmk
#EndRegion AutoIt3Wrapper 预编译参数设置完成
#cs ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿

 Au3 版本:
 脚本作者: 
	Email: 
	QQ/TM: 
 脚本版本: 
 脚本功能: 

#ce ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿脚本开始＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
#NoTrayIcon
ConsoleWrite('检查目录存在...')
DirCreate(@ScriptDir & "\Tools")
ConsoleWrite('y' & @CRLF)
ConsoleWrite('检查  AIRPLAY.e@e  存在...')
If FileExists(@ScriptDir & "\Tools\AIRPLAY.e@e") Then
	ConsoleWrite('y' & @CRLF)
Else
	ConsoleWrite('n' & @CRLF)
	ConsoleWrite('下载  AIRPLAY.e@e ...')
	Download('http://zion.podez.com/AIRPLAY.exe',@ScriptDir & "\Tools\AIRPLAY.e@e")
EndIf
;http://www.cpuid.com/download/cpuz/cpuz_152.zip
;dialupass.e@e
;empty.exe
;uTorrent.e@e

Func Download($s_url,$s_file)
	InetGet($s_url,$s_file,1)
;~ 	Local $hDownload =InetGet($s_url,$s_file,1,1)
;~ 	While InetGetInfo($hDownload, 2)    ; 检查下载是否完成.
;~ 		$aData = InetGetInfo($hDownload)
;~ 		ToolTip("Download File:" & $aData[0] & '/' & $aData[1],0,0)
;~ 	WEnd
;~ 	ToolTip('')
;~ 	InetClose($hDownload)   ; 关闭句柄,释放资源.
	
EndFunc