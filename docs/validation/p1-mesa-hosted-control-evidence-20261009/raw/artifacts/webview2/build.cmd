@echo off
call "%UI_CI_VS%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cl 2>"%UI_CI_EVIDENCE%/compiler.log"
cmake -S . -B "%UI_CI_BUILD%" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=%UI_CI_LIGHT% -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=%UI_CI_LIGHT% -DUI_FRAMEWORK_ENABLE_WEBVIEW2=%UI_CI_WEBVIEW2% "-DUI_OSMESA_LIBRARY=%UI_CI_PROVIDER%" >"%UI_CI_EVIDENCE%/configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "%UI_CI_BUILD%" >"%UI_CI_EVIDENCE%/build.log" 2>&1
exit /b %errorlevel%
