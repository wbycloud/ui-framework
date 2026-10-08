@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cmake -S . -B build/ui-repair-20261009/osmesa -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=ON -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF "-DUI_OSMESA_LIBRARY=D:/应用软件框架/应用层序框架/.deps/mesa-24.3.4/x64/osmesa.dll" >build/ui-repair-20261009/osmesa-configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/ui-repair-20261009/osmesa >build/ui-repair-20261009/osmesa-build.log 2>&1
exit /b %errorlevel%
