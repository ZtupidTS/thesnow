; tcmatch.ahk: QuickSearch eXtended GUI for Total Commander 7.5+
  Version=2.0.1
; by Samuel Plentz

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - init the script
#SingleInstance force
#Persistent
#NoTrayIcon
#NoEnv
SetBatchLines,-1
SetKeyDelay,-1
DetectHiddenWindows,On
OnMessage(0x06 ,"WM_ACTIVATE")
OnMessage(0x200,"WM_MOUSEOVER")
isWinActive=0
QSvisible=0
QSDelay=0

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - use 16x16 and 32x32 program icon
if(A_IsCompiled){
 hModule:=DllCall("GetModuleHandle",Str,A_ScriptFullPath)
 Icon16:=DllCall("LoadImageW",UInt,hModule,UInt,              159,UInt,IMAGE_ICON:=0x1,Int,16,Int,16,UInt,LR_SHARED      :=0x8000)
 Icon32:=DllCall("LoadImageW",UInt,hModule,UInt,              159,UInt,IMAGE_ICON:=0x1,Int,32,Int,32,UInt,LR_SHARED      :=0x8000)
}else{
 Icon16:=DllCall("LoadImageW",UInt,      0, Str,"tcmatch.ahk.ico",UInt,IMAGE_ICON:=0x1,Int,16,Int,16,UInt,LR_LOADFROMFILE:=0x10)
 Icon32:=DllCall("LoadImageW",UInt,      0, Str,"tcmatch.ahk.ico",UInt,IMAGE_ICON:=0x1,Int,32,Int,32,UInt,LR_LOADFROMFILE:=0x10)
}
Gui 2:Default
Gui,+LastFound
SendMessage,WM_SETICON:=0x80,ICON_SMALL:=0x0,Icon16
SendMessage,WM_SETICON:=0x80,ICON_BIG  :=0x1,Icon32
Gui 3:Default
Gui,+LastFound
SendMessage,WM_SETICON:=0x80,ICON_SMALL:=0x0,Icon16
SendMessage,WM_SETICON:=0x80,ICON_BIG  :=0x1,Icon32

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - load language strings
LanguageStrings=
(
 0,English                                                    ,Deutsch
 1,Simple search                                              ,einfache Suche
 2,RegEx search                                               ,RegEx Suche
 3,Similarity search                                          ,Ähnlichkeitssuche
 4,Srch                                                       ,Srch
 5,<Presets>                                                  ,<Favoriten>
 6,General                                                    ,Allgemein
 7,Change syntax                                              ,Suchsyntax ändern
 8,Presets                                                    ,Favoriten
 9,Replacement rules                                          ,Ersetzungsregeln
10,Search options                                             ,Suchoptionen
11,Match only at beginning of files/words                     ,Übereinstimmung nur am Datei-/Wortanfang
12,Case sensitive                                             ,Groß-/Kleinschreibung beachten
13,"Allow input, leading to empty results (TC restart needed)","Eingaben erlauben, die zu keinem Ergebnis führen (TC-Neustart nötig)"
14,Filter files                                               ,Dateien filtern
15,Filter folders                                             ,Ordner filtern
16,Use PinYin (for Chinese users)                             ,PinYin verwenden (für chinesische Anwender)
17,Additional user interface                                  ,ergänzende Benutzeroberfläche
18,Activate                                                   ,aktivieren
19,Show in one row                                            ,einzeilig anzeigen
20,Activation chars for search modes                          ,Zeichen die Suchmodi aktivieren
21,Change                                                     ,ändern
22,Char for simple search                                     ,Zeichen für die einfache Suche
23,Char for simple search with match only at beginning        ,Zeichen für die einfache Suche mit Übereinstimmung am Anfang
24,Char for RegEx                                             ,Zeichen für die RegEx Suche
25,Char for similarity search                                 ,Zeichen für die Ähnlichkeitssuche
26,Char for srch                                              ,Zeichen für die Srch
27,Other chars                                                ,sonstige Zeichen
28,And separator char                                         ,Und Trennzeichen
29,Or separator char                                          ,Oder Trennzeichen
30,Negate char                                                ,Negationszeichen
31,Activation char for presets                                ,Favoriten Aktivierungszeichen
32,Content plugin separator char                              ,Inhaltsplugins Trennzeichen
33,New char                                                   ,neues Zeichen
34,Please enter a new char:                                   ,Bitte ein neues Zeichen eingeben:
35,Confirm                                                    ,übernehmen
36,Char:                                                      ,Zeichen:
37,Preset string:                                             ,Favoritensuchstring:
38,Add                                                        ,hinzufügen
39,Preset list:                                               ,Favoritenliste:
40,Chars to replace:                                          ,zu ersetzende Zeichen:
41,New text:                                                  ,Ersetzungstext:
42,Replacement rules:                                         ,Ersetzungsliste:
43,Show help                                                  ,Hilfe anzeigen
44,Helpfile not found                                         ,Hilfedatei nicht gefunden
45,Helpfile was not found in this path:                       ,Die Hilfedatei wurde unter folgendem Pfad nicht gefunden:
46,Show presets                                               ,Favoriten anzeigen
47,Select language: (TC restart needed)                       ,Sprache auswählen: (TC-Neustart nötig)
48,Content plugins                                            ,Inhaltsplugins
49,Choose content plugin file:                                ,Inhaltsplugin-Datei auswählen:
50,Fields of the content plugin:                              ,Felder des Inhaltsplugins:
51,Choose group:                                              ,Gruppe wählen:
52,update                                                     ,ändern
53,Content plugin list:                                       ,Inhaltsplugin-Liste:
54,Extended Options                                           ,erweiterte Einstellungen
55,Logfile: (slows down the search)                           ,Logdatei: (verlangsamt die Suche)
56,disabled                                                   ,deaktiviert
57,create only for errors                                     ,nur bei Fehlern erstellen
58,log the values of content plugin                           ,Werte der Inhaltsplugins loggen
59,log all function calls                                     ,alle Funktionsaufrufe loggen
60,Size of cache: (Number of Files)                           ,Größe des Cache: (Dateianzahl)
61,Invert the result                                          ,invertiere das Ergebnis
62,Quick Search eXtended - configuration                      ,Quick Search eXtended - Einstellungen
)

IniRead,INI_language,%A_ScriptDir%\tcmatch.ini,gui,language,1
if(INI_language!="2" && INI_language!="3"){
 if A_Language in 0407,0807,0c07,1007,1407
  INI_language=3
 ;### else if A_Language in 040c,080c,0c0c,100c,140c,180c ; French
 else INI_language=2
}      
       
