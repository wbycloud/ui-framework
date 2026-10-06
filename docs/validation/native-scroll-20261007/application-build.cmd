@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake -S "build/native-scroll-20261007/application-source" -B "build/native-scroll-20261007/application" -G Ninja -DCMAKE_BUILD_TYPE=Release "-DKC_FRAMEWORK_SOURCE_DIRECTORY=D:/应用软件框架/应用层序框架" > "build/native-scroll-20261007/application-configure.log" 2>&1
if errorlevel 1 exit /b 1
cmake --build "build/native-scroll-20261007/application" --parallel 8 > "build/native-scroll-20261007/application-build.log" 2>&1
exit /b %errorlevel%
