@echo off
color 0a
echo ����ע��        =====����������======
taskkill /f /im MxDownloadServer.exe
regsvr32 /s Modules\MxDownloader\download_interface.dll
start Modules\MxDownloader\MxDownloadServer.exe
