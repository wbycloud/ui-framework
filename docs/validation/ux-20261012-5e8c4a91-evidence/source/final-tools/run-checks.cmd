@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
ctest --test-dir build/ux-20261012-5e8c4a91/final -j 1 -R "^(ui_language_core|ui_public_headers_c|ui_public_headers_cpp|ui_components_core|ui_component_layout|ui_component_layout_webview2|ui_component_widths|ui_component_widths_webview2|ui_component_experience|ui_component_experience_webview2|ui_component_scroll|ui_component_scroll_webview2|ui_component_scroll_native|ui_light_glyph_padding|ui_light_scroll|ui_light_web_dynamic|ui_web_text_edit|ui_menu_desktop|ui_menu_offscreen|ui_menu_cascade_offscreen|ui_menu_access|ui_floating_web_dpi|ui_stateful_components_light|ui_stateful_components_webview2|ui_instance_language_light|ui_instance_language_webview2|ui_api7_integration|ui_api7_integration_webview2)$" --output-on-failure --output-junit "%CD%\build\ux-20261012-5e8c4a91\raw\regression-junit.xml" >build/ux-20261012-5e8c4a91/raw/regression.log 2>&1
exit /b %errorlevel%
