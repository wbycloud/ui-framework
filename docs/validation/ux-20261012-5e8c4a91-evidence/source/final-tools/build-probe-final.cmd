@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cl /nologo /W4 /WX /utf-8 /std:c11 /O2 /Z7 /D_CRT_SECURE_NO_WARNINGS /DUI_FRAMEWORK_BUILD_SHARED /Fobuild/ux-20261012-5e8c4a91/final/ux_probe.obj /I. /Iinclude /Isrc /Ibuild/ux-20261012-5e8c4a91/final/generated build/ux-20261012-5e8c4a91/tools/ux-probe.c build/ux-20261012-5e8c4a91/final/CMakeFiles/ui_stateful_components_test.dir/generated/host.rc.res build/ux-20261012-5e8c4a91/final/ui_framework_runtime.lib user32.lib gdi32.lib comdlg32.lib shell32.lib dwmapi.lib imm32.lib ole32.lib /link /DEBUG /OUT:build/ux-20261012-5e8c4a91/final/ux_probe.exe /PDB:build/ux-20261012-5e8c4a91/final/ux_probe.pdb >build/ux-20261012-5e8c4a91/raw/probe-final-build0.log 2>&1
exit /b %errorlevel%




