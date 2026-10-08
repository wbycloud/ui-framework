@echo off
call "%UI_RECOVERY_VS%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cmake -S . -B "%UI_RECOVERY_DIR%/native" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF >"%UI_RECOVERY_DIR%/native-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "%UI_RECOVERY_DIR%/native" >"%UI_RECOVERY_DIR%/native-build.log" 2>&1
exit /b %errorlevel%
