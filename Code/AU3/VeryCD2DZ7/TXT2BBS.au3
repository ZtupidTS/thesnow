#Region AutoIt3Wrapper Ԥ�������(���ò���)
#AutoIt3Wrapper_Icon= 	nis									;ͼ��,֧��EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;����ļ���
#AutoIt3Wrapper_UseX64=n
#AutoIt3Wrapper_OutFile_Type=exe							;�ļ�����
#AutoIt3Wrapper_Compression=4								;ѹ���ȼ�
#AutoIt3Wrapper_UseUpx=y 									;ʹ��ѹ��
#AutoIt3Wrapper_Res_Comment= 								;ע��
#AutoIt3Wrapper_Res_Description=thesnoW							;��ϸ��Ϣ
#AutoIt3Wrapper_Res_Fileversion=0.0.1.8
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;�Զ����°汾  
#AutoIt3Wrapper_Res_LegalCopyright=thesnoW 						;��Ȩ
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
#Region 
	#Include <Array.au3>
	#include <IE.au3> 
	#Include <String.au3>
#EndRegion

Global $SrcUrl=	InputBox("������һ��URL","������һ��URL","")
Global $SrcTmp=@TempDir & "\tmped.tmp"
Global $title=''
Global $ED2KURL=''
Global $Contents
Global $IE
Global $oForm
Global $oQuery

FileDelete(@ScriptDir & "\*.jpg")
FileDelete(@ScriptDir & "\*.txt")
If $SrcUrl="" Then Exit
ToolTip("��Ŀ������...",0,0)
inetGet($SrcUrl,$SrcTmp,1,0)
If @error  Then Exit
ToolTip("")
;FileCopy($SrcTmp,"src.txt",1)
ReadFile()
;MsgBox(32,$title,$Contents)
ConsoleWrite($title)
ConsoleWrite($Contents)
post()
Exit


Func ReadFile()
	$file = FileOpen($SrcTmp, 128)
	$fileX= FileOpen("info.txt",129)
	; ���򿪵��ļ��Ƿ�ɶ�
	If $file = -1 Then
		MsgBox(0, "����", "���ܴ��ļ�.")
		Exit
	EndIf

	; ÿ�ζ�ȡһ���ı�,ֱ���ļ�����.
	While 1
		$line = FileReadLine($file)
		If @error = -1 Then ExitLoop
		;------
			If $title = '' Then
				If StringInStr($line,'<title>') Then
					$title=$line
					$title=StringReplace($title,'<title>','')
					$title=StringReplace($title,'</title>','')
					$title=StringReplace($title,'_VeryCD��¿����','')
	;				MsgBox(32,"title",$title)
				EndIf
			EndIf
			If StringInStr($line,'<!--eMule begin-->') Then
			;ED2K start
				While 1
					$line = StringStripWS(FileReadLine($file),3)
					If @error = -1 Then ExitLoop
					$lineX= _StringBetween($line,'ed2k="','</a>')
					If Not @error Then WrapED2K($lineX[0])
					If StringInStr($line,'<!--eMule end-->') Then ExitLoop
				WEnd
			;con start
					FileWriteLine($fileX,@CRLF & @CRLF)
				While 1
					$line = StringStripWS(FileReadLine($file),3)
					If @error = -1 Then ExitLoop
					;$lineX= _StringBetween($line,'value=','onclick="em_size')
					If Not @error Then FileWriteLine($fileX,DropHtml($line))
					If StringInStr($line,'<script type="text/javascript">') Then ExitLoop
				WEnd
				ExitLoop				
			EndIf
		;------
	Wend
;	FileWriteLine($fileX, $ED2KURL)
	FileWrite($fileX,'[b]����[/b]:' & @CRLF & $ED2KURL)
	FileClose($fileX)
	$fileX= FileOpen("info.txt",128)
	$Contents=FileRead($fileX)
	FileClose($fileX)
	FileClose($file)
	FileDelete("info.txt")
EndFunc

Func WrapED2K($ED2K)
	Local $ED2KURLS
	$ED2K=StringStripWS($ED2K,3)
	$ED2KURLS=stringleft($ED2K,StringInStr($ED2K,'">',-1)-1)
	$ED2KFile=StringReplace($ED2K,$ED2KURLS,'')
	$ED2KFile=stringtrimleft($ED2KFile,2)
	$ED2K='[emule=' & $ED2KURLS & ']' & $ED2KFile & '[/emule]'
	$ED2KURL=$ED2KURL & @CRLF & $ED2K
	Return $ED2K
EndFunc

