#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_UseAnsi=N									;����
#AutoIt3Wrapper_Icon=D:\emule\emule.exe						;ͼ��,֧��EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_OutFile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= thesnoW								;ע��
#AutoIt3Wrapper_Res_Description=thesnoW							;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=0.0.0.20
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright= thesnoW						;��Ȩ
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

;��������
;http://www.verycd.com/archives/music/china/
;ŷ������
;http://www.verycd.com/archives/music/occident/

#NoTrayIcon
#Include <String.au3>
If $CmdLine[0] >0 Then					;�Ӳ�������
	_GetFileInfo($CmdLine[1])
	Exit
EndIf

If Not FileExists("TotalList.txt") Then
	ConsoleWrite("�������б�..." & @CRLF)
	If _GetNetFile('http://www.verycd.com/archives/music/occident/') Then
		ConsoleWrite("�������б�URL..." & @CRLF)
		_GetTotalListUrl()
	Else
		ConsoleWrite("�������б�ʧ��...")
		Sleep(2000)
		Exit
	EndIf
Else
	ConsoleWrite("�������б�..." & @CRLF)
	_GetTotalList()
EndIf


Func _GetNetFile($Url)
	FileDelete(@ScriptDir & "\TmpList.txt")
	InetGet($Url,@ScriptDir & "\TmpList.txt",1,0)
	if @error Then
		Return 0
	Else
		Return 1
	EndIf
EndFunc

Func _GetTotalListUrl()
	FileDelete("TotalList.txt")
	Local $Url, $i=0
	$hList = FileOpen(@ScriptDir & "\TmpList.txt", 128)
	; Check if file opened for reading OK
	If $hList = -1 Then
		ConsoleWrite("Unable to open file:" & @ScriptDir & "\TmpList.txt" & @CRLF)
		Return 0
	EndIf
	; Read in lines of text until the EOF is reached
	While 1
		$line = FileReadLine($hList)
		If @error = -1 Then ExitLoop
		If StringInStr($line,'<dl id="archivePageList">') Then		;�б�ʼ
			While 1
				$line = FileReadLine($hList)
				If @error = -1 Then ExitLoop
				If StringInStr($line,'</dl><!--end of archivePageList-->') Then		;�б����
					ExitLoop
				Else
					$Url=_StringBetween($line,'<dd><a href="','</a></dd>')
					If Not @error Then
						$i+=1
						IniWrite("TotalList.txt",'list',$i,'http://www.verycd.com/' & StringReplace($Url[0],'">','|'))
					EndIf
				EndIf
			WEnd
			ExitLoop
		EndIf
	Wend
	FileClose($hList)
	FileDelete(@ScriptDir & "\TmpList.txt")
	Return 1
EndFunc

Func _GetTotalList()
	Local $Url ,$Title
	$ini=IniReadSection("TotalList.txt","list")
	If @error Then Return 0
	For $i= 1 To $ini[0][0]
		If StringRight($ini[$i][1],2) <> "00" Then ContinueLoop
		$str= StringSplit($ini[$i][1],'|')	
		If $str[0] <> 2 Then
			ContinueLoop
		Else
			$Url=$str[1]
			$Title=$str[2]
			If FileExists($Title) Then
				ContinueLoop
			Else
				DirCreate($Title)
				ConsoleWrite("����һ���б�...[" & $Title & "]" & @CRLF)
				_GetLevelOneList($Url,$Title)
			EndIf
		EndIf
	Next
EndFunc

