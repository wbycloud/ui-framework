@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake -S build/experience-20261007/sdk/source -B build/experience-20261007/sdk-offline-rebuild -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=ON -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF -DUI_BUILD_STANDALONE_HOST=OFF > build/experience-20261007/sdk-offline-configure.log 2>&1
if errorlevel 1 exit /b 1
cmake --build build/experience-20261007/sdk-offline-rebuild --target ui_public_headers_c_test ui_component_widths_test ui_component_experience_test --parallel 3 > build/experience-20261007/sdk-offline-build.log 2>&1
exit /b %errorlevel%
