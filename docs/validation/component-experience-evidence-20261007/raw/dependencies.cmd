@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
dumpbin /dependents ".deps/mesa-24.3.4/x64/osmesa.dll" > "build/experience-20261007/osmesa-dependents.log"
exit /b %errorlevel%
