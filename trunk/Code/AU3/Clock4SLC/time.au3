#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_UseAnsi=N									;����
#AutoIt3Wrapper_Icon= nis										;ͼ��
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_OutFile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= ��С���������								;ע��
#AutoIt3Wrapper_Res_Description=	��С���������						;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=1.0
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright=������ 						;��Ȩ
#AutoIt3Wrapper_Change2CUI=N                   				;�޸�����ĳ���ΪCUI(����̨����)
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

Opt('TrayMenuMode',1)
Local $H,$M,$S
$g_szVersion = "mytime__"
If WinExists($g_szVersion) Then Exit ; �˽ű��Ѿ�������
AutoItWinSetTitle($g_szVersion)
TraySetToolTip('�������С������������...')
$H=IniRead(@ScriptDir & '\' & @ScriptName & '.ini','time','H','6')
$M=IniRead(@ScriptDir & '\' & @ScriptName & '.ini','time','M','30')
$S=IniRead(@ScriptDir & '\' & @ScriptName & '.ini','time','SOUND','SOUND.MP3')

While 1
		If int($H)=int(@HOUR) And int($M)=Int(@MIN) Then
			SoundPlay($S,0)
			While 1
				if MsgBox(36,'','���[��/NO]�˳���')<>6 Then	Exit
			WEnd
		EndIf	
Sleep(1000)		
WEnd