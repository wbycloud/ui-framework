@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cmake -S . -B build/language-20261008/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF > build/language-20261008/native-configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/language-20261008/native --parallel 8 > build/language-20261008/native-build.log 2>&1
exit /b %errorlevel%
