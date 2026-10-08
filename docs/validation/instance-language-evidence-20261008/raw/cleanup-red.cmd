@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cmake --build build/language-20261008/after --target ui_language_core_test > build/language-20261008/cleanup-red-build.log 2>&1
if errorlevel 1 exit /b 1
build\language-20261008\after\ui_language_core_test.exe > build/language-20261008/cleanup-red.log 2>&1
exit /b %errorlevel%
