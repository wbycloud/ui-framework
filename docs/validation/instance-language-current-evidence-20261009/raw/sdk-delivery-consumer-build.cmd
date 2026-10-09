@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cl /nologo /std:c11 /W4 /WX /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED /Ibuild/language-current-20261009/sdk-delivery/include build/language-current-20261009/sdk-delivery/examples/sdk-consumer.c /Febuild/language-current-20261009/sdk-delivery/bin/sdk-consumer.exe /Fobuild/language-current-20261009/sdk-delivery-consumer.obj /link build/language-current-20261009/sdk-delivery/lib/ui_framework_runtime.lib >build/language-current-20261009/sdk-delivery-consumer-build.log 2>&1
if errorlevel 1 exit /b 1
build\language-current-20261009\sdk-delivery\bin\sdk-consumer.exe >build/language-current-20261009/sdk-delivery-consumer-staging.log 2>&1
exit /b %errorlevel%