@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cmake --build build/ui-repair-20261009/light >build/ui-repair-20261009/light-final-build.log 2>&1
exit /b %errorlevel%
