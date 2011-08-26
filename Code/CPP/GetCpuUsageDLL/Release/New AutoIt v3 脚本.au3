#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon= 										;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment= 								;注释
#AutoIt3Wrapper_Res_Description=							;详细信息
#AutoIt3Wrapper_Res_Fileversion=3.3.6.4
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright= thesnoW						;版权
#AutoIt3Wrapper_Change2CUI=y                   				;修改输出的程序为CUI(控制台程序)
#AutoIt3Wrapper_UseX64=n
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
;PS:任何程序开始运行时将增加CPU占用,请在程序启动完成后再检测CPU占用.请连续运行,求取平均值.
;GetCpuUsage_NoVCCRT.dll 为静态VC运行时的DLL,调用此DLL无需系统安装VC运行库
;暂时不打算发布64位DLL
;编译后看效果...

;例子1,连续控制台绘图(先编译)
DllCall('GetCpuUsage.dll','int:cdecl','DrawCpuUsage','int',0)

;例子2,得到CPU占用率
;~ $x=DllCall('GetCpuUsage.dll','int:cdecl','GetCpuUsage','int',0)
;~ ConsoleWrite($x[0] & @CRLF)

;~ ;例子3,得到第一颗(或者第一个核心)CPU的占用率
;~ $x=DllCall('GetCpuUsage.dll','int:cdecl','GetCpuUsage','int',1)
;~ ConsoleWrite($x[0] & @CRLF)
