@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cl /nologo /std:c11 /utf-8 /MD /WX /DUI_FRAMEWORK_BUILD_SHARED /Iinclude tests/language_core.c build/language-20261008/baseline/ui_framework_runtime.lib /Fe:build/language-20261008/language-red.exe /Fo:build/language-20261008/language-red.obj > build/language-20261008/language-api-red.log 2>&1
exit /b %errorlevel%
