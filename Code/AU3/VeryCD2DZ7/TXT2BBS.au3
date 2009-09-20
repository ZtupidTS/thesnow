#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_Icon= 	nis									;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_UseX64=n
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment= 								;注释
#AutoIt3Wrapper_Res_Description=thesnoW							;详细信息
#AutoIt3Wrapper_Res_Fileversion=0.0.1.8
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright=thesnoW 						;版权
#AutoIt3Wrapper_Change2CUI=N                   				;修改输出的程序为CUI(控制台程序)
;#AutoIt3Wrapper_Res_Field=AutoIt Version|%AutoItVer%		;自定义资源段
;#AutoIt3Wrapper_Run_Tidy=                   				;脚本整理
;#AutoIt3Wrapper_Run_Obfuscator=      						;代码迷惑
;#AutoIt3Wrapper_Run_AU3Check= 								;语法检查
;#AutoIt3Wrapper_Run_Before= 								;运行前
;#AutoIt3Wrapper_Run_After=									;运行后
#EndRegion AutoIt3Wrapper 预编译参数设置完成
#cs ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿

 Au3 版本:
 脚本作者: 
	Email: 
	QQ/TM: 
 脚本版本: 
 脚本功能: 

#ce ＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿脚本开始＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿＿
#Region 
	#Include <Array.au3>
	#include <IE.au3> 
	#Include <String.au3>
#EndRegion

Global $SrcUrl=	InputBox("请输入一个URL","请输入一个URL","")
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
ToolTip("项目下载中...",0,0)
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
	; 检查打开的文件是否可读
	If $file = -1 Then
		MsgBox(0, "错误", "不能打开文件.")
		Exit
	EndIf

	; 每次读取一行文本,直到文件结束.
	While 1
		$line = FileReadLine($file)
		If @error = -1 Then ExitLoop
		;------
			If $title = '' Then
				If StringInStr($line,'<title>') Then
					$title=$line
					$title=StringReplace($title,'<title>','')
					$title=StringReplace($title,'</title>','')
					$title=StringReplace($title,'_VeryCD电驴下载','')
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
	FileWrite($fileX,'[b]下载[/b]:' & @CRLF & $ED2KURL)
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
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomCname"><span class="iptcom-title">',@CRLF)				;中文专辑名
	$str=StringReplace($str,'</div><div class="iptcom-multiline" id="iptcomActor"><span class="iptcom-title">',@CRLF)	;歌手
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomKind"><span class="iptcom-title">',@CRLF)				;风格
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomFiletype"><span class="iptcom-title">',@CRLF)			;资源格式
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomVersion"><span class="iptcom-title">',@CRLF)			;资源版本
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomTime"><span class="iptcom-title">',@CRLF)				;发行时间
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomCountry"><span class="iptcom-title">',@CRLF)			;地区
	$str=StringReplace($str,'<div class="iptcom" id="iptcomLanguageWriting"><span class="iptcom-title">',@CRLF)			;同上
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomLanguage"><span class="iptcom-title">',@CRLF)			;语言
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomContents"><span class="iptcom-title">',@CRLF)			;简介
	$str=StringReplace($str,'</div><div class="iptcom" id="iptcomTrack"><br /><br /><span class="iptcom-title">',"")	;专辑曲目
	$str=StringReplace($str,'<!--Wrap-head begin-->',"")
	$str=StringReplace($str,'<p class="inner_content">',"")
	$str=StringReplace($str,'<!--Wrap-head end-->',"")
	$str=StringReplace($str,'<!--Wrap-tail begin-->',"")
	$str=StringReplace($str,'<!--Wrap-tail end-->',"")	
	$str=StringReplace($str,'</a></span>',"")
	$str=StringReplace($str,'&nbsp;</span>',"")	
	$str=StringReplace($str,'<script type="text/javascript">',"")	
	$str=StringReplace($str,'</p>',"")	
	$str=StringReplace($str,'</b>','[/b]')				;转换UBB格式
	$str=StringReplace($str,'<b>','[b]')				;转换UBB格式
	$str=StringReplace($str,'</strong>','[/b]')			;转换UBB格式
	$str=StringReplace($str,'<strong>','[b]')			;转换UBB格式
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
	ToolTip('下载图片中...',0,0)
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
	;创建对象
	
;	$ie=_IECreate ( 'http://www.zwtiso.com/post.php?action=newthread&fid=31', 1 , 1 , 1 , 1)	;欧美
;	$ie=_IECreate ( 'http://www.zwtiso.com/post.php?action=newthread&fid=32', 1 , 1 , 1 , 1)	;日韩	
;	$ie=_IECreate ( 'http://www.zwtiso.com/post.php?action=newthread&fid=33', 1 , 1 , 1 , 1)	;国语
;	$ie=_IECreate ( 'http://www.zwtiso.com/post.php?action=newthread&fid=34', 1 , 1 , 1 , 1)	;MTV
	
	$ie=_IECreate ( $CmdLineRaw, 1 , 1 , 1 , 1)	;MTV
	;$ie=_IECreate ( 'http://thesnow.111.80000web.net.cn/post.php?action=newthread&fid=33', 1 , 1 , 1 , 1)
	$oForm = _IEFormGetObjByName ($IE, "postform")
	$oQuery = _IEGetObjById ($oForm, "subject")					;标题
	_IEFormElementSetValue ($oQuery, $title)
	$oQuery = _IEGetObjById ($oForm, 'message')					;内容
	_IEFormElementSetValue ($oQuery, $Contents)

	;~ ;_IEFormSubmit ($oForm)
	;AdlibRegister("LoadData")
	;$attach = _IEGetObjById($ie,"attachnew_1")					;添加附件
	;$attach.click
	;AdlibUnRegister("LoadData")

EndFunc

Func date()
	Return '[' & @YEAR & @MON & @MDAY & ']'
EndFunc

Func LoadData()
	If WinExists('选择要加载的文件') Then
		ControlSetText('选择要加载的文件',"",'Edit1','D:\deepfreeze_std_setup.exe')
		ControlClick('选择要加载的文件',"","Button2")
		Sleep(500)
	EndIf
EndFunc