Func _GetLevelOneList($szUrl,$szTitle)
	If _GetNetFile($szUrl) Then
		FileMove(@ScriptDir & "\TmpList.txt",@ScriptDir & "\" & $szTitle & "\TmpList.txt")
		ConsoleWrite("����һ���б�URL..." & @CRLF)
		_GetLevelOneListUrl($szTitle)
	Else
		ConsoleWrite("����һ���б�ʧ��...�����˳�" & @CRLF)
		Sleep(2000)
		Exit
	EndIf
EndFunc

Func _GetLevelTwoList($szUrl,$szFileName)
	If _GetNetFile($szUrl) Then
		FileMove(@ScriptDir & "\TmpList.txt",$szFileName)
		Return 1
	Else
		Return 0
	EndIf
EndFunc


Func _GetFileInfo($szFileName)
	Local $Url, $i=0 ,$str,$File,$count=1
	$hFile = FileOpen($szFileName, 128)
	; Check if file opened for reading OK
	If $hFile = -1 Then
		MsgBox(0, "����", "���ܴ��ļ�:" & $szFileName)
		ConsoleWrite("���ܴ��ļ�:" & $szFileName & @CRLF)
		Return 0
	EndIf
	While 1
		$line = FileReadLine($hFile)
		If @error = -1 Then ExitLoop
		If StringInStr($line,'<!--eMule begin-->') Then		;�б�ʼ
			$File &= $line & @CRLF
			While 1
				$line = FileReadLine($hFile)
				If @error = -1 Then ExitLoop
				If StringInStr($line,'<!--Wrap-tail begin-->') Then		;�б����
					ExitLoop
				Else
					$File &= $line & @CRLF
					$Url=_StringBetween($line,'ed2k="','</a>')
					If Not @error Then
						$i+=1
						$str=StringSplit(StringReplace($Url[0],'">','||||'),"||||",1)
						If $str[0] <> 2 Then
							ContinueLoop
						Else
							$File=DelHtmlMark($str[2])
							ConsoleWrite("������ļ�(" & $count & "):	" & $File & @CRLF)
							$count+=1
							Run(@ScriptDir & "\emule.exe" & " " & $str[1])
							Sleep(500)
						EndIf				
					EndIf
				EndIf
			WEnd
			ExitLoop
		EndIf
	Wend
	FileClose($hFile)
	If $count <> 1 Then FileSetAttrib($szFileName,"+h")
	ConsoleWrite("������ļ���:" & $count-1 & @CRLF)
	Sleep(1000)
	Return 1
EndFunc

Func _GetLevelOneListUrl($szTitle)
	Local $Url, $i=0 ,$str,$szFileName
	$hList = FileOpen(@ScriptDir & "\" & $szTitle & "\TmpList.txt", 128)
	; Check if file opened for reading OK
	If $hList = -1 Then
		ConsoleWrite("Unable to open file:" & @ScriptDir  & "\" & $szTitle & "\TmpList.txt" & @CRLF)
		Return 0
	EndIf
	; Read in lines of text until the EOF is reached
	While 1
		$line = FileReadLine($hList)
		If @error = -1 Then ExitLoop
							  ;<ol id="archiveResourceList" start="1201">
		If StringInStr($line,'<ol id="archiveResourceList" start="') Then		;�б�ʼ
			While 1
				$line = FileReadLine($hList)
				If @error = -1 Then ExitLoop
									  ;</div><!--end of resList-->
				If StringInStr($line,'</div><!--end of resList-->') Then		;�б����
					ExitLoop
				Else						   ;<li><a href=
					$Url=_StringBetween($line,'<li><a href="','</a></li>')
					If Not @error Then
						$i+=1
						$str=StringSplit(StringReplace($Url[0],'">','|'),"|")
						If $str[0] <> 2 Then
							ContinueLoop
						Else
							$szFileName=DelHtmlMark($str[2])
							;ToolTip("("& $i & "/100)����ҳ�������...",0,0)
							ConsoleWrite("���ض����б���Ŀ		[" & $i & "/100]" & @CRLF)
							If _GetLevelTwoList("http://www.verycd.com" & $str[1],@ScriptDir & "\" & $szTitle & "\" & $szFileName & '.emule') Then
								ConsoleWrite("���ɵ����ļ���...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)
								ConsoleWrite("...")
								Sleep(300)								
								ConsoleWrite("...	�ɹ�!" & @CRLF & @CRLF)
							Else
								ConsoleWrite("�ļ�����ʧ��,�����˳�..." & @CRLF)
								Sleep(3000)
								Exit
							EndIf
						EndIf
						;IniWrite("TotalList.txt",'list',$i,'http://www.verycd.com/' & StringReplace($Url[0],'">','|'))
					EndIf
				EndIf
				ToolTip("")
			WEnd
			ExitLoop
		EndIf
	Wend
	FileClose($hList)
	FileDelete(@ScriptDir & "\TmpList.txt")
	Return 1
EndFunc

Func DelHtmlMark($str)
	;������ҳ����
	$str=StringReplace($str,'&amp;','&')
	$str=StringReplace($str,'&#33;','!')
	$str=StringReplace($str,'&#39;',"'")
	;���������ַ�	\/:*?"<>|
	$str=StringReplace($str,'/','-')
	$str=StringReplace($str,'\','-')
	$str=StringReplace($str,':','-')
	$str=StringReplace($str,'*','-')
	$str=StringReplace($str,'?','��')
	$str=StringReplace($str,'"','$')
	$str=StringReplace($str,'<','[')
	$str=StringReplace($str,'>',']')
	$str=StringReplace($str,'|','-')
	Return $str
EndFunc