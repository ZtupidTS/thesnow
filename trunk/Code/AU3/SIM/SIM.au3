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

#include <ButtonConstants.au3>
#include <GUIConstantsEx.au3>
#include <ListViewConstants.au3>
#include <StaticConstants.au3>
#include <TabConstants.au3>
#include <WindowsConstants.au3>
#Region ### START Koda GUI section ### Form=
$Form1 = GUICreate("ACN Install Manager", 635, 427, 344, 122)
$LogoPic = GUICtrlCreatePic("", 0, 0, 635, 70, BitOR($SS_NOTIFY, $WS_GROUP, $WS_CLIPSIBLINGS))
GUICtrlSetImage ($LogoPic,@ScriptDir & "\logo.jpg")
$Tab1 = GUICtrlCreateTab(0, 72, 530, 345)
GUICtrlSetResizing(-1, $GUI_DOCKWIDTH + $GUI_DOCKHEIGHT)
$TabSheet1 = GUICtrlCreateTabItem("���00")
$ListView1 = GUICtrlCreateListView("����:      |�汾|ģ���С", 16, 112, 505, 161, BitOR($LVS_REPORT, $LVS_SHOWSELALWAYS), BitOR($WS_EX_CLIENTEDGE, $LVS_EX_CHECKBOXES,$LVS_EX_FULLROWSELECT,$LVS_SMALLICON))
GUICtrlSetFont(-1, 9, 400, 0, "����")
$ListView1_0 = GUICtrlCreateListViewItem("Winamp|1.0|333 kb", $ListView1)
$ListView1_0 = GUICtrlCreateListViewItem("test|2.0|333 mb", $ListView1)
$TabSheet2 = GUICtrlCreateTabItem("���01")
$TabSheet3 = GUICtrlCreateTabItem("���02")
GUICtrlCreateTabItem("")
$Group1 = GUICtrlCreateGroup("��ϸ��Ϣ:", 16, 296, 500, 105)
;~ GUICtrlSetFont(-1, 9, 400, 0, "����")
GUICtrlCreateGroup("", -99, -99, 1, 1)
$Button1 = GUICtrlCreateButton("��װ", 538, 110, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "����")
$Button2 = GUICtrlCreateButton("����", 538, 142, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "����")
$Button3 = GUICtrlCreateButton("ȫѡ", 538, 175, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "����")
$Button4 = GUICtrlCreateButton("��ѡ", 538, 208, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "����")
$Button5 = GUICtrlCreateButton("�˳�", 538, 241, 89, 25)
;~ GUICtrlSetFont(-1, 9, 400, 0, "����")
GUISetState(@SW_SHOW)
#EndRegion ### END Koda GUI section ###

While 1
	$nMsg = GUIGetMsg()
	Switch $nMsg
		Case $GUI_EVENT_CLOSE
			Exit

	EndSwitch
WEnd


;~ ;������:
;~ /install ��װ
;~ /about	����
;~ /setting	����
;~ 