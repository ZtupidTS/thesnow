dim mx,url
set mx=wscript.createObject("MxDownloadServer.ThunderMx")
url=inputbox ("������URL","������һ��URL")
mx.CallAddTask url, "��������ע��", "", 0, "", ""
