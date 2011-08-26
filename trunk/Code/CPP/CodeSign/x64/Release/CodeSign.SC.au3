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
#AutoIt3Wrapper_UseX64=y
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
;-1
;文件已签名,并且有效.
;-2
;文件未签名
;-3
;签名无效或者打开文件错误
;-4
;签名倒是有,但是你没权限去验证
;-5
;签名没问题,但是颁发机构不受信任
;-6
;签名损坏
;-7
;其它错误
;~ $xxx=DllCall('CodeSign.dll','int:cdecl','fnCodeSign','wstr','C:\Program Files (x86)\VMware\VMware Workstation\vmware.exe')


;-1
;签名错误,原因可能是文件不存在,文件没有签名
;-2
;无法得到签名
;-3
;无法给签名者信息分配内存.
;-4
;不能得到签名者信息.
;-5
;不能得到证书信息.
;-6
;得到证书项目字符串发生错误.

$xxx=DllCall('CodeSign.dll','wstr:cdecl','fnSignWhois','wstr','C:\Program Files (x86)\VMware\VMware Workstation\vmware.exe')
MsgBox(32,"",$xxx[0])