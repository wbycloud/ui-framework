@echo off
call "%UI_RECOVERY_VS%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cl 2>"%UI_RECOVERY_DIR%/compiler.log"
cmake -S . -B "%UI_RECOVERY_DIR%/baseline" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=%UI_RECOVERY_PROVIDER%" >"%UI_RECOVERY_DIR%/baseline-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "%UI_RECOVERY_DIR%/baseline" >"%UI_RECOVERY_DIR%/baseline-build.log" 2>&1
exit /b %errorlevel%
