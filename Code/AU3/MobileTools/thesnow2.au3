#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_Icon= NIS.ico								;ͼ��
#AutoIt3Wrapper_Outfile=..\..\thesnow.exe					;����ļ���
#AutoIt3Wrapper_Outfile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=n 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= Mobile Tools					;ע��
#AutoIt3Wrapper_Res_Description=Mobile Tools				;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=2.0.0.2
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=n				;�Զ����°汾
#AutoIt3Wrapper_Res_LegalCopyright= thesnow					;��Ȩ
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;�Զ�����Դ��
;#AutoIt3Wrapper_Run_Tidy=                   				;�ű�����
;#AutoIt3Wrapper_Run_Obfuscator=      						;�����Ի�
;#AutoIt3Wrapper_Run_AU3Check= 								;�﷨���
#AutoIt3Wrapper_Run_Before=%autoitdir%\autoit3.exe %scriptdir%\compile.tmk
;#AutoIt3Wrapper_Run_After=									;���к�
#EndRegion AutoIt3Wrapper Ԥ�������(���ò���)
#cs �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
	
	AutoIt �汾: 3.3.1.1
	�ű�����:thesnow
	Email:rundll32@126.com
	QQ/TM:133333542
	�ű��汾:
	�ű�����:
	
#ce �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣ߽ű���ʼ�ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
#Region setting/var and include
#NoTrayIcon
#RequireAdmin
If Not @Compiled Then Exit
Global $ProgramIni 		= @ScriptFullPath & ".ini"						;����INI�����ļ�����
Global $MobileDevice	= IniRead(@ScriptName & ".ini","SystemSet",'%DRV%','Z:')	;�����̷�(������ܲ�������̷�)
Global $ReleaseDir 		= @HomeDrive & "\MobileTools"					;�ͷ�Ŀ¼
Global $HideWinHwnd														;ȫ�ִ��ھ��,�˵�ģʽ����(��Ҫ����/��ʾ�Ĵ���)
Global $TrayToolTips 	= "thesnow's Mobile Tools"						;�˵�ģʽ,����ͼ����ʾ���ı�
Global $ProgramVersion 	= FileGetVersion(@ScriptFullPath) 				;����汾
#include <Date.au3>  													;�������ں�����
#include <GDIPlus.au3>													;����GDI��
#include "Func.au3"
#include "TrayMode.au3"
If StringLen($MobileDevice) <> 2 Then
	MsgBox(32,"�̷��������","��ʹ��[ Z: ]�����ĸ�ʽ.�̷��Զ�����Ϊ[ Z: ]")
	$MobileDevice="Z:"
EndIf
#EndRegion end of setting/var and include

