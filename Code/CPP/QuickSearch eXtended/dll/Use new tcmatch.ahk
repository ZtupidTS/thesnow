WinClose ahk_class TTOTAL_CMD
WinWaitClose

Loop
{
 FileDelete C:\Programme\totalcmd\tcmatch.dll
 if(!FileExist("C:\Programme\totalcmd\tcmatch.dll")) {
 	break
 }
 Sleep,50
}

FileCopy %A_ScriptDir%\tcmatch.dll,C:\Programme\totalcmd\tcmatch.dll
Run C:\Programme\totalcmd\TOTALCMD.EXE,C:\Programme\totalcmd\