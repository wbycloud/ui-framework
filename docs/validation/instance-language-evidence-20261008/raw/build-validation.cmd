@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
cmake --build build/language-20261008/after --target ui_component_experience_test > build/language-20261008/validation-build.log 2>&1
exit /b %errorlevel%
