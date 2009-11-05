dim mx,obj
set mx=wscript.createObject("MxDownloadServer.ThunderMx")
set obj=wscript.createObject("wscript.shell")
mx.InitDownloader
WScript.Sleep(2000)
obj.Run "down.htm"