loop,parse,LanguageStrings,`n,`r
{      
 i:=A_Index-1
 loop,parse,A_LoopField,CSV
 {     
  if(A_Index==INI_language){
   tmp=%A_LoopField%
   L%i%:=tmp
  }     
 }      
}       

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - load the icons
loop,9  
{       
 VarSetCapacity(Icon_small,4,0)
 VarSetCapacity(Icon_big,4,0)
 DllCall("shell32.dll\ExtractIconExW","str","tcmatch.dll","int",A_Index-1,"str",Icon_big,"str",Icon_small,"uint",1)
 DllCall("DestroyIcon","uint",NumGet(Icon_big))
 Icon_%A_Index%:=NumGet(Icon_small)
}        
         
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - create gui1
Gui 1:Default
IniRead,INI_one_line_gui,%A_ScriptDir%\tcmatch.ini,gui,one_line_gui,-1
if(INI_one_line_gui=="-1"){
 IniWrite,1,%A_ScriptDir%\tcmatch.ini,gui,one_line_gui
 INI_one_line_gui=1
}         
IniRead,INI_show_presets,%A_ScriptDir%\tcmatch.ini,gui,show_presets,-1
if(INI_show_presets=="-1"){
 IniWrite,0,%A_ScriptDir%\tcmatch.ini,gui,show_presets
 INI_show_presets=0
}         
          
Gui,+AlwaysOnTop -Border +ToolWindow
if(INI_one_line_gui=="0"){
 Gui,add,button      ,x0   y0  hwndhwnd_1 +64 h24 w24 vQSXVB1 gLMatchBeginning
 Gui,add,button      ,x26  y0  hwndhwnd_2 +64 h24 w24 vQSXVB2 gLCaseSensitive
 Gui,add,button      ,x0   y26 hwndhwnd_3 +64 h24 w24 vQSXVB3 gLInvert
 Gui,add,button      ,x26  y26 hwndhwnd_4 +64 h24 w24 vQSXVB4 gShowGui2
 Gui,add,dropdownlist,x52  y1  w130 altsubmit vVSearch gLSearch       ,%L1%||%L2%|%L3%|%L4%
 Gui,add,picture     ,x52  y30 w16 h16 vVFavicon Icon7                ,tcmatch.dll
 Gui,add,dropdownlist,x72  y27 w110 vVFavitems gLFavitems             ,
}else{     
 Gui,add,button      ,x0   y0  hwndhwnd_1 +64 h24 w24 vQSXVB1 gLMatchBeginning
 Gui,add,button      ,x26  y0  hwndhwnd_2 +64 h24 w24 vQSXVB2 gLCaseSensitive
 Gui,add,button      ,x52  y0  hwndhwnd_3 +64 h24 w24 vQSXVB3 gLInvert
 Gui,add,dropdownlist,x83  y1  w110 altsubmit vVSearch gLSearch       ,%L1%||%L2%|%L3%|%L4%
 Gui,add,picture     ,x200 y4  w16 h16 vVFavicon Icon7                ,tcmatch.dll
 Gui,add,dropdownlist,x220 y1  w110 vVFavitems gLFavitems             ,
 Gui,add,button      ,x337 y0  hwndhwnd_4 +64 h24 w24 vQSXVB4 gShowGui2
 if(INI_show_presets==0){
  GuiControl,move,QSXVB4   ,x200 y0
  GuiControl,move,VFavitems,x0 y100
  GuiControl,move,VFavicon ,x0 y100
  GuiControl,disable,VFavitems
  GuiControl,disable,VFavicon
 }           
}            
QSXVB1_Tooltip=%L11%
QSXVB2_Tooltip=%L12%
QSXVB3_Tooltip=%L61%
QSXVB4_Tooltip=%L62%
Gui,+Delimiter`n
Gui 2:+Delimiter`n

