@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cmake -S . -B build/language-current-20261009/current -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY="D:/应用软件框架/应用层序框架/.deps/mesa-24.3.4/x64/osmesa.dll" >build/language-current-20261009/configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/language-current-20261009/current >build/language-current-20261009/build.log 2>&1
exit /b %errorlevel%