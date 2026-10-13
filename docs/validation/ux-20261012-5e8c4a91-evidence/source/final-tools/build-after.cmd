@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
cmake -S . -B build/ux-20261012-5e8c4a91/after -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY="%CD%\.deps\mesa-24.3.4\x64\osmesa.dll" >build/ux-20261012-5e8c4a91/raw/after-configure0.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/ux-20261012-5e8c4a91/after -j 4 >build/ux-20261012-5e8c4a91/raw/after-build0.log 2>&1
exit /b %errorlevel%
