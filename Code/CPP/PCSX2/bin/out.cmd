@echo off
rd /s /q out
md out
md out\bios
md out\inis
md out\logs
md out\memcards
md out\patches
md out\plugins
md out\snaps
md out\sstates
copy bios\*.* out\bios\*.*
copy inis\*.* out\inis\*.*
copy memcards\*.* out\memcards\*.*
copy patches\*.* out\patches\*.*
copy plugins\*.dll out\plugins\*.dll
copy *.dll out\*.dll
copy *.exe out\*.exe