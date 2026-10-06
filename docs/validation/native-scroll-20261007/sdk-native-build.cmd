@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cl /nologo /MD /utf-8 /std:c11 /D_CRT_SECURE_NO_WARNINGS /Iinclude "tests/component_scroll_native.c" "build/native-scroll-20261007/sdk/lib/ui_framework_runtime.lib" user32.lib gdi32.lib /Fe:"build/native-scroll-20261007/sdk-verify/component_scroll_native.exe" /Fo:"build/native-scroll-20261007/sdk-verify/component_scroll_native.obj" > "build/native-scroll-20261007/sdk-native-build.log" 2>&1
exit /b %errorlevel%
