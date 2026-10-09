@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cmake -S build/language-current-20261009/sdk-delivery-extracted/sdk/source -B build/language-current-20261009/sdk-offline -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY="D:/应用软件框架/应用层序框架/build/language-current-20261009/sdk-delivery-extracted/sdk/bin/osmesa.dll" >build/language-current-20261009/sdk-offline-configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/language-current-20261009/sdk-offline --target framework_host stateful_components_package ui_instance_language_test ui_language_core_test ui_public_headers_cpp_test ui_offscreen_msaa_test >build/language-current-20261009/sdk-offline-build.log 2>&1
exit /b %errorlevel%