@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake -S . -B "build/native-scroll-20261007/webview2" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=ON -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=ON -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON > "build/native-scroll-20261007/webview2-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "build/native-scroll-20261007/webview2" --parallel 8 --target ui_component_scroll_test ui_component_experience_test ui_webview2_render_test > "build/native-scroll-20261007/webview2-build.log" 2>&1
if errorlevel 1 exit /b 1
exit /b %errorlevel%
