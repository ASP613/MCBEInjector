@echo off

REM assumes running in modern x64 native tools cmd

cl /c /O1 /Oi- /GS- createproc.c
cl /c /O1 /Oi- /GS- crt_helpers.c
cl /c /O1 /Oi- /GS- gui.c
cl /c /O1 /Oi- /GS- inject.c

rc Resource.rc

link createproc.obj crt_helpers.obj gui.obj inject.obj Resource.res user32.lib kernel32.lib ole32.lib uuid.lib comdlg32.lib ^
/NODEFAULTLIB ^
/ENTRY:WinMainCRTStartup ^
/SUBSYSTEM:WINDOWS ^
/RELEASE ^
/DEBUG:NONE ^
/ALIGN:16 ^
/OUT:MCBEInject.exe