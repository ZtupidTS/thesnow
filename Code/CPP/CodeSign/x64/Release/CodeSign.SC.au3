#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_Icon= 										;ͼ��,֧��EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_OutFile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= 								;ע��
#AutoIt3Wrapper_Res_Description=							;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=							;�ļ��汾
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright= 						;��Ȩ
#AutoIt3Wrapper_UseX64=y
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
;-1
;�ļ���ǩ��,������Ч.
;-2
;�ļ�δǩ��
;-3
;ǩ����Ч���ߴ��ļ�����
;-4
;ǩ��������,������ûȨ��ȥ��֤
;-5
;ǩ��û����,���ǰ䷢������������
;-6
;ǩ����
;-7
;��������
;~ $xxx=DllCall('CodeSign.dll','int:cdecl','fnCodeSign','wstr','C:\Program Files (x86)\VMware\VMware Workstation\vmware.exe')


;-1
;ǩ������,ԭ��������ļ�������,�ļ�û��ǩ��
;-2
;�޷��õ�ǩ��
;-3
;�޷���ǩ������Ϣ�����ڴ�.
;-4
;���ܵõ�ǩ������Ϣ.
;-5
;���ܵõ�֤����Ϣ.
;-6
;�õ�֤����Ŀ�ַ�����������.

$xxx=DllCall('CodeSign.dll','wstr:cdecl','fnSignWhois','wstr','C:\Program Files (x86)\VMware\VMware Workstation\vmware.exe')
MsgBox(32,"",$xxx[0])