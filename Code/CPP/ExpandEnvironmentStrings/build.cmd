cl /Ox /GA /Fx /nologo /MT /c x.cpp
link /ENTRY:"WinMain" /OUT:thesnoW.exe /SUBSYSTEM:WINDOWS /NODEFAULTLIB x.obj user32.lib kernel32.lib /nologo