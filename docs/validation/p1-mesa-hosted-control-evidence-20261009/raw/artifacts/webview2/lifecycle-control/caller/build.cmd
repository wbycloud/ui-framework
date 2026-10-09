@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cl /nologo /W4 /WX /utf-8 /std:c11 /O2 /Z7 /MT "D:\a\ui-framework\ui-framework\tests\osmesa_thread_probe.c" /link user32.lib /DEBUG /OUT:osmesa_thread_probe.exe /PDB:osmesa_thread_probe.pdb
exit /b %errorlevel%
