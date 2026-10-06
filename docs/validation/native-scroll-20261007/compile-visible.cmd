@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cl /nologo /MD /utf-8 /std:c11 /Iinclude /I"build/native-scroll-20261007" "build/native-scroll-20261007/scroll_repro-visible.c" "build/native-scroll-20261007/baseline/ui_framework_runtime.lib" user32.lib /Fe:"build/native-scroll-20261007/baseline/scroll_repro-visible.exe" /Fo:"build/native-scroll-20261007/baseline/scroll_repro-visible.obj" > "build/native-scroll-20261007/repro-visible-build.log" 2>&1
exit /b %errorlevel%
