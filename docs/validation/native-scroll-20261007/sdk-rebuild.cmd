@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake -S "build/native-scroll-20261007/sdk-unpacked/source" -B "build/native-scroll-20261007/sdk-rebuilt" -G Ninja -DCMAKE_BUILD_TYPE=Release > "build/native-scroll-20261007/sdk-rebuild-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "build/native-scroll-20261007/sdk-rebuilt" --parallel 8 --target ui_framework_shared framework_host uapp_pack ui_component_scroll_native_test > "build/native-scroll-20261007/sdk-rebuild.log" 2>&1
exit /b %errorlevel%
