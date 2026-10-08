@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cmake --build build/ui-repair-20261009/baseline --target ui_display_probe ui_interaction_probe >build/ui-repair-20261009/debug-build.log 2>&1
exit /b %errorlevel%
