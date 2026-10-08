@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake -S . -B build/stability-state-20261008/baseline -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=%CD%/.deps/mesa-24.3.4/x64/osmesa.dll" > build/stability-state-20261008/baseline-configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/stability-state-20261008/baseline --parallel 8 > build/stability-state-20261008/baseline-build.log 2>&1
exit /b %errorlevel%
