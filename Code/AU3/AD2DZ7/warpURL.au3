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


$file = FileOpen("URL.txt", 0)

; 检查打开的文件是否可读
If $file = -1 Then
    MsgBox(0, "错误", "不能打开文件.")
    Exit
EndIf

; 每次读取一行文本,直到文件结束.
While 1
    $line = FileReadLine($file)
    If @error = -1 Then ExitLoop
	$line='http://' & $line
	$line=StringTrimRight($line,(StringLen($line) -  StringInStr($line,'/',0,-1)))
	$line=StringReplace($line,'archiver/','')
    ;MsgBox(0, "读取的行:", $line)
	If StringInStr($line,'5d6d') Then ContinueLoop
	FileWriteLine("xxxxxxxxxxx.txt",$line)
Wend

FileClose($file)