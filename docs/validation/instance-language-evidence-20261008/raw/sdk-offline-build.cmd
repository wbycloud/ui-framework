@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cmake -S "D:\应用软件框架\应用层序框架\build\language-20261008\sdk-verify-0a151f15c196\sdk\source" -B "D:\应用软件框架\应用层序框架\build\language-20261008\sdk-offline" -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=D:\应用软件框架\应用层序框架\build\language-20261008\sdk-verify-0a151f15c196\sdk\bin\osmesa.dll" > "D:\应用软件框架\应用层序框架\build\language-20261008\sdk-offline-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "D:\应用软件框架\应用层序框架\build\language-20261008\sdk-offline" --target framework_host stateful_components_package ui_instance_language_test ui_language_core_test ui_public_headers_cpp_test ui_offscreen_msaa_test --parallel 8 > "D:\应用软件框架\应用层序框架\build\language-20261008\sdk-offline-build.log" 2>&1
if errorlevel 1 exit /b 1
ctest --test-dir "D:\应用软件框架\应用层序框架\build\language-20261008\sdk-offline" -R "^(ui_language_core|ui_public_headers_cpp|ui_windowless_gl)$" --output-on-failure --output-junit "D:\应用软件框架\应用层序框架\build\language-20261008\sdk-offline-tests.xml" > "D:\应用软件框架\应用层序框架\build\language-20261008\sdk-offline-tests.log" 2>&1
exit /b %errorlevel%
