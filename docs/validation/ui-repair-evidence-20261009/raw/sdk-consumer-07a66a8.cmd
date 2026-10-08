@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cl /nologo /W4 /WX /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED /Ibuild/ui-repair-20261009/sdk-unpacked-07a66a8/include build/ui-repair-20261009/sdk-consumer-07a66a8/consumer.c /Febuild/ui-repair-20261009/sdk-consumer-07a66a8/consumer.exe /Fobuild/ui-repair-20261009/sdk-consumer-07a66a8/consumer.obj /link build/ui-repair-20261009/sdk-unpacked-07a66a8/lib/ui_framework_runtime.lib >build/ui-repair-20261009/sdk-consumer-07a66a8-build.log 2>&1
if errorlevel 1 exit /b 1
copy /y build\ui-repair-20261009\sdk-unpacked-07a66a8\bin\ui_framework.dll build\ui-repair-20261009\sdk-consumer-07a66a8\ui_framework.dll >nul
build\ui-repair-20261009\sdk-consumer-07a66a8\consumer.exe >build/ui-repair-20261009/sdk-consumer-07a66a8.log 2>&1
exit /b %errorlevel%
