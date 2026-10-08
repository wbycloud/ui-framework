@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cl /nologo /std:c11 /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED /D_CRT_SECURE_NO_WARNINGS /I include /I src /I examples\framework_host /I build\stability-state-20261008\baseline\generated tests\host_repaint.c build\stability-state-20261008\baseline\CMakeFiles\ui_visual_ui_test.dir\generated\host.rc.res build\stability-state-20261008\baseline\ui_framework_runtime.lib comdlg32.lib shell32.lib dwmapi.lib ole32.lib user32.lib gdi32.lib imm32.lib /Fe:build\stability-state-20261008\baseline\host_repaint.exe /Fo:build\stability-state-20261008\host_repaint.obj > build\stability-state-20261008\diagnostic-build.log 2>&1
exit /b %errorlevel%