#Region command line process
If $CmdLine[0] = 0 Then 													;�����������0(ֱ������ʱ)
	If Not FileExists($MobileDevice & '\' & @ScriptName) Then 				;���������δ���ƶ��豸��Ŀ¼
		If StringInStr(@WindowsDir, StringLeft(@ScriptDir, 2)) Or _
			StringLeft(@ScriptDir, 2) = 'c:' Or _
			StringLeft(@ScriptDir, 3) = '"c:' Then
			;�������ж��ǲ���ϵͳ�̺�C��
			MsgBox(16, "����", "������ע��,�ѵ������ϵͳ�̸�Ϊ" & $MobileDevice & "��ô?")
			Exit
		EndIf
		DirCreate($ReleaseDir)												;�����ͷ�Ŀ¼
		FileSetAttrib($ReleaseDir,"+H")
		FileCopy(@ScriptFullPath, $ReleaseDir & '\' & @ScriptName, 1) 		;�����Լ����ͷ�Ŀ¼
		FileCopy($ProgramIni, $ReleaseDir & '\' & @ScriptName & ".ini", 1) 	;���������ļ����ͷ�Ŀ¼
		Run($ReleaseDir & "\" & @ScriptName & ' "' & @ScriptFullPath & '"', $ReleaseDir, @SW_HIDE)
		;�����ͷ�Ŀ¼���Լ�,���˲���(�������Լ���ǰ������·��)
		Exit 																;�˳�
	EndIf
	;������ھʹ� setting file process start ��ʼ����.
Else 																		;�����������0(����������ʱ)
	;�������ǲ��Ǳ�����Ľű�,�Ǿ�����.�����漰һ�����ŵ�����,Ҳ����ո��·�����л�������,�Ժ�����.
	If IsString($CmdLineRaw) And ((StringRight($CmdLineRaw, 4) = '.a3x') Or (StringRight($CmdLineRaw, 4) = 'a3x"')) Then
		Run(@AutoItExe & ' /AutoIt3ExecuteScript ' & $CmdLineRaw)
		Exit
	EndIf
	;(��)����ģʽ,Ҳ�������ƶ��豸��ֱ�ӼӵĲ�������.
	If $CmdLineRaw == "Tray" Then
		MsgBox(32, "", "���̲˵�ģʽ.ȷ�������.")
		DirCreate($ReleaseDir)
		FileCopy(@ScriptFullPath, $ReleaseDir & '\' & @ScriptName, 1) 		;�����Լ����ͷ�Ŀ¼
		FileCopy($ProgramIni, $ReleaseDir & '\' & @ScriptName & ".ini", 1) 	;���������ļ����ͷ�Ŀ¼
		Run($ReleaseDir & "\" & @ScriptName & ' TrayMode', $ReleaseDir, @SW_HIDE);������ʱĿ¼���Լ������˲���(ִ�����������ģʽ)
		Exit ;�˳�
	EndIf
	;(ʵ)����ģʽ,���ͷ�Ŀ¼����,��ò�Ҫ���ƶ��豸�ϼ������������.
	If $CmdLineRaw == "TrayMode" Then
		TrayMode()
	EndIf	
	;���UNC·��,�Ǿ��˳�(��ֹ������·��������)
	If StringInStr($CmdLine[1],'\\') Or  StringInStr($CmdLine[1],'//') Then
		MsgBox(32, "", "UNC·����֧��.�����˳�,��ȷ�������ļ��ڿ��ƶ��豸��.")
		Exit
	EndIf
	;���������ƶ��豸�̷�������.(�������ڱ������Ѿ���ϵͳ����)
	If Not FileExists($MobileDevice) Then 
		;����Դ����������һ���ظ�������ҵĵ���.(��������Դ�����������ƶ��豸�̷�,�������Ƿ�ֹ�豸ռ�ã��޷������̷�)
		ControlSend("[CLASS:CabinetWClass]", "", "SysListView321", "{BACKSPACE}")
		Sleep(1000)																		;�޸��̷�ǰ�Ļ�����,1��
		ChangeDriver(StringReplace(StringLeft($CmdLine[1], 3),'"',""), $MobileDevice)	;���̷���Ϊ������ƶ��豸�̷�
		Run($MobileDevice & '\' & @ScriptName, $MobileDevice, @ScriptDir) 				;�����ƶ��豸�϶���ı�����(û�Ӳ���)
		TrayMode()																	;��������
	Else ;���������ƶ��豸�̷��Ѿ�������
		MsgBox(32, "����", $MobileDevice & '���Ѿ�����,�������̷��޸�.�����˳�.') 	;��ʾ��Ϣ
		Exit 																			;�˳�
	EndIf
EndIf
#EndRegion end of command line process

#Region setting file process start--->
Sleep(1000) 											;��ͣ1��
If Not FileExists($ProgramIni) Then 					;���������ƶ��豸�̷���Ŀ¼�²����������ļ�
	MsgBox(32, "�����ļ�������!", '�����ļ�������,�����˳�...')
	Exit 												;�˳�
EndIf
IniRun()												;�����ļ��е���������
IniExecExt()											;��ĳЩ��չ��֧��ֱ������,���ڷ�ֹ������Ⱦ.
IniLocalizedResourceName()								;desktop.ini�ļ������õ�(����).
IniSystemSetting()										;�����ļ��в�������
Favorite(Int(IniRead($ProgramIni, "ToolSet", 'Favorite', 0)))		;�Ƿ�ָ��ղؼ�
If IniRead($ProgramIni, "SystemSet", 'WallPaper', "") <> "" Then	;�޸������ֽ
	_ChangeDesktopWallpaper(PathConv(IniRead($ProgramIni, "SystemSet", 'WallPaper', 0)), 2)
EndIf
_ChangeDesktopIcon()									;�޸�����ͼ����ʽ
#Region 												;�Ƴ������ļ���ָ����������
$RemoveDriver = IniReadSection($ProgramIni, "RemoveDriver")
If Not @error Then
	For $i = 1 To $RemoveDriver[0][0]
		RemoveDriver($RemoveDriver[$i][0])
	Next
EndIf
#EndRegion
ImageFileExecutionOptions()								;�����ļ��ٳ�
RefreshSystem()											;ˢ��ϵͳ;��Ҫ����ԱȨ��
Exit
#EndRegion setting file process end.
