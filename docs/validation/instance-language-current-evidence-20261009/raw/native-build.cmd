@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cmake -S . -B build/language-current-20261009/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF >build/language-current-20261009/native-configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/language-current-20261009/native >build/language-current-20261009/native-build.log 2>&1
exit /b %errorlevel%