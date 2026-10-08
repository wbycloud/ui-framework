@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cmake -S build/stability-state-20261008/sdk/source -B build/stability-state-20261008/sdk-offline -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=%CD%/build/stability-state-20261008/sdk/bin/osmesa.dll" > build/stability-state-20261008/sdk-offline-configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/stability-state-20261008/sdk-offline --parallel 8 > build/stability-state-20261008/sdk-offline-build.log 2>&1
exit /b %errorlevel%
