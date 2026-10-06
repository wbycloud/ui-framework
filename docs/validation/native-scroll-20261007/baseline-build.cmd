@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake -S . -B "build/native-scroll-20261007/baseline" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=ON -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=ON -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF > "build/native-scroll-20261007/baseline-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "build/native-scroll-20261007/baseline" --parallel 8 > "build/native-scroll-20261007/baseline-build.log" 2>&1
if errorlevel 1 exit /b 1
cl /nologo /utf-8 /std:c11 /Iinclude /I"build/native-scroll-20261007" "build/native-scroll-20261007/scroll_repro.c" "build/native-scroll-20261007/baseline/ui_framework.lib" user32.lib /Fe:"build/native-scroll-20261007/baseline/scroll_repro.exe" /Fo:"build/native-scroll-20261007/baseline/scroll_repro.obj" > "build/native-scroll-20261007/repro-build.log" 2>&1
exit /b %errorlevel%
