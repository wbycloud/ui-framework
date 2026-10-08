@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cl /nologo /W4 /WX /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED /Ibuild/ui-repair-20261009/sdk-unpacked/include build/ui-repair-20261009/sdk-consumer/consumer.c /Febuild/ui-repair-20261009/sdk-consumer/consumer.exe /Fobuild/ui-repair-20261009/sdk-consumer/consumer.obj /link build/ui-repair-20261009/sdk-unpacked/lib/ui_framework_runtime.lib >build/ui-repair-20261009/sdk-consumer-build.log 2>&1
if errorlevel 1 exit /b 1
copy /y build\ui-repair-20261009\sdk-unpacked\bin\ui_framework.dll build\ui-repair-20261009\sdk-consumer\ui_framework.dll >nul
build\ui-repair-20261009\sdk-consumer\consumer.exe >build/ui-repair-20261009/sdk-consumer.log 2>&1
exit /b %errorlevel%
