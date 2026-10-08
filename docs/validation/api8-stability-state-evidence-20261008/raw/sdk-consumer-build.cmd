@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cl /nologo /std:c11 /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED /Ibuild\stability-state-20261008\sdk\include build\stability-state-20261008\sdk-consumer.c build\stability-state-20261008\sdk\lib\ui_framework_runtime.lib /Fe:build\stability-state-20261008\sdk\bin\sdk-consumer.exe /Fo:build\stability-state-20261008\sdk-consumer.obj > build\stability-state-20261008\sdk-consumer-build.log 2>&1
exit /b %errorlevel%
