@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cl /nologo /MD /std:c11 /utf-8 /DUI_FRAMEWORK_BUILD_SHARED /Ibuild/experience-20261007/sdk/include build/experience-20261007/sdk-consumer.c build/experience-20261007/sdk/lib/ui_framework_runtime.lib /Fe:build/experience-20261007/sdk/bin/sdk-consumer.exe /Fo:build/experience-20261007/sdk-consumer.obj > build/experience-20261007/sdk-consumer-build.log 2>&1
if errorlevel 1 exit /b 1
dumpbin /exports build/experience-20261007/sdk/bin/ui_framework.dll > build/experience-20261007/sdk-exports.log
build\experience-20261007\sdk\bin\sdk-consumer.exe > build/experience-20261007/sdk-consumer.log 2>&1
exit /b %errorlevel%
