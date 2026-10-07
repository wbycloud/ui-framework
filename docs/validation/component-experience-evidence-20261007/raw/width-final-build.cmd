@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cmake --build build/experience-20261007/after --target ui_component_widths_test --parallel 2 > build/experience-20261007/width-final-build.log 2>&1
exit /b %errorlevel%
