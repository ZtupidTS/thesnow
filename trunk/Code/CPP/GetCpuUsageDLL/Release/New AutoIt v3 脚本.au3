#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_Icon= 										;ͼ��,֧��EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_OutFile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= 								;ע��
#AutoIt3Wrapper_Res_Description=							;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=3.3.6.4
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright= thesnoW						;��Ȩ
#AutoIt3Wrapper_Change2CUI=y                   				;�޸�����ĳ���ΪCUI(����̨����)
#AutoIt3Wrapper_UseX64=n
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
;PS:�κγ���ʼ����ʱ������CPUռ��,���ڳ���������ɺ��ټ��CPUռ��.����������,��ȡƽ��ֵ.
;GetCpuUsage_NoVCCRT.dll Ϊ��̬VC����ʱ��DLL,���ô�DLL����ϵͳ��װVC���п�
;��ʱ�����㷢��64λDLL
;�����Ч��...

;����1,��������̨��ͼ(�ȱ���)
DllCall('GetCpuUsage.dll','int:cdecl','DrawCpuUsage','int',0)

;����2,�õ�CPUռ����
;~ $x=DllCall('GetCpuUsage.dll','int:cdecl','GetCpuUsage','int',0)
;~ ConsoleWrite($x[0] & @CRLF)

;~ ;����3,�õ���һ��(���ߵ�һ������)CPU��ռ����
;~ $x=DllCall('GetCpuUsage.dll','int:cdecl','GetCpuUsage','int',1)
;~ ConsoleWrite($x[0] & @CRLF)
