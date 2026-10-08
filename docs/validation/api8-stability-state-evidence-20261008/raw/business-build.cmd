@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake -S "build/stability-state-20261008/business-source" -B "build/stability-state-20261008/business-current" -G Ninja -DCMAKE_BUILD_TYPE=Release "-DKC_FRAMEWORK_SOURCE_DIRECTORY=D:/应用软件框架/应用层序框架" "-DUI_LEXBOR_SOURCE_DIR=D:/应用软件框架/应用层序框架/.deps/lexbor" "-DUI_QUICKJS_SOURCE_DIR=D:/应用软件框架/应用层序框架/.deps/quickjs" > "build/stability-state-20261008/business-current-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "build/stability-state-20261008/business-current" --parallel 8 > "build/stability-state-20261008/business-current-build.log" 2>&1
exit /b %errorlevel%

