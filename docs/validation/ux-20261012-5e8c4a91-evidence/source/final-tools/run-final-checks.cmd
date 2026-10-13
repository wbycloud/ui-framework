@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
ctest --test-dir build/ux-20261012-5e8c4a91/final -j 1 -R "^(ui_light_web_dynamic|ui_web_text_edit|ui_light_glyph_padding|ui_component_layout|ui_component_scroll_native|ui_menu_desktop|ui_menu_access)$" --output-on-failure --output-junit "%CD%\build\ux-20261012-5e8c4a91\raw\final-regression-junit.xml" >build/ux-20261012-5e8c4a91/raw/final-regression.log 2>&1
exit /b %errorlevel%


