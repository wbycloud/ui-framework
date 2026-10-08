@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cl /nologo /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED /Iinclude build/ui-repair-20261009/help-red-bin/light_scroll.c /Febuild/ui-repair-20261009/help-red-bin/help.exe /Fobuild/ui-repair-20261009/help-red-bin/help.obj /link build/ui-repair-20261009/sdk-unpacked/lib/ui_framework_runtime.lib user32.lib >build/ui-repair-20261009/help-red-build-fixed.log 2>&1
if errorlevel 1 exit /b 1
build\ui-repair-20261009\help-red-bin\help.exe >build/ui-repair-20261009/help-red.log 2>&1
exit /b %errorlevel%
