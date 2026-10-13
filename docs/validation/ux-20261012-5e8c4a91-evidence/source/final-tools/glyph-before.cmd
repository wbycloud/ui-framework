@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
cl /nologo /W4 /WX /utf-8 /std:c11 /O2 /DUI_FRAMEWORK_BUILD_SHARED /Iinclude /Fobuild/ux-20261012-5e8c4a91/final/glyph-before.obj tests/light_glyph_padding.c build/ux-20261012-5e8c4a91/final/ui_framework_runtime.lib user32.lib gdi32.lib /link /OUT:build/ux-20261012-5e8c4a91/final/glyph-before.exe >build/ux-20261012-5e8c4a91/raw/glyph-before-build.log 2>&1
if errorlevel 1 exit /b 1
build\ux-20261012-5e8c4a91\final\glyph-before.exe >build/ux-20261012-5e8c4a91/raw/glyph-before.log 2>&1
exit /b %errorlevel%
