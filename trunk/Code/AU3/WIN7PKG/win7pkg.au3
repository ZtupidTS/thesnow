#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_Icon= 										;ͼ��,֧��EXE,DLL,ICO
#AutoIt3Wrapper_Outfile=									;����ļ���
#AutoIt3Wrapper_Outfile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= 								;ע��
#AutoIt3Wrapper_Res_Description=							;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=							;�ļ��汾
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;�Զ����°汾
#AutoIt3Wrapper_Res_LegalCopyright= 						;��Ȩ
#AutoIt3Wrapper_Change2CUI=N                   				;�޸�����ĳ���ΪCUI(����̨����)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;�Զ�����Դ��
;#AutoIt3Wrapper_Run_Tidy=                   				;�ű�����
;#AutoIt3Wrapper_Run_Obfuscator=      						;�����Ի�
;#AutoIt3Wrapper_Run_AU3Check= 								;�﷨���
;#AutoIt3Wrapper_Run_Before= 								;����ǰ
;#AutoIt3Wrapper_Run_After=									;���к�
#EndRegion AutoIt3Wrapper Ԥ�������(���ò���)
#cs �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�
	
	Au3 �汾:
	�ű�����:
	Email:
	QQ/TM:
	�ű��汾:
	�ű�����:
	
#ce �ߣߣߣߣߣߣߣߣߣߣߣߣߣߣ߽ű���ʼ�ߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣߣ�

;HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Component Based Servicing\Packages
#include <ButtonConstants.au3>
#include <GUIConstantsEx.au3>
#include <StaticConstants.au3>
#include <TabConstants.au3>
#include <TreeViewConstants.au3>
#include <WindowsConstants.au3>
#Region ### START Koda GUI section ### Form=E:\WorkSpace\SVN\SVNthesnoW\trunk\Code\AU3\WIN7PKG\Form1.kxf
$Form1 = GUICreate("WIN7PKG", 786, 545, 197, 133)
$Tab1 = GUICtrlCreateTab(8, 8, 769, 529)
GUICtrlSetResizing(-1, $GUI_DOCKWIDTH + $GUI_DOCKHEIGHT)
$TabSheet1 = GUICtrlCreateTabItem("WIN7PKG")
$PkgList = GUICtrlCreateTreeView(16, 40, 505, 489, BitOR($TVS_HASBUTTONS, $TVS_HASLINES, $TVS_LINESATROOT, $TVS_DISABLEDRAGDROP, $TVS_SHOWSELALWAYS, $TVS_CHECKBOXES, $WS_GROUP, $WS_TABSTOP), $WS_EX_STATICEDGE)
$Group1 = GUICtrlCreateGroup("Group1", 528, 40, 233, 489)
$Button1 = GUICtrlCreateButton("ж�ذ�", 648, 456, 89, 25)
$Button2 = GUICtrlCreateButton("ɾ����", 536, 496, 81, 25)
$Button3 = GUICtrlCreateButton("ж�ز�ɾ����", 648, 496, 89, 25)
$Button4 = GUICtrlCreateButton("��װ��", 537, 456, 81, 25)
$Label1 = GUICtrlCreateLabel("������:", 536, 56, 43, 17)
$Label2 = GUICtrlCreateLabel("Y", 592, 56, 11, 17)
$Label3 = GUICtrlCreateLabel("����С:", 536, 80, 43, 17)
$Label4 = GUICtrlCreateLabel("xxxxxMb", 592, 80, 44, 17)
GUICtrlCreateGroup("", -99, -99, 1, 1)
$TabSheet2 = GUICtrlCreateTabItem("DrvPKG")
$TabSheet3 = GUICtrlCreateTabItem("About")
GUICtrlCreateTabItem("")
GUISetState(@SW_SHOW)
#EndRegion ### END Koda GUI section ###

While 1
	$nMsg = GUIGetMsg()
	Switch $nMsg
		Case $GUI_EVENT_CLOSE
			Exit

	EndSwitch
WEnd