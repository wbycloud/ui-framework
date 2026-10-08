@echo off
call "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/Tools/VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cl /nologo /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED /D_CRT_SECURE_NO_WARNINGS /Iinclude tests/interaction_probe.c /Febuild/ui-repair-20261009/red-dll/probe.exe /Fobuild/ui-repair-20261009/red-dll/probe.obj build/ci-recovery-20261008/current/ui_framework_runtime.lib user32.lib gdi32.lib opengl32.lib >build/ui-repair-20261009/old-probe-build.log 2>&1
exit /b %errorlevel%
