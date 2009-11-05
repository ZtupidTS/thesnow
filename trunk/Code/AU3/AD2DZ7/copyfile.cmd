@echo off
md E:\script\src
md E:\script\forum
copy *.au3 E:\script\src /y
copy *.exe E:\script\ /y
copy site.* E:\script\ /y
copy forum\*.* e:\script\forum\*.* /y
echo OK!
pause >nul