GoSub LoadIni
DllCall("SendMessage","UInt",hwnd_4,"UInt",247,"UInt",1,"UInt",Icon_9)
             
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - start gui2 when called without parameters
GoSub CreateGui2
if 0>0        
{             
 parameter=%1%
}             
if(parameter!="gui"){                                                         ; "gui" is used as parameter when called by tcmatch.dll
 GoSub ShowGui2
}             
              
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - start automatic show/hide
;GoSub QSWindowCheck
SetTimer,QSWindowCheck,100
return         
               
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - load ini settings
LoadIni:       
 Gui 1:Default 
 FileGetTime,IniTimestamp,%A_ScriptDir%\tcmatch.ini
 if(IniTimestamp==IniLastTimestamp){
  return       
 }             
 IniLastTimestamp=%IniTimestamp%
                
 if(INI_case_sensitive==""){
  IniRead,INI_case_sensitive ,%A_ScriptDir%\tcmatch.ini,general,case_sensitive ,0
  IniRead,INI_match_beginning,%A_ScriptDir%\tcmatch.ini,general,match_beginning,0
  IniRead,INI_override_search,%A_ScriptDir%\tcmatch.ini,gui    ,override_search,1
  IniRead,INI_invert_result  ,%A_ScriptDir%\tcmatch.ini,gui    ,invert_result  ,0
 }else{         
  IniRead,INI_case_sensitive ,%A_ScriptDir%\tcmatch.ini,general,case_sensitive ,%INI_case_sensitive%
  IniRead,INI_match_beginning,%A_ScriptDir%\tcmatch.ini,general,match_beginning,%INI_match_beginning%
  IniRead,INI_override_search,%A_ScriptDir%\tcmatch.ini,gui    ,override_search,%INI_override_search%
  IniRead,INI_invert_result  ,%A_ScriptDir%\tcmatch.ini,gui    ,invert_result  ,%INI_invert_result%
 }               
                 
 GuiControl,choose,VSearch,%INI_override_search%
 GuiControl,,VFavitems,`n%L5%`n`n
 FileRead,IniFile,%A_ScriptDir%\tcmatch.ini
                 
 IniSection=0
 Loop,parse,IniFile,`n,`r
 {
  if(RegExMatch(A_LoopField,"^\s*\[presets]\s*$")){
   IniSection=1
  }else if(RegExMatch(A_LoopField,"^\s*\[.*]\s*$")){
   IniSection=0
  }
  pos:=InStr(A_LoopField,"=")
  if(IniSection && pos){
   StringMid,LoopField,A_LoopField,pos+1
   GuiControl,,VFavitems,%LoopField%
  }
 }

 Icon:=INI_match_beginning ? Icon_2 : Icon_1
 DllCall("SendMessage","UInt",hwnd_1,"UInt",247,"UInt",1,"UInt",Icon)
 Icon:=INI_case_sensitive  ? Icon_4 : Icon_3
 DllCall("SendMessage","UInt",hwnd_2,"UInt",247,"UInt",1,"UInt",Icon)
 Icon:=INI_invert_result   ? Icon_6 : Icon_5
 DllCall("SendMessage","UInt",hwnd_3,"UInt",247,"UInt",1,"UInt",Icon)
return

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - handle events gui1
LMatchBeginning:
 INI_match_beginning:=INI_match_beginning ? 0 : 1
 IniWrite,%INI_match_beginning%,%A_ScriptDir%\tcmatch.ini,general,match_beginning
 Icon:=INI_match_beginning ? Icon_2 : Icon_1
 DllCall("SendMessage","UInt",hwnd_1,"UInt",247,"UInt",1,"UInt",Icon)
 ActivateTC("^s{space}{backspace}")
return

LCaseSensitive:
 INI_case_sensitive:=INI_case_sensitive ? 0 : 1
 IniWrite,%INI_case_sensitive%,%A_ScriptDir%\tcmatch.ini,general,case_sensitive
 Icon:=INI_case_sensitive ? Icon_4 : Icon_3
 DllCall("SendMessage","UInt",hwnd_2,"UInt",247,"UInt",1,"UInt",Icon)
 ActivateTC("^s{space}{backspace}")
return

LInvert:
 INI_invert_result:=INI_invert_result ? 0 : 1
 IniWrite,%INI_invert_result%,%A_ScriptDir%\tcmatch.ini,gui,invert_result
 Icon:=INI_invert_result ? Icon_6 : Icon_5
 DllCall("SendMessage","UInt",hwnd_3,"UInt",247,"UInt",1,"UInt",Icon)
 ActivateTC("^s{space}{backspace}")
return

LSearch:
 Gui 1:Default
 GuiControlGet,INI_override_search,,VSearch
 IniWrite,%INI_override_search%,%A_ScriptDir%\tcmatch.ini,gui,override_search
 ActivateTC("^s{space}{backspace}")
return

LFavitems:
 Gui 1:Default
 GuiControlGet,Favitem,,VFavitems
 GuiControl,choose,VFavitems,1
 ActivateTC("^s+{home}" . Favitem)
return

ActivateTC(Keys)
{
 global QSDelay
 QSDelay=3
 WinActivate ahk_class TTOTAL_CMD
 WinWaitActive ahk_class TTOTAL_CMD,,2
 if(!ErrorLevel){
  send %Keys%
 }
}

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - automatic show/hide functions
WM_ACTIVATE(wParam,lParam,msg,hwnd)
{
 global isWinActive
 isWinActive=%wParam%
}

QSWindowCheck:
 if(QSvisible==1){ ; tooltips
  MouseGetPos,newmx,newmy
  if(Messagex!=newmx || Messagey!=newmy){
   newMessage=
   Tooltip
  }
  if(newmx!=oldmx || newmy!=oldmy){
   oldmx=%newmx%
   oldmy=%newmy%
   Tooltip   
   mz=0
  }else{
   if(mz=4 && newMessage!=""){
    newmx:=newmx+20
    Tooltip,%newMessage%,%newmx%,%newmy%
   }
   mz++
  }
 }
 if(QSvisible==0 && INI_override_search!=0 && WinExist("ahk_class TQUICKSEARCH")){
  gosub showQSX
 }else if(QSvisible==1 && !WinExist("ahk_class TQUICKSEARCH") && !isWinActive){
  if(QSDelay){
   QSDelay--
  }else{
   gosub hideQSX
  }
 }else if(QSvisible!=2 && !WinExist("ahk_class TTOTAL_CMD")){
  ExitApp  
 }
return

WM_MOUSEOVER()
{
 global
 MouseGetPos,Messagex,Messagey
 if A_GuiControl in QSXVB1,QSXVB2,QSXVB3,QSXVB4
 {
  MessageVar=%A_GuiControl%_Tooltip
  newMessage:=%MessageVar%
 }
}

showQSX:
 Gui 2:Default
 Gui,hide
 Gui 1:Default
 WinGetPos,posX,posY,posDX,posDY,ahk_class TQUICKSEARCH
 GoSub LoadIni
 posx:=posx+posdx+5
 if(INI_one_line_gui==0){
  posy:=posy-25
  gui,Show,X%posX% Y%posY% w183 h50 NoActivate
 }else{
  if(INI_show_presets==1){
   gui,Show,X%posX% Y%posY% w361 h24 NoActivate
  }else{
   gui,Show,X%posX% Y%posY% w224 h24 NoActivate
  }
 }
 QSvisible=1
 QSDelay=0
return

hideQSX:
 Gui 1:Default
 gui,hide
 Tooltip
 QSvisible=0
return

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - create gui2
CreateGui2:
 Gui 2:Default
 Gui,add,tab2    ,              w450 h415 +Theme -Background,%L6%`n`n%L7%`n%L8%`n%L9%`n%L48%
 
