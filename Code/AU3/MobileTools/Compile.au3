#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_Icon= 										;ͼ��,֧��EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_OutFile_Type=a3x							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= thesnoW						;ע��
#AutoIt3Wrapper_Res_Description=							;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=0.0.0.0
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright= thesnow						;��Ȩ
#AutoIt3Wrapper_Change2CUI=y                   				;�޸�����ĳ���ΪCUI(����̨����)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;�Զ�����Դ��
;#AutoIt3Wrapper_Run_Tidy=                   				;�ű�����
;#AutoIt3Wrapper_Run_Obfuscator=      						;�����Ի�
;#AutoIt3Wrapper_Run_AU3Check= 								;�﷨���
;#AutoIt3Wrapper_Run_Before= 								;����ǰ
#AutoIt3Wrapper_Run_After=del %scriptdir%\%scriptfile%.tmk
#AutoIt3Wrapper_Run_After=ren %scriptdir%\%scriptfile%.a3x %scriptfile%.tmk
#EndRegion AutoIt3Wrapper Ԥ��������������
#cs �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�

 Au3 �汾:
 �ű�����: 
	Email: 
	QQ/TM: 
 �ű��汾: 
 �ű�����: 

#ce �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣ߽ű���ʼ�ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
#NoTrayIcon
ConsoleWrite('���Ŀ¼����...')
DirCreate(@ScriptDir & "\Tools")
ConsoleWrite('y' & @CRLF)
ConsoleWrite('���  AIRPLAY.e@e  ����...')
If FileExists(@ScriptDir & "\Tools\AIRPLAY.e@e") Then
	ConsoleWrite('y' & @CRLF)
Else
	ConsoleWrite('n' & @CRLF)
	ConsoleWrite('����  AIRPLAY.e@e ...')
	Download('http://zion.podez.com/AIRPLAY.exe',@ScriptDir & "\Tools\AIRPLAY.e@e")
EndIf
;http://www.cpuid.com/download/cpuz/cpuz_152.zip
;dialupass.e@e
;empty.exe
;uTorrent.e@e

Func Download($s_url,$s_file)
	InetGet($s_url,$s_file,1)
;~ 	Local $hDownload =InetGet($s_url,$s_file,1,1)
;~ 	While InetGetInfo($hDownload, 2)    ; ��������Ƿ����.
;~ 		$aData = InetGetInfo($hDownload)
;~ 		ToolTip("Download File:" & $aData[0] & '/' & $aData[1],0,0)
;~ 	WEnd
;~ 	ToolTip('')
;~ 	InetClose($hDownload)   ; �رվ��,�ͷ���Դ.
	
EndFunc