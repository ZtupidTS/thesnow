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

Func RegisterBbs($url)
	;$url = URL()
	If IniRead(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "��ע��", 0) = 1 Then
		If MsgBox(36, "", "�ϴεı���˵�Ѿ��ɹ�ע����,�Ƿ�����ע��?") <> 6 Then Return
	EndIf
	$url2 = GetRegistrAddr()
	If Not StringInStr($url2, "http:") Then $url2 = $url & "/" & $url2
	;	$url2 = $url; & "/registerbbs.php"
	$ie = _IECreate($url2, 1, 1, 1, 1) ;MTV
	$oForm = _IEFormGetObjByName($ie, "registerform")
	If Not IsObj($oForm) Then
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "��ע��", 0)
		MsgBox(32, "����", "������,������ǵĳ���,�Ҳ���ע���ַ.",5)
		_IEQuit($ie)
		Return 0
	EndIf
	$oQuery = _IEGetObjById($oForm, "invitecode") ;�û���
	If $oQuery <> 0 Then
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "��ע��", 0)
		MsgBox(32, "����", "������,��Ҫ������.�ﲻ����.")
		_IEQuit($ie)
		Return 0
	EndIf

	$oQuery = _IEGetObjById($oForm, "username") ;�û���
	_IEFormElementSetValue($oQuery, $UserName)
	$oQuery = _IEGetObjById($oForm, "email") ;email
	_IEFormElementSetValue($oQuery, '123@sb23456.com')
	$oQuery = _IEGetObjById($oForm, "password") ;password
	_IEFormElementSetValue($oQuery, $PassWord)
	$oQuery = _IEGetObjById($oForm, "password2") ;password
	_IEFormElementSetValue($oQuery, $PassWord)
	$oQuery = _IEGetObjById($oForm, "seccodeverify") ;��֤
	$oQuery.click()
	_IEFormElementSetValue($oQuery, '')
	If MsgBox(36, "�ܲ���ע��?", "�ǲ���ע��ɹ���?") = 6 Then
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "��ע��", 1)
		_IEQuit($ie)
		getBBSinfo()
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "username", $UserName)
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "password", $PassWord)
		$url = GUICtrlRead($InputURL)
		FileMove(@ScriptDir & "\site\" & $url,@ScriptDir & "\X\" & $url,1)
		Return 1
	Else
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", "forum", "��ע��", 0)
		_IEQuit($ie)
	EndIf
;~ 	$oQuery = _IEGetObjById ($oForm, "registerformsubmit")			;�ύ
;~ 	$oQuery.click()
;~ 	$oQuery = _IEGetObjById ($oForm, "checkseccodeverify")			;У����֤
;~ 	$oQuery = _IEFormElementGetValue ($oQuery)						;
EndFunc   ;==>RegisterBbs


Func GetRegistrAddr()
	ToolTip('�����̳������,��ȴ�.', 0, 0)
	$url = URL()
	$oIE = _IECreate($url, 1, 0, 1, 1) ;MTV
	$src = _IEBodyReadHTML($oIE)
	_IEQuit($oIE)
	$reg = _StringBetween($src, '<A class=lightlink', '</A>')
	If Not IsArray($reg) Then
		$reg = _StringBetween($src, '<li><a href="', '" onclick="')
		If Not IsArray($reg) Then
			$reg=''
		Else
			$reg = $reg[0]
		EndIf
	Else
		$reg = _StringBetween($reg[0], 'href="', '">')
		$reg = $reg[0]
	EndIf
	ToolTip("")
	Return $reg
EndFunc   ;==>GetRegistrAddr

Func getBBSinfo()
	ToolTip("���ڵõ���Ϣ,��Ҫ�ر�IE����.", 0, 0)
	$url = URL()
	$oIE = _IECreate($url, 1, 1, 1, 1) ;MTV
	$src = _IEBodyReadHTML($oIE)
	$mini = _StringBetween($src, '<H2><A href="', '</H2>')
	For $j = 0 To UBound($mini) - 1
		$urla = StringTrimRight($mini[$j], (StringLen($mini[$j]) - StringInStr($mini[$j], '">')) + 1)
		$urlb = StringTrimRight($mini[$j], (StringLen($mini[$j]) - StringInStr($mini[$j], '</A>')) + 1)
		$urlb = StringTrimLeft($urlb, StringInStr($mini[$j], '">') + 1)
		GUICtrlCreateTreeViewItem($urlb, $TreeView1)
		IniWrite(@ScriptDir & '\forum\' & $url & ".bbs", $urlb, "url", $urla)
	Next
	_IEQuit($oIE)
	ToolTip("")
EndFunc   ;==>getBBSinfo