#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_Icon=nis 										;ͼ��,֧��EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_OutFile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment=����IE��ǩ(�ղؼ�)ͼ�� 								;ע��
#AutoIt3Wrapper_Res_Description=����IE��ǩ(�ղؼ�)ͼ��							;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=0.0.0.1							;�ļ��汾
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright=thesnoW 						;��Ȩ
#AutoIt3Wrapper_Change2CUI=y                   				;�޸�����ĳ���ΪCUI(����̨����)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;�Զ�����Դ��
;#AutoIt3Wrapper_Run_Tidy=                   				;�ű�����
;#AutoIt3Wrapper_Run_Obfuscator=      						;�����Ի�
;#AutoIt3Wrapper_Run_AU3Check= 								;�﷨���
;#AutoIt3Wrapper_Run_Before= 								;����ǰ
;#AutoIt3Wrapper_Run_After=									;���к�
#EndRegion AutoIt3Wrapper Ԥ��������������
#cs �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�

 Au3 �汾:
 �ű�����: 
	Email: 
	QQ/TM: 
 �ű��汾: 
 �ű�����: 

#ce �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣ߽ű���ʼ�ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
#Include <Array.au3>
#NoTrayIcon

Global $BookMarkFile[1]
LOOP(@FavoritesDir)
Global $UrlTotal=UBound($BookMarkFile)-1
Global $UrlNow=0
AdlibRegister('Now')
For $i = 1 To $UrlTotal
	DllCall("Kernel32.dll","bool","SetConsoleTitleW","wstr","���ڼ���[" & $i & "/" & $UrlTotal & "]����ǩ")
	$UrlNow=$i
	CheckIcon($BookMarkFile[$i])
Next

Func LOOP($dir)
	Local $file
	; ��ʾ��ǰĿ¼�������ļ����ļ���
	$search = FileFindFirstFile($dir & "\*.*")  
	; ��������Ƿ�ɹ�
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
	; �ر��������
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