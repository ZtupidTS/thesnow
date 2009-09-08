#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_UseAnsi=N									;编码
#AutoIt3Wrapper_Icon= nis										;图标
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment= 给小懒虫的闹铃								;注释
#AutoIt3Wrapper_Res_Description=	给小懒虫的闹铃						;详细信息
#AutoIt3Wrapper_Res_Fileversion=1.0
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright=大懒虫 						;版权
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

Opt('TrayMenuMode',1)
Local $H,$M,$S
$g_szVersion = "mytime__"
If WinExists($g_szVersion) Then Exit ; 此脚本已经运行了
AutoItWinSetTitle($g_szVersion)
TraySetToolTip('大懒虫给小懒虫做的闹钟...')
$H=IniRead(@ScriptDir & '\' & @ScriptName & '.ini','time','H','6')
$M=IniRead(@ScriptDir & '\' & @ScriptName & '.ini','time','M','30')
$S=IniRead(@ScriptDir & '\' & @ScriptName & '.ini','time','SOUND','SOUND.MP3')

While 1
		If int($H)=int(@HOUR) And int($M)=Int(@MIN) Then
			SoundPlay($S,0)
			While 1
				if MsgBox(36,'','点击[否/NO]退出！')<>6 Then	Exit
			WEnd
		EndIf	
Sleep(1000)		
WEnd