Func DropHtml($str)
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomCname"><span class="iptcom-title">',@CRLF)				;����ר����
	$str=StringReplace($str,'</div><div class="iptcom-multiline" id="iptcomActor"><span class="iptcom-title">',@CRLF)	;����
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomKind"><span class="iptcom-title">',@CRLF)				;���
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomFiletype"><span class="iptcom-title">',@CRLF)			;��Դ��ʽ
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomVersion"><span class="iptcom-title">',@CRLF)			;��Դ�汾
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomTime"><span class="iptcom-title">',@CRLF)				;����ʱ��
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomCountry"><span class="iptcom-title">',@CRLF)			;����
	$str=StringReplace($str,'<div class="iptcom" id="iptcomLanguageWriting"><span class="iptcom-title">',@CRLF)			;ͬ��
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomLanguage"><span class="iptcom-title">',@CRLF)			;����
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomContents"><span class="iptcom-title">',@CRLF)			;���
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomTrack"><br /><br /><span class="iptcom-title">',"")	;ר����Ŀ
	$str=StringReplace($str,'<!--Wrap-head begin-->',"")
	$str=StringReplace($str,'<p class="inner_content">',"")
	$str=StringReplace($str,'<!--Wrap-head end-->',"")
	$str=StringReplace($str,'<!--Wrap-tail begin-->',"")
	$str=StringReplace($str,'<!--Wrap-tail end-->',"")	
	$str=StringReplace($str,'</a></span>',"")
	$str=StringReplace($str,'&nbsp;</span>',"")	
	$str=StringReplace($str,'<script type="text/javascript">',"")	
	$str=StringReplace($str,'</p>',"")	
	$str=StringReplace($str,'</b>','[/b]')				;ת��UBB��ʽ
	$str=StringReplace($str,'<b>','[b]')				;ת��UBB��ʽ
	$str=StringReplace($str,'</strong>','[/b]')			;ת��UBB��ʽ
	$str=StringReplace($str,'<strong>','[b]')			;ת��UBB��ʽ
	$str=StringReplace($str,'<br />',@CRLF)
	$str=StringReplace($str,'</div>',"")
	$str=DropAD($str)
	Return DownloadImage($str)
EndFunc

Func DropAD($ADstr)
	$AD=_StringBetween($ADstr,'<!--Flash','<!--End Flash-->')
	If Not @error Then
		For $i= 0 To (UBound($AD)-1)
			$ADstr=StringReplace($ADstr,'<!--Flash' & $AD[$i] & '<!--End Flash-->','')
		Next
	EndIf
	$AD=_StringBetween($ADstr,'<span class="iptcom-info"><a href=',')">')
	If Not @error Then
		$ADstr=StringReplace($ADstr,'<span class="iptcom-info"><a href=' & $AD[0] & ')">','')
	EndIf
	$AD=_StringBetween($ADstr,'<a href="',')">')
	If Not @error Then
		For $i= 0 To (UBound($AD)-1)
			$ADstr=StringReplace($ADstr,'<a href="' & $AD[$i] & ')">','')
		Next
	EndIf
	$ADstr=StringReplace($ADstr,'</a>','')
	Return $ADstr
EndFunc

Func DownloadImage($ImageStr)
	ToolTip('����ͼƬ��...',0,0)
	$Image=_StringBetween($ImageStr,'<img src="','" border="0" alt="IPB Image" name="post_img" />')
	If Not @error Then
		For $i= 0 To (UBound($Image)-1)
			$ImageStr=StringReplace($ImageStr,'<img src="' & $Image[$i] & '" border="0" alt="IPB Image" name="post_img" />','')
			InetGet($Image[$i],@ScriptDir & "\" & $i & ".jpg",1)
		Next
	EndIf
	ToolTip("")
	Return $ImageStr
EndFunc





Func post()
	;��������
	
;	$ie=_IECreate ( 'http://www.zwtiso.com/post.php?action=newthread&fid=31', 1 , 1 , 1 , 1)	;ŷ��
;	$ie=_IECreate ( 'http://www.zwtiso.com/post.php?action=newthread&fid=32', 1 , 1 , 1 , 1)	;�պ�	
;	$ie=_IECreate ( 'http://www.zwtiso.com/post.php?action=newthread&fid=33', 1 , 1 , 1 , 1)	;����
;	$ie=_IECreate ( 'http://www.zwtiso.com/post.php?action=newthread&fid=34', 1 , 1 , 1 , 1)	;MTV
	
	$ie=_IECreate ( $CmdLineRaw, 1 , 1 , 1 , 1)	;MTV
	;$ie=_IECreate ( 'http://thesnow.111.80000web.net.cn/post.php?action=newthread&fid=33', 1 , 1 , 1 , 1)
	$oForm = _IEFormGetObjByName ($IE, "postform")
	$oQuery = _IEGetObjById ($oForm, "subject")					;����
	_IEFormElementSetValue ($oQuery, $title)
	$oQuery = _IEGetObjById ($oForm, 'message')					;����
	_IEFormElementSetValue ($oQuery, $Contents)

	;~ ;_IEFormSubmit ($oForm)
	;AdlibRegister("LoadData")
	;$attach = _IEGetObjById($ie,"attachnew_1")					;��Ӹ���
	;$attach.click
	;AdlibUnRegister("LoadData")

EndFunc

Func date()
	Return '[' & @YEAR & @MON & @MDAY & ']'
EndFunc

Func LoadData()
	If WinExists('ѡ��Ҫ���ص��ļ�') Then
		ControlSetText('ѡ��Ҫ���ص��ļ�',"",'Edit1','D:\deepfreeze_std_setup.exe')
		ControlClick('ѡ��Ҫ���ص��ļ�',"","Button2")
		Sleep(500)
	EndIf
EndFunc