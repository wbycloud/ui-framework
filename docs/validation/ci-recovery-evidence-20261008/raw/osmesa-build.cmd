@echo off
call "%UI_RECOVERY_VS%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cmake -S . -B "%UI_RECOVERY_DIR%/osmesa" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF "-DUI_OSMESA_LIBRARY=%UI_RECOVERY_PROVIDER%" >"%UI_RECOVERY_DIR%/osmesa-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "%UI_RECOVERY_DIR%/osmesa" >"%UI_RECOVERY_DIR%/osmesa-build.log" 2>&1
exit /b %errorlevel%
