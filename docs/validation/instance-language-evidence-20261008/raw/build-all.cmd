@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cmake -S . -B build/language-20261008/after -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=%CD%/.deps/mesa-24.3.4/x64/osmesa.dll" > build/language-20261008/after-configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/language-20261008/after --parallel 8 > build/language-20261008/after-build.log 2>&1
exit /b %errorlevel%

