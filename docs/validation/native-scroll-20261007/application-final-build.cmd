@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake --build "build/native-scroll-20261007/application" --parallel 8 > "build/native-scroll-20261007/application-final-build.log" 2>&1
exit /b %errorlevel%
