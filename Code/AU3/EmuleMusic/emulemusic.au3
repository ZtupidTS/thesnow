#Region AutoIt3Wrapper 预编译参数(常用参数)
#AutoIt3Wrapper_UseAnsi=N									;编码
#AutoIt3Wrapper_Icon=D:\emule\emule.exe						;图标,支持EXE,DLL,ICO
#AutoIt3Wrapper_OutFile=									;输出文件名
#AutoIt3Wrapper_OutFile_Type=exe							;文件类型
#AutoIt3Wrapper_Compression=4								;压缩等级
#AutoIt3Wrapper_UseUpx=y 									;使用压缩
#AutoIt3Wrapper_Res_Comment= thesnoW								;注释
#AutoIt3Wrapper_Res_Description=thesnoW							;详细信息
#AutoIt3Wrapper_Res_Fileversion=0.0.0.20
#AutoIt3Wrapper_Res_FileVersion_AutoIncrement=p				;自动更新版本  
#AutoIt3Wrapper_Res_LegalCopyright= thesnoW						;版权
#AutoIt3Wrapper_Change2CUI=y                   				;修改输出的程序为CUI(控制台程序)
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

;华语音乐
;http://www.verycd.com/archives/music/china/
;欧美音乐
;http://www.verycd.com/archives/music/occident/

#NoTrayIcon
#Include <String.au3>
If $CmdLine[0] >0 Then					;加参数运行
	_GetFileInfo($CmdLine[1])
	Exit
EndIf

If Not FileExists("TotalList.txt") Then
	ConsoleWrite("下载总列表..." & @CRLF)
	If _GetNetFile('http://www.verycd.com/archives/music/occident/') Then
		ConsoleWrite("分析总列表URL..." & @CRLF)
		_GetTotalListUrl()
	Else
		ConsoleWrite("下载总列表失败...")
		Sleep(2000)
		Exit
	EndIf
Else
	ConsoleWrite("分析总列表..." & @CRLF)
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
		If StringInStr($line,'<dl id="archivePageList">') Then		;列表开始
			While 1
				$line = FileReadLine($hList)
				If @error = -1 Then ExitLoop
				If StringInStr($line,'</dl><!--end of archivePageList-->') Then		;列表结束
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
				ConsoleWrite("下载一级列表...[" & $Title & "]" & @CRLF)
				_GetLevelOneList($Url,$Title)
			EndIf
		EndIf
	Next
EndFunc

Func _GetLevelOneList($szUrl,$szTitle)
	If _GetNetFile($szUrl) Then
		FileMove(@ScriptDir & "\TmpList.txt",@ScriptDir & "\" & $szTitle & "\TmpList.txt")
		ConsoleWrite("分析一级列表URL..." & @CRLF)
		_GetLevelOneListUrl($szTitle)
	Else
		ConsoleWrite("下载一级列表失败...程序退出" & @CRLF)
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
		MsgBox(0, "错误", "不能打开文件:" & $szFileName)
		ConsoleWrite("不能打开文件:" & $szFileName & @CRLF)
		Return 0
	EndIf
	While 1
		$line = FileReadLine($hFile)
		If @error = -1 Then ExitLoop
		If StringInStr($line,'<!--eMule begin-->') Then		;列表开始
			$File &= $line & @CRLF
			While 1
				$line = FileReadLine($hFile)
				If @error = -1 Then ExitLoop
				If StringInStr($line,'<!--Wrap-tail begin-->') Then		;列表结束
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
							ConsoleWrite("已添加文件(" & $count & "):	" & $File & @CRLF)
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
	ConsoleWrite("总添加文件数:" & $count-1 & @CRLF)
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
		If StringInStr($line,'<ol id="archiveResourceList" start="') Then		;列表开始
			While 1
				$line = FileReadLine($hList)
				If @error = -1 Then ExitLoop
									  ;</div><!--end of resList-->
				If StringInStr($line,'</div><!--end of resList-->') Then		;列表结束
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
							;ToolTip("("& $i & "/100)下载页面分析中...",0,0)
							ConsoleWrite("下载二级列表项目		[" & $i & "/100]" & @CRLF)
							If _GetLevelTwoList("http://www.verycd.com" & $str[1],@ScriptDir & "\" & $szTitle & "\" & $szFileName & '.emule') Then
								ConsoleWrite("生成电骡文件中...")
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
								ConsoleWrite("...	成功!" & @CRLF & @CRLF)
							Else
								ConsoleWrite("文件下载失败,程序退出..." & @CRLF)
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
	;处理网页符号
	$str=StringReplace($str,'&amp;','&')
	$str=StringReplace($str,'&#33;','!')
	$str=StringReplace($str,'&#39;',"'")
	;处理特殊字符	\/:*?"<>|
	$str=StringReplace($str,'/','-')
	$str=StringReplace($str,'\','-')
	$str=StringReplace($str,':','-')
	$str=StringReplace($str,'*','-')
	$str=StringReplace($str,'?','？')
	$str=StringReplace($str,'"','$')
	$str=StringReplace($str,'<','[')
	$str=StringReplace($str,'>',']')
	$str=StringReplace($str,'|','-')
	Return $str
EndFunc