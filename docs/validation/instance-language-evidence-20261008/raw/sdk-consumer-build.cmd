@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
echo WindowsSDKVersion=%WindowsSDKVersion% > build/language-20261008/sdk-toolchain.log
echo VCToolsVersion=%VCToolsVersion% >> build/language-20261008/sdk-toolchain.log
cl /nologo /std:c11 /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED /Ibuild/language-20261008/sdk-api9/include build/language-20261008/sdk-api9/examples/sdk-consumer.c build/language-20261008/sdk-api9/lib/ui_framework_runtime.lib /Febuild/language-20261008/sdk-api9/bin/sdk-consumer.exe /Fobuild/language-20261008/sdk-consumer.obj > build/language-20261008/sdk-consumer-build.log 2>&1
exit /b %errorlevel%
