#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon=nis
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_UseX64=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment=NTP 								;注释
#AutoIt3Wrapper_Res_Description=thesnoW							;详细信息
#AutoIt3Wrapper_Res_Fileversion=1.1							;文件版本
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright= thesnoW						;版权
#AutoIt3Wrapper_Change2CUI=N                   				;修改输出的程序为CUI(控制台程序)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;自定义资源段
;#AutoIt3Wrapper_Run_Tidy=                   				;脚本整理
;#AutoIt3Wrapper_Run_Obfuscator=      						;代码迷惑
;#AutoIt3Wrapper_Run_AU3Check= 								;语法检查
;#AutoIt3Wrapper_Run_Before= 								;运行前
;#AutoIt3Wrapper_Run_After=									;运行后
#EndRegion AutoIt3Wrapper 预编译参数设置完成
#include <Date.au3>
#NoTrayIcon
Global $ntpServer = "time.buaa.edu.cn" ; NTP server
Global $socket,$status,$data
UDPStartup()
$socket = UDPOpen(TCPNameToIP($ntpServer), 123)
If @error <> 0 Then
    MsgBox(0,"","Can't open connection!")
    Exit
EndIf
$status = UDPSend($socket, MakePacket("1b0e01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"))
If $status = 0 Then
    MsgBox(0, "ERROR", "Error while sending UDP message: " & @error)
    Exit
EndIf
While $data=""
    $data = UDPRecv($socket, 100)
    sleep(100)
WEnd
UDPCloseSocket($socket)
UDPShutdown()

$unsignedHexValue=StringMid($data,83,8); Extract time from packet. Disregards the fractional second.
$value=UnsignedHexToDec($unsignedHexValue)
$TZinfo = _Date_Time_GetTimeZoneInformation()
$TZoffset=$TZinfo[1]*-1
$UTC=_DateAdd("s",$value,"1900/01/01 00:00:00")

;~ Extracts the data & time into vars
;~ Date format & offsets
;~ 2009/12/31 19:26:05
;~ 1234567890123456789  [1 is start of string]

$m = StringMid ($UTC,6,2)
$d = StringMid ($UTC,9,2)
$y = StringMid ($UTC,1,4)
$h = StringMid ($UTC,12,2)
$mi = StringMid ($UTC,15,2)
$s = StringMid ($UTC,18,2)

;~ Sets the new current time to the computer
$tCurr = _Date_Time_EncodeSystemTime($m,$d,$y,$h,$mi,$s)
_Date_Time_SetSystemTime(DllStructGetPtr($tCurr))


;**************************************************************************************************
;** Fuctions **************************************************************************************
;**************************************************************************************************
Func MakePacket($d)
    Local $p=""
    While $d
        $p&=Chr(Dec(StringLeft($d,2)))
        $d=StringTrimLeft($d,2)
    WEnd
    Return $p
EndFunc
;**************************************************************************************************
Func UnsignedHexToDec($n)
   Local $ones=StringRight($n,1)
    $n=StringTrimRight($n,1)
    Return dec($n)*16+dec($ones)
EndFunc
;**************************************************************************************************