; - - - - - tab #1
 Gui,add,groupbox,x+10   y+10   w425 h150 section            ,%L10%
 Gui,add,checkbox,xp+10  yp+25            vC01 gL01          ,%L11%
 Gui,add,checkbox,                        vC02 gL02          ,%L12%
 Gui,add,checkbox,                        vC03 gL03          ,%L13%
 Gui,add,checkbox,                        vC04 gL04          ,%L14%
 Gui,add,checkbox,                        vC05 gL05          ,%L15%
 Gui,add,checkbox,                        vC06 gL06          ,%L16%
 Gui,add,groupbox,xs     ys+160 w425 h150 section            ,%L17%
 Gui,add,checkbox,xp+10  yp+25            vC07 gL07          ,%L18%
 Gui,add,checkbox,                        vC08 gL08          ,%L19%
 Gui,add,checkbox,                        vCa9 gLa9          ,%L46%
 Gui,add,text    ,       yp+30                               ,%L47%
 Gui,add,dropdownlist,          w80       vCaa gLaa altsubmit,English`nDeutsch
                                                         
; - - - - - tab #2                                       
 Gui,tab,2                                               
 Gui,add,groupbox,x+10   y+10   w425 h180 section            ,%L20%
 Gui,add,Edit    ,xp+10  yp+25  w20  h21  vC09 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL09               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L22%
 Gui,add,Edit    ,xp-85  yp+25  w20  h21  vC10 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL10               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L23%
 Gui,add,Edit    ,xp-85  yp+25  w20  h21  vC11 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL11               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L24%
 Gui,add,Edit    ,xp-85  yp+25  w20  h21  vC12 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL12               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L25%
 Gui,add,Edit    ,xp-85  yp+25  w20  h21  vC13 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL13               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L26%
 Gui,add,groupbox,xs     ys+190 w425 h180 section            ,%L27%
 Gui,add,Edit    ,xp+10  yp+25  w20  h21  vC14 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL14               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L28%
 Gui,add,Edit    ,xp-85  yp+25  w20  h21  vC15 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL15               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L29%
 Gui,add,Edit    ,xp-85  yp+25  w20  h21  vC16 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL16               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L30%
 Gui,add,Edit    ,xp-85  yp+25  w20  h21  vC17 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL17               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L31%
 Gui,add,Edit    ,xp-85  yp+25  w20  h21  vC18 Disabled      ,
 Gui,add,button  ,xp+25  yp-1        h23  gL18               ,%L21%
 Gui,add,text    ,xp+60  yp+5                                ,%L32%

; - - - - - tab #3
 Gui,tab,3
 Gui,add,text    ,x+10   y+15                                ,%L36%
 Gui,add,text    ,xp+50  yp                                  ,%L37%
 Gui,add,Edit    ,xp-50  yp+18  w20  h21  vC19 limit1        ,
 Gui,add,Edit    ,xp+50  yp     w290 h21  vC20               ,
 Gui,add,Button  ,xp+300 yp-1             gAddListEntry1     ,%L38%
 Gui,add,text    ,xp-350 yp+35                               ,%L39%
 Gui,add,ListBox ,              w425 h300 vC21 gListEntryChanged1 sort t25,

; - - - - - tab #4
 Gui,tab,4
 Gui,add,text    ,x+10   y+15                                ,%L40%
 Gui,add,text    ,xp+250 yp                                  ,%L41%
 Gui,add,Edit    ,xp-250 yp+18  w240 h21  vC22               ,
 Gui,add,Edit    ,xp+250 yp     w90  h21  vC23               ,
 Gui,add,Button  ,xp+100 yp-1             gAddListEntry2     ,%L38%
 Gui,add,text    ,xp-350 yp+35                               ,%L42%
 Gui,add,ListBox ,              w425 h300 vC24 gListEntryChanged2 sort t128 t160 t192,
 
; - - - - - tab #5
 Gui,tab,5
 Gui,add,text    ,x+10   y+15             section            ,%L49%
 Gui,add,text    ,xp+280 yp                                  ,%L50%
 Gui,add,Edit    ,xp-280 yp+18  w245 h21  vC25 Disabled      ,
 Gui,add,Button  ,xp+248 yp-1                  gL25          ,...
 Gui,add,listbox ,xp+32  yp+1   w145 h100 vC26 Multi         ,
 Gui,add,text    ,xp-280 yp+55                               ,%L51%
 Gui,add,Edit    ,              w40            Limit2 Number ,
 Gui,add,UpDown  ,                        vC27 Range1-99     ,1
 Gui,add,Button  ,xp+155 yp-1             gUpdateListEntry3  ,%L52%
 Gui,add,Button  ,xp+50  yp               gAddListEntry3     ,%L38%
 Gui,add,text    ,xp-205 yp+35                               ,%L53%
 Gui,add,ListBox ,              w425 h130 vC28 gListEntryChanged3 sort t60 t195,
 Gui,add,groupbox,xs     ys+280 w425 h85  section            ,%L54%
 Gui,add,text    ,xp+10  yp+25                               ,%L55%
 Gui,add,dropdownlist,   yp+20  w175      vC29 gL29 altsubmit,%L56%`n`n%L57%`n%L58%`n%L59%
 Gui,add,text    ,xp+190 yp-20                               ,%L60%
 Gui,add,edit    ,              w70       vC30 gL30 Number   ,
 
 Gui,tab
 Gui,add,button,xm gShowHelp                                 ,%L43%
 Gui,add,button,xp+425 hwndhwnd_5 +64 h24 w24 gCloseGui2
 DllCall("SendMessage","UInt",hwnd_5,"UInt",247,"UInt",1,"UInt",Icon_8)
 
 Gui 3:Default
 Gui,+owner2 -MinimizeBox -MaximizeBox
 Gui,add,Text  ,                                             ,%L34%
 Gui,add,Edit  ,xp+30 yp+25 w20 h21 vC99 limit1              ,
 Gui,add,Button,xp+30 yp-1          gHideGui3                ,%L35%
return

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - show gui2
ShowGui2:
 GoSub hideQSX
 QSvisible=2
 Gui 2:Default
 GoSub LoadIni2
 Gui,show,,QuickSearch eXtended %Version% © Samuel Plentz
 if(!V07){
  GuiControl 2:+Disabled,C08
  GuiControl 2:+Disabled,Ca9
 }
 if(!V08){
  GuiControl 2:+Disabled,Ca9
 }
return

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - loadini gui2
LoadIni2:
 Gui 2:Default
 
; - - - - - tab #1
 IniRead,V01,%A_ScriptDir%\tcmatch.ini,general,match_beginning         ,0
 IniRead,V02,%A_ScriptDir%\tcmatch.ini,general,case_sensitive          ,0
 IniRead,V03,%A_ScriptDir%\tcmatch.ini,general,allow_empty_result      ,1
 IniRead,W04,%A_ScriptDir%\tcmatch.ini,general,filter_files_and_folders,3
 V04:=(W04==1) ? 0 : 1
 V05:=(W04==2) ? 0 : 1
 Chinese=0
 if(SubStr(A_Language,3)=="04"){
  Chinese=1
 }
 IniRead,V06,%A_ScriptDir%\tcmatch.ini,general,use_pinyin              ,%Chinese%
 IniRead,V07,%A_ScriptDir%\tcmatch.ini,gui    ,override_search         ,1
 IniRead,V08,%A_ScriptDir%\tcmatch.ini,gui    ,one_line_gui            ,0
 IniRead,Va9,%A_ScriptDir%\tcmatch.ini,gui    ,show_presets            ,1
 GuiControl,,C01,%V01%
 GuiControl,,C02,%V02%
 GuiControl,,C03,%V03%
 GuiControl,,C04,%V04%
 GuiControl,,C05,%V05%
 GuiControl,,C06,%V06%
 GuiControl,,C07,%V07%
 GuiControl,,C08,%V08%
 GuiControl,,Ca9,%Va9%
 if(INI_language=="3"){
  GuiControl,Choose,Caa,2
 }else{
  GuiControl,Choose,Caa,1
 }

; - - - - - tab #2
 IniRead,V09,%A_ScriptDir%\tcmatch.ini,general,simple_search_activate_char                ,%A_Space%
 IniRead,V10,%A_ScriptDir%\tcmatch.ini,general,simple_search_match_beginning_activate_char,^
 IniRead,V11,%A_ScriptDir%\tcmatch.ini,general,regex_search_activate_char                 ,?
 IniRead,V12,%A_ScriptDir%\tcmatch.ini,general,leven_search_activate_char                 ,<
 IniRead,V13,%A_ScriptDir%\tcmatch.ini,general,srch_activate_char                         ,*
 IniRead,V14,%A_ScriptDir%\tcmatch.ini,general,and_separator_char                         ,
 IniRead,V15,%A_ScriptDir%\tcmatch.ini,general,or_separator_char                          ,|
 IniRead,V16,%A_ScriptDir%\tcmatch.ini,general,negate_char                                ,!
 IniRead,V17,%A_ScriptDir%\tcmatch.ini,general,preset_activate_char                       ,>
 IniRead,V18,%A_ScriptDir%\tcmatch.ini,general,wdx_separator_char                         ,/
 V09:=ConvertChar(V09,1)
 V10:=ConvertChar(V10,1)
 V11:=ConvertChar(V11,1)
 V12:=ConvertChar(V12,1)
 V13:=ConvertChar(V13,1)
 V14:=ConvertChar(V14,1)
 V15:=ConvertChar(V15,1)
 V16:=ConvertChar(V16,1)
 V17:=ConvertChar(V17,1)
 V18:=ConvertChar(V18,1)
 GuiControl,,C09,%V09%
 GuiControl,,C10,%V10%
 GuiControl,,C11,%V11%
 GuiControl,,C12,%V12%
 GuiControl,,C13,%V13%
 GuiControl,,C14,%V14%
 GuiControl,,C15,%V15%
 GuiControl,,C16,%V16%
 GuiControl,,C17,%V17%
 GuiControl,,C18,%V18%

; - - - - - tab #3
 GuiControl,,C21,`n
 PresetList=
 FileRead,IniFile,%A_ScriptDir%\tcmatch.ini
 IniSection=0
 Loop,parse,IniFile,`n,`r
 {
  if(RegExMatch(A_LoopField,"^\s*\[presets]\s*$")){
   IniSection=1
  }else if(RegExMatch(A_LoopField,"^\s*\[.*]\s*$")){
   IniSection=0
  }
  pos:=InStr(A_LoopField,"=")
  if(IniSection && pos){
   StringReplace,LoopField,A_LoopField,=,%A_Tab%
   PresetList=%PresetList%`n%LoopField%
  }
 }
 GuiControl,,C21,%PresetList%
 GuiControl,Choose,C21,1
 gosub ListEntryChanged1
 
; - - - - - tab #4
 GuiControl,,C24,`n
 ReplaceList=
 ReplaceCount=0
 Loop
 {
  IniRead,Entry,%A_ScriptDir%\tcmatch.ini,replace,chars%A_Index%,
  if(Entry=="ERROR"){
   break
  }
  StringReplace,Entry,Entry,|,%A_Tab%
  ReplaceList=%ReplaceList%`n%Entry%
  ReplaceCount=%A_Index%
 }
 GuiControl,,C24,%ReplaceList%
 GuiControl,Choose,C24,1
 gosub ListEntryChanged2
  
; - - - - - tab #5
 IniRead,V29,%A_ScriptDir%\tcmatch.ini,wdx,debug_output,1
 IniRead,V30,%A_ScriptDir%\tcmatch.ini,wdx,wdx_cache   ,1000
 V29++
 GuiControl,Choose,C29,%V29%
 GuiControl,,C30,%V30%
 GuiControl,,C28,`n
 wdxListA=
 wdxListB=
 Loop 100
 {
  wdxListC%A_Index%:=1
 }
 IniRead,VGroups,%A_ScriptDir%\tcmatch.ini,wdx,wdx_groups,%A_Space%
 StringSplit,VGroups,VGroups,|
 Loop %VGroups0%
 {
  VGroup:=VGroups%A_Index%
  Index=%A_Index%
  StringSplit,VGroup,VGroup,`,
  Loop %VGroup0%
  {
   Item:=VGroup%A_Index%
   wdxListC%Item%:=Index
  }
 }
 Loop
 {
  IniRead,Entry,%A_ScriptDir%\tcmatch.ini,wdx,wdx_text_plugin%A_Index%,
  if(Entry=="ERROR"){
   break
  }
  Group:=wdxListC%A_Index%
  wdxListB=%wdxListB%`n%Entry%
  StringSplit,Entry_,Entry,@
  SplitPath,Entry_2,Filename
  StringReplace,Entry_1,Entry_1,|,`,%A_Space%,1
  wdxListA=%wdxListA%`n%Filename%%A_Tab%%Entry_1%%A_Tab%%Group%
 }
 GuiControl,,C28,%wdxListA%
 GuiControl,Choose,C28,1
 gosub ListEntryChanged3

 Hotkey,IfWinActive,QuickSearch eXtended %Version% © Samuel Plentz
 Hotkey,~Delete,DeleteListEntry
return

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - handle events gui2
ShowHelp:
 if(INI_language==3 && FileExist(A_ScriptDir . "\tcmatch_de.pdf")){
  run,%A_ScriptDir%\tcmatch_de.pdf
 }else if(FileExist(A_ScriptDir . "\tcmatch.pdf")){
  run,%A_ScriptDir%\tcmatch.pdf
 }else{
  msgbox,8240,%L44%,%L45%`n%A_ScriptDir%\tcmatch.pdf
 }
return

; - - - - - tab #1
L01:
 V01:=(V01!=0) ? 0 : 1
 IniWrite,%V01%,%A_ScriptDir%\tcmatch.ini,general,match_beginning
return

L02:
 V02:=(V02!=0) ? 0 : 1
 IniWrite,%V02%,%A_ScriptDir%\tcmatch.ini,general,case_sensitive
return

L03:
 V03:=(V03!=0) ? 0 : 1
 IniWrite,%V03%,%A_ScriptDir%\tcmatch.ini,general,allow_empty_result
return

L04:
 V04:=(V04!=0) ? 0 : 1
 if(V04==0){
  GuiControl 2:,C05,1
  V05=1
 }
 W04:=2*V05+V04
 IniWrite,%W04%,%A_ScriptDir%\tcmatch.ini,general,filter_files_and_folders
return

L05:
 V05:=(V05!=0) ? 0 : 1
 if(V05==0){
  GuiControl 2:,C04,1
  V04=1
 }
 W04:=2*V05+V04
 IniWrite,%W04%,%A_ScriptDir%\tcmatch.ini,general,filter_files_and_folders
return

L06:
 V06:=(V06!=0) ? 0 : 1
 IniWrite,%V06%,%A_ScriptDir%\tcmatch.ini,general,use_pinyin
return

L07:
 V07:=(V07!=0) ? 0 : 1
 IniWrite,%V07%,%A_ScriptDir%\tcmatch.ini,gui,override_search
 INI_override_search=%V07%
 if(!V07){
  GuiControl 2:+Disabled,C08
  GuiControl 2:+Disabled,Ca9
 }else{
  GuiControl 2:-Disabled,C08
  if(V08){
   GuiControl 2:-Disabled,Ca9
  }
 }
return

L08:
 V08:=(V08!=0) ? 0 : 1
 IniWrite,%V08%,%A_ScriptDir%\tcmatch.ini,gui,one_line_gui
 INI_one_line_gui=%V08%
 if(!V08){
  GuiControl 2:+Disabled,Ca9
 }else{
  GuiControl 2:-Disabled,Ca9
 }
 GoSub LMoveWindow
return

La9:
 Va9:=(Va9!=0) ? 0 : 1
 IniWrite,%Va9%,%A_ScriptDir%\tcmatch.ini,gui,show_presets
 INI_show_presets=%Va9%
 GoSub LMoveWindow
return

LMoveWindow:
 Gui 1:Default
 if(INI_one_line_gui=="0"){
  GuiControl,enable,VFavitems
  GuiControl,enable,VFavicon
  GuiControl,move,QSXVB3   ,x0  y26
  GuiControl,move,QSXVB4   ,x26 y26
  GuiControl,move,VSearch  ,x52 y1 w130
  GuiControl,move,VFavicon ,x52 y30
  GuiControl,move,VFavitems,x72 y27
 }else{
  if(INI_show_presets!=0){
   GuiControl,enable,VFavitems
   GuiControl,enable,VFavicon
   GuiControl,move,QSXVB4   ,x337 y0
   GuiControl,move,VFavicon ,x200 y4
   GuiControl,move,VFavitems,x220 y1
  }else{
   GuiControl,move,QSXVB4   ,x200 y0
   GuiControl,move,VFavitems,x0 y100
   GuiControl,move,VFavicon ,x0 y100
   GuiControl,disable,VFavitems
   GuiControl,disable,VFavicon
  }
  GuiControl,move,QSXVB3 ,x52 y0
  GuiControl,move,VSearch,x83 y1 w110
 }
return

Laa:
 GuiControlGet,INI_language,,Caa
 INI_language++
 IniWrite,%INI_language%,%A_ScriptDir%\tcmatch.ini,gui,language
return

; - - - - - tab #2
L09:
 CurItem=9
 Gosub ShowGui3
return

L10:
 CurItem=10
 Gosub ShowGui3
return

L11:
 CurItem=11
 Gosub ShowGui3
return

L12:
 CurItem=12
 Gosub ShowGui3
return

L13:
 CurItem=13
 Gosub ShowGui3
return

L14:
 CurItem=14
 Gosub ShowGui3
return

L15:
 CurItem=15
 Gosub ShowGui3
return

L16:
 CurItem=16
 Gosub ShowGui3
return

L17:
 CurItem=17
 Gosub ShowGui3
return

L18:
 CurItem=18
 Gosub ShowGui3
return

ShowGui3:
 Gui 2:+Disabled
 Gui 3:Default
 GuiControl,,C99,
 GuiControl,Focus,C99
 Gui 3:show,,%L33%
return

HideGui3:
 Gui 3:Default
 GuiControlGet,NewChar,,C99
 ConvertedChar:=ConvertChar(NewChar,1)
 Char4File    :=ConvertChar(NewChar,2)
 if(CurItem=="9"){
  GuiControl 2:,C09,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,simple_search_activate_char
 }else if(CurItem=="10"){
  GuiControl 2:,C10,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,simple_search_match_beginning_activate_char
 }else if(CurItem=="11"){
  GuiControl 2:,C11,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,regex_search_activate_char
 }else if(CurItem=="12"){
  GuiControl 2:,C12,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,leven_search_activate_char
 }else if(CurItem=="13"){
  GuiControl 2:,C13,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,srch_activate_char
 }else if(CurItem=="14" && ConvertedChar!=""){
  GuiControl 2:,C14,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,and_separator_char
 }else if(CurItem=="15" && ConvertedChar!=""){
  GuiControl 2:,C15,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,or_separator_char
 }else if(CurItem=="16" && ConvertedChar!=""){
  GuiControl 2:,C16,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,negate_char
 }else if(CurItem=="17" && ConvertedChar!=""){
  GuiControl 2:,C17,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,preset_activate_char
 }else if(CurItem=="18" && ConvertedChar!=""){
  GuiControl 2:,C18,%ConvertedChar%
  IniWrite,%Char4File%,%A_ScriptDir%\tcmatch.ini,general,wdx_separator_char
 }
3guiClose:
3guiEscape:
 Gui 2:-Disabled
 Gui 3:hide
return

ConvertChar(Char,Mode)
{
 if(Mode==1){
  if(Char=="ERROR" or Char==" "){
   return """ """
  }else if(Char==""){
   return ""
  }else{
   return " " . Char
  }
 }else if(Mode==2){
  if(Char==" "){
   return """ """
  }else{
   return Char
  }
 }
}

; - - - - - tab #3
AddListEntry1:
 GuiControlGet,Key,  2:,C19
 GuiControlGet,Value,2:,C20
 if(StrLen(Key)!=1 || StrLen(Value)<1){
  return
 }
 newPresetList=
 Loop,parse,PresetList,`n
 {
  if(A_LoopField!="" && InStr(A_LoopField,Key . A_Tab)!=1){
   newPresetList=%newPresetList%`n%A_LoopField%
  }
 }
 PresetList=%newPresetList%`n%Key%%A_Tab%%Value%
 IniWrite,%Value%,%A_ScriptDir%\tcmatch.ini,presets,%Key%
 GuiControl 2:,C21,%PresetList%
 GuiControl,2:ChooseString,C21,%Key%%A_Tab%%Value%
 gosub ListEntryChanged1
return

ListEntryChanged1:
 GuiControlGet,Value,2:,C21
 StringLeft,Key,Value,1
 StringTrimLeft,Value,Value,2
 GuiControl 2:,C19,%Key%
 GuiControl 2:,C20,%Value%
return

; - - - - - tab #4
AddListEntry2:
 GuiControlGet,Key,  2:,C22
 GuiControlGet,Value,2:,C23
 if(StrLen(Key)<1 || StrLen(Value)<1){
  return
 }
 ReplaceList=%ReplaceList%`n%Key%%A_Tab%%Value%
 ReplaceCount++
 IniWrite,%Key%|%Value%,%A_ScriptDir%\tcmatch.ini,replace,chars%ReplaceCount%
 GuiControl 2:,C24,%ReplaceList%
 GuiControl,2:ChooseString,C24,%Key%%A_Tab%%Value%
 gosub ListEntryChanged2
return

ListEntryChanged2:
 GuiControlGet,Value,2:,C24
 Position:=InStr(Value,A_Tab)
 StringLeft,Key,Value,Position-1
 StringTrimLeft,Value,Value,Position
 GuiControl 2:,C22,%Key%
 GuiControl 2:,C23,%Value%
return

; - - - - - tab #5
UpdateListEntry3:
 GuiControlGet,Filepath,2:,C25
 GuiControlGet,Fields,2:,C26
 GuiControlGet,Value,2:,C28
 if(Filepath=="" || Fields=="" || Value==""){
  return
 }
 StringSplit,wdxListA,wdxListA,`n
 StringSplit,wdxListB,wdxListB,`n
 wdxListA=
 wdxListB=
 Found=0
 Loop %wdxListA0%
 {
  IndexM1:=A_Index-1
  ValueA:=wdxListA%A_Index%
  ValueB:=wdxListB%A_Index%
  if(ValueA==Value && Found==0){
   Found=1
  }else if(ValueA!=""){
   IndexM2:=IndexM1-Found
   wdxListA=%wdxListA%`n%ValueA%
   wdxListB=%wdxListB%`n%ValueB%
   IniWrite,%ValueB%,%A_ScriptDir%\tcmatch.ini,wdx,wdx_text_plugin%IndexM2%
  }
  if(Found==1){
   wdxListC%IndexM1%:=wdxListC%A_Index%
  }
 }
 IniDelete,%A_ScriptDir%\tcmatch.ini,wdx,wdx_text_plugin%IndexM1%
 gosub AddListEntry3
return

AddListEntry3:
 GuiControlGet,Filepath,2:,C25
 GuiControlGet,Fields,2:,C26
 GuiControlGet,Group,2:,C27
 if(Filepath=="" || Fields==""){
  return
 }
 StringSplit,wdxListA,wdxListA,`n
 SplitPath,Filepath,Filename
 wdxListC%wdxListA0%:=Group
 StringReplace,Fields,Fields,`n,|,1
 wdxListB=%wdxListB%`n%Fields%@%Filepath%
 IniWrite,%Fields%@%Filepath%,%A_ScriptDir%\tcmatch.ini,wdx,wdx_text_plugin%wdxListA0%
 StringReplace,Fields,Fields,|,`,%A_Space%,1
 wdxListA=%wdxListA%`n%Filename%%A_Tab%%Fields%%A_Tab%%Group%
 wdxListC=
 Loop,100
 {
  B_Index=%A_Index%
  wdxListCpart=
  Loop,%wdxListA0%
  {
   ValueC:=wdxListC%A_Index%
   if(B_Index==ValueC){
    if(wdxListCpart=""){
     wdxListCpart=%A_Index%
    }else{
     wdxListCpart=%wdxListCpart%,%A_Index%
    }
   }
  }
  wdxListC=%wdxListC%%wdxListCpart%|
 }
 while(SubStr(wdxListC,StrLen(wdxListC))=="|"){
  StringTrimRight,wdxListC,wdxListC,1
 }
 IniWrite,%wdxListC%,%A_ScriptDir%\tcmatch.ini,wdx,wdx_groups
 GuiControl,2:,C28,`n
 GuiControl,2:,C28,%wdxListA%
 GuiControl,2:Choose,C28,1
 gosub ListEntryChanged3
return

StrPutVar(Str,@){
 return StrPut(Str,@,"cp1252")
}

wdxLoadFields:
 Critical,On
 GuiControlGet,wdxFilename,2:,C25
 wdxListD=
 if(FileExist(wdxFilename)){
  wdxFile:=DllCall("LoadLibrary","str",wdxFilename)
  if(errorlevel!=0){
   msgbox,Error in LoadLibrary:`n%wdxFilename%`nErrorlevel: %errorlevel%`nLast Error: %A_LastError%
  }else{
   setVersion:=DllCall("GetProcAddress",uint,wdxFile,astr,"ContentSetDefaultParams")
   if(errorlevel!=0){
    msgbox,Error in GetProcAddress_ContentSetDefaultParams:`n%wdxFilename%`nErrorlevel: %errorlevel%`nLast Error: %A_LastError%
   }
   getFields:=DllCall("GetProcAddress",uint,wdxFile,astr,"ContentGetSupportedField")
   if(errorlevel!=0){
    msgbox,Error in GetProcAddress_ContentGetSupportedField:`n%wdxFilename%`nErrorlevel: %errorlevel%`nLast Error: %A_LastError%
   }
   unloadPlugin:=DllCall("GetProcAddress",uint,wdxFile,astr,"ContentPluginUnloading")
   if(errorlevel!=0){
    msgbox,Error in GetProcAddress_ContentPluginUnloading:`n%wdxFilename%`nErrorlevel: %errorlevel%`nLast Error: %A_LastError%
   }
   if(setVersion!=0){
    VersionFileName := A_ScriptDir . "\contplug.ini"
    Size:=12+StrLen(VersionFileName)+1
    VarSetCapacity(VersionStruct,Size,0)
    NumPut(Size,VersionStruct,0,"Int")
    NumPut(2   ,VersionStruct,4,"UInt")
    NumPut(0   ,VersionStruct,8,"UInt")
    StrPutVar(VersionFileName,&VersionStruct+12)
    DllCall(setVersion,"uint",&VersionStruct)
    if(errorlevel!=0){
     msgbox,Error in ContentSetDefaultParams:`n%wdxFilename%`nErrorlevel: %errorlevel%`nLast Error: %A_LastError%
    }
   }
   Loop
   {
    IndexM1:=A_Index-1
    Size=1024
    FieldNameA=
    UnitsA=
    VarSetCapacity(FieldNameA,Size+1)
    VarSetCapacity(UnitsA,Size+1)
    VarSetCapacity(FieldName,(Size+1)*2)
    result:=DllCall(getFields,"int",IndexM1,"UInt",&FieldNameA,"UInt",&UnitsA,"int",Size)
    FieldName:=StrGet(&FieldNameA,"cp1252")
    if(errorlevel!=0){
     msgbox,Error in ContentGetSupportedField:`n%wdxFilename%`nErrorlevel: %errorlevel%`nLast Error: %A_LastError%
     break
    }
    if(result==0){
     break
    }
    wdxListD=%wdxListD%`n%FieldName%
   }
   if(unloadPlugin!=0){
    DllCall(unloadPlugin)
    if(errorlevel!=0){
     msgbox,Error in ContentPluginUnloading:`n%wdxFilename%`nErrorlevel: %errorlevel%`nLast Error: %A_LastError%
    }
   }
   DllCall("FreeLibrary","UInt",wdxFile)
   if(errorlevel!=0){
    msgbox,Error in FreeLibrary:`n%wdxFilename%`nErrorlevel: %errorlevel%`nLast Error: %A_LastError%
   }
  }
 }
 GuiControl 2:,C26,`n
 if(wdxListD==""){
  GuiControl 2:,C25,
 }else{
  GuiControl 2:,C26,%wdxListD%
 }
 Critical,Off
return

ListEntryChanged3:
 GuiControlGet,Value,2:,C28
 StringSplit,wdxListA,wdxListA,`n
 StringSplit,wdxListB,wdxListB,`n
 Loop %wdxListA0%
 {
  IndexM1:=A_Index-1
  ValueA:=wdxListA%A_Index%
  ValueB:=wdxListB%A_Index%
  ValueC:=wdxListC%IndexM1%
  if(ValueA==Value){
   StringSplit,ValueB,ValueB,@
   GuiControl 2:,C25,%ValueB2%
   GuiControl 2:,C27,%ValueC%`naaa
   GoSub wdxLoadFields
   Loop,parse,ValueB1,|
   {
    GuiControl 2:Choose,C26,%A_LoopField%
   }
   return
  }
 }
return

DeleteListEntry:
 ControlGetFocus,focusedControl
 if(focusedControl=="ListBox1"){
  GuiControlGet,Value,2:,C21
  if(Value!=""){
   StringLeft,Key,Value,1
   IniDelete,%A_ScriptDir%\tcmatch.ini,presets,%Key%
   StringReplace,PresetList,PresetList,`n%Value%,,All
   GuiControl 2:,C21,`n
   GuiControl 2:,C21,%PresetList%
   GuiControl,2:Choose,C21,1
   gosub ListEntryChanged1
  }
 }else if(focusedControl=="ListBox2"){
  GuiControlGet,Value,2:,C24
  if(Value!=""){
   StringReplace,ReplaceList,ReplaceList,`n%Value%
   IniDelete,%A_ScriptDir%\tcmatch.ini,replace,chars%ReplaceCount%
   ReplaceCount--
   Loop,parse,ReplaceList,`n
   {
    if(A_LoopField!=""){
     StringReplace,Value,A_LoopField,%A_Tab%,|
     Index:=A_Index-1
     IniWrite,%Value%,%A_ScriptDir%\tcmatch.ini,replace,chars%Index%
    }
   }
   GuiControl 2:,C24,`n
   GuiControl 2:,C24,%ReplaceList%
   GuiControl,2:Choose,C24,1
   gosub ListEntryChanged2
  }
 }else if(focusedControl=="ListBox4"){
  GuiControlGet,Value,2:,C28
  if(Value!=""){
   StringSplit,wdxListA,wdxListA,`n
   StringSplit,wdxListB,wdxListB,`n
   wdxListA=
   wdxListB=
   Found=0
   Loop %wdxListA0%
   {
    IndexM1:=A_Index-1
    ValueA:=wdxListA%A_Index%
    ValueB:=wdxListB%A_Index%
    if(ValueA==Value && Found==0){
     Found=1
    }else if(ValueA!=""){
     IndexM2:=IndexM1-Found
     wdxListA=%wdxListA%`n%ValueA%
     wdxListB=%wdxListB%`n%ValueB%
     IniWrite,%ValueB%,%A_ScriptDir%\tcmatch.ini,wdx,wdx_text_plugin%IndexM2%
    }
    if(Found==1){
     wdxListC%IndexM1%:=wdxListC%A_Index%
    }
   }
   IniDelete,%A_ScriptDir%\tcmatch.ini,wdx,wdx_text_plugin%IndexM1%
   IndexM1--
   wdxListC=
   Loop,100
   {
    B_Index=%A_Index%
    wdxListCpart=
    Loop,%IndexM1%
    {
     ValueC:=wdxListC%A_Index%
     if(B_Index==ValueC){
      if(wdxListCpart=""){
       wdxListCpart=%A_Index%
      }else{
       wdxListCpart=%wdxListCpart%,%A_Index%
      }
     }
    }
    wdxListC=%wdxListC%%wdxListCpart%|
   }
   while(SubStr(wdxListC,StrLen(wdxListC))=="|"){
    StringTrimRight,wdxListC,wdxListC,1
   }
   IniWrite,%wdxListC%,%A_ScriptDir%\tcmatch.ini,wdx,wdx_groups
   GuiControl,2:,C28,`n
   GuiControl,2:,C28,%wdxListA%
   GuiControl,2:Choose,C28,1
   gosub ListEntryChanged3
  }
 }
return

L25:
 GuiControlGet,oldwdxfile,2:,C25
 Gui 2:+OwnDialogs 
 FileSelectFile,wdxfile,3,%oldwdxfile%,%L49%,%L48% (*.wdx)
 StringRight,wdxext,wdxfile,4
 if(wdxfile!="" && wdxext==".wdx"){
  GuiControl 2:,C25,%wdxfile%
  GoSub wdxLoadFields
  GuiControl 2:Choose,C26,1
 }
return

L29:
 GuiControlGet,V29,2:,C29
 V29--
 IniWrite,%V29%,%A_ScriptDir%\tcmatch.ini,wdx,debug_output
return

L30:
 GuiControlGet,V30,2:,C30
 IniWrite,%V30%,%A_ScriptDir%\tcmatch.ini,wdx,wdx_cache
return

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - close gui2
CloseGui2:
 Gui 2:Default
 Gui,hide
 QSvisible=0
 ActivateTC("^s{space}{backspace}")
 if(INI_override_search==0){
  ExitApp  
 }
return

2guiClose:
2guiEscape:
 Gui 2:Default
 Gui,hide
 QSvisible=0
 if(INI_override_search==0){
  ExitApp  
 }
return

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - exit script
; #x::
;  ExitApp
; return
; 
; #IfWinActive ahk_class TfPSPad.UnicodeClass
; F5::
;  ExitApp
; return
; #IfWinActive
