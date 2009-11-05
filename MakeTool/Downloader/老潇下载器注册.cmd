@echo off
color 0a
echo 正在注册        =====老潇下载器======
taskkill /f /im MxDownloadServer.exe
regsvr32 /s Modules\MxDownloader\download_interface.dll
start Modules\MxDownloader\MxDownloadServer.exe
