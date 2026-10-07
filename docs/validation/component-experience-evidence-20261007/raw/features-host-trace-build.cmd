@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
set VSLANG=1033
cl /nologo /MD /utf-8 /std:c11 /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /DUI_FRAMEWORK_BUILD_SHARED /Iinclude /Isrc /Iexamples/framework_host /Ibuild/experience-20261007/after/generated build/experience-20261007/features-host-trace.c build/experience-20261007/after/CMakeFiles/ui_framework_features_host_test.dir/generated/host.rc.res build/experience-20261007/after/ui_framework_runtime.lib user32.lib gdi32.lib comdlg32.lib shell32.lib dwmapi.lib imm32.lib /Fe:build/experience-20261007/after/features-host-trace.exe /Fo:build/experience-20261007/after/features-host-trace.obj > build/experience-20261007/features-host-trace-build.log 2>&1
exit /b %errorlevel%
