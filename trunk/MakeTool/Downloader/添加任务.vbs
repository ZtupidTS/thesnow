dim mx,url
set mx=wscript.createObject("MxDownloadServer.ThunderMx")
url=inputbox ("请输入URL","请输入一个URL")
mx.CallAddTask url, "这里输入注释", "", 0, "", ""
