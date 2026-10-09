cmake_minimum_required(VERSION 3.20)

project(ui_framework VERSION 0.9.0 LANGUAGES C)

option(UI_BUILD_STANDALONE_HOST "Build the Windows Web application host" ${WIN32})

option(UI_FRAMEWORK_ENABLE_WEBVIEW2
       "Build the optional WebView2 backend (requires .deps SDK)" OFF)
option(UI_FRAMEWORK_ENABLE_LIGHT_WEB
       "Build the Lexbor/QuickJS controlled Web backend" ${UI_BUILD_STANDALONE_HOST})

if(UI_BUILD_STANDALONE_HOST AND NOT UI_FRAMEWORK_ENABLE_LIGHT_WEB)
    message(FATAL_ERROR "The standalone Web host requires UI_FRAMEWORK_ENABLE_LIGHT_WEB. Disable UI_BUILD_STANDALONE_HOST for native embedding/core-only builds.")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/utf-8>)
endif()

option(UI_BUILD_TESTS "Build framework integration tests" ON)
set(UI_OSMESA_LIBRARY "" CACHE FILEPATH "Optional explicit OSMesa x64 DLL for true windowless GL acceptance")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    src/visual.css examples/framework_host/host.html examples/framework_host/host.rc)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/visual.css" _visual_style)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(_visual_literal "${_visual_style}")
string(REPLACE "\\" "\\\\" _visual_literal "${_visual_literal}")
string(REPLACE "\"" "\\\"" _visual_literal "${_visual_literal}")
string(REPLACE "\n" "\\n\"\n\"" _visual_literal "${_visual_literal}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/visual_style.h" "static const char ui_visual_style[] = \"${_visual_literal}\";\n")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/examples/framework_host/host.html" _host_page)
string(REPLACE "<style>" "<style>\n${_visual_style}" _host_page "${_host_page}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/framework_host_page.html" "${_host_page}")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/examples/framework_host/host.rc" _host_resource)
string(REPLACE "host.html" "framework_host_page.html" _host_resource "${_host_resource}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc" "${_host_resource}")
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc" PROPERTIES
    OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generated/framework_host_page.html")
configure_file(examples/web_common/assistant.html web_parity.html COPYONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS src/components.html)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/components.html" _component_page)
string(REPLACE "<style>" "<style>\n${_visual_style}" _component_page "${_component_page}")
string(REPLACE "\\" "\\\\" _component_page "${_component_page}")
string(REPLACE "\"" "\\\"" _component_page "${_component_page}")
string(REPLACE "\n" "\\n\"\n\"" _component_page "${_component_page}")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/component_page.h"
    "static const char ui_component_page[] = \"${_component_page}\";\n")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS src/menu.html)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/menu.html" _menu_page)
string(REPLACE "<style>" "<style>\n${_visual_style}" _menu_page "${_menu_page}")
string(REPLACE "\\" "\\\\" _menu_page "${_menu_page}")
string(REPLACE "\"" "\\\"" _menu_page "${_menu_page}")
string(REPLACE "\n" "\\n\"\n\"" _menu_page "${_menu_page}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/menu_page.h" "static const char ui_menu_page[] = \"${_menu_page}\";\n")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS src/backends/webview2_bridge.js)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/backends/webview2_bridge.js" _wv_bridge)
string(REPLACE "\\" "\\\\" _wv_bridge "${_wv_bridge}")
string(REPLACE "\"" "\\\"" _wv_bridge "${_wv_bridge}")
string(REPLACE "\n" "\\n\"\nL\"" _wv_bridge "${_wv_bridge}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/webview2_bridge.h" "static const wchar_t bridge_script[] = L\"${_wv_bridge}\";\n")

add_library(ui_framework STATIC
    src/ui.c
    src/language.c
    src/assistant.c
    src/package.c
    src/images.c
    src/components.c
    src/menus.c
)

target_include_directories(ui_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)

if(WIN32)
    target_sources(ui_framework PRIVATE
        src/platform/win32_surface.c
        src/platform/win32_shell.c)
    target_link_libraries(ui_framework PUBLIC user32 gdi32 opengl32 comctl32
        imm32 usp10 msimg32 ole32 windowscodecs uuid)
else()
    target_sources(ui_framework PRIVATE
        src/platform/headless_surface.c
        src/platform/headless_shell.c)
endif()

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_library(ui_framework_shared SHARED src/ui.c src/language.c src/assistant.c src/package.c
        src/images.c src/components.c src/menus.c
        src/workspace.c src/platform/win32_surface.c src/platform/win32_shell.c)
    set_target_properties(ui_framework_shared PROPERTIES
        OUTPUT_NAME ui_framework ARCHIVE_OUTPUT_NAME ui_framework_runtime)
    target_include_directories(ui_framework_shared PUBLIC include PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/generated)
    target_compile_definitions(ui_framework_shared PUBLIC UI_FRAMEWORK_BUILD_SHARED
        PRIVATE UI_FRAMEWORK_BUILDING _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(ui_framework_shared PUBLIC user32 gdi32 opengl32 comctl32
        imm32 usp10 msimg32 ole32 windowscodecs uuid)
    if(MSVC)
        target_compile_options(ui_framework_shared PRIVATE /W4 /WX /utf-8)
    endif()
    if(UI_BUILD_STANDALONE_HOST)
        enable_language(RC)
        add_executable(framework_host WIN32 examples/framework_host/main.c
            "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
        target_include_directories(framework_host PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
        target_link_libraries(framework_host PRIVATE ui_framework_shared comdlg32 shell32 dwmapi)
        add_executable(dialog_layout_example WIN32 examples/dialog_layout/main.c)
        target_link_libraries(dialog_layout_example PRIVATE ui_framework_shared)
        target_compile_definitions(framework_host PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
    add_library(minimal_eda_app MODULE examples/minimal_eda/app.c)
    target_link_libraries(minimal_eda_app PRIVATE ui_framework_shared)
    # Current functional fixtures use the current public SDK.
    target_compile_definitions(minimal_eda_app PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(MSVC)
        if(TARGET framework_host)
            target_compile_options(framework_host PRIVATE $<$<COMPILE_LANGUAGE:C>:/W4;/WX;/utf-8>)
            set_property(TARGET framework_host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
        target_compile_options(minimal_eda_app PRIVATE /W4 /WX /utf-8)
        set_target_properties(ui_framework_shared minimal_eda_app
            PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        if(UI_FRAMEWORK_ENABLE_LIGHT_WEB OR UI_FRAMEWORK_ENABLE_WEBVIEW2)
            # Bundled static engines/loader use the dynamic CRT. Match them;
            # native-only distributed binaries remain self-contained /MT.
            set_target_properties(ui_framework_shared PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endif()
    add_executable(uapp_pack tools/uapp_pack.c)
    target_link_libraries(uapp_pack PRIVATE ui_framework)
    if(MSVC)
        target_compile_options(uapp_pack PRIVATE /W4 /WX /utf-8)
    endif()
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/eda_package"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:minimal_eda_app>"
            "${CMAKE_CURRENT_BINARY_DIR}/eda_package/minimal_eda_app.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/examples/minimal_eda/resources"
            "${CMAKE_CURRENT_BINARY_DIR}/eda_package"
        COMMAND $<TARGET_FILE:uapp_pack>
            "${CMAKE_CURRENT_SOURCE_DIR}/examples/minimal_eda/manifest.ini"
            "${CMAKE_CURRENT_BINARY_DIR}/eda_package"
            "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp"
        DEPENDS minimal_eda_app uapp_pack examples/minimal_eda/manifest.ini
            examples/minimal_eda/resources/readme.txt VERBATIM)
    add_custom_target(minimal_eda_package ALL DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp")
endif()

if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/webview2.cmake)
    ui_framework_enable_webview2(ui_framework)
    if(TARGET ui_framework_shared)
        ui_framework_enable_webview2(ui_framework_shared)
    endif()
    add_executable(webview2_probe
        examples/webview2_probe/main.c
    )
    target_link_libraries(webview2_probe PRIVATE ui_framework)
    target_compile_definitions(webview2_probe PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(webview2_probe PRIVATE
        UI_WEB_PARITY_PAGE_PATH="web_parity.html")
endif()

if(UI_FRAMEWORK_ENABLE_LIGHT_WEB)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/light_web.cmake)
    ui_enable_light_web(ui_framework)
    if(TARGET ui_framework_shared)
        add_library(generic_components_app MODULE examples/generic_components/app.c)
        target_link_libraries(generic_components_app PRIVATE ui_framework_shared)
        target_compile_definitions(generic_components_app PRIVATE _CRT_SECURE_NO_WARNINGS)
        if(MSVC)
            target_compile_options(generic_components_app PRIVATE /W4 /WX /utf-8)
        endif()
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generic_components.uapp"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generic_package"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:generic_components_app>" "${CMAKE_CURRENT_BINARY_DIR}/generic_package/generic_components_app.dll"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/examples/generic_components/icon.png" "${CMAKE_CURRENT_BINARY_DIR}/generic_package/icon.png"
            COMMAND $<TARGET_FILE:uapp_pack> "${CMAKE_CURRENT_SOURCE_DIR}/examples/generic_components/manifest.ini" "${CMAKE_CURRENT_BINARY_DIR}/generic_package" "${CMAKE_CURRENT_BINARY_DIR}/generic_components.uapp"
            DEPENDS generic_components_app uapp_pack examples/generic_components/manifest.ini examples/generic_components/icon.png VERBATIM)
        add_custom_target(generic_components_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generic_components.uapp")
        add_library(framework_features_app MODULE examples/generic_components/app.c)
        target_link_libraries(framework_features_app PRIVATE ui_framework_shared)
        target_compile_definitions(framework_features_app PRIVATE UI_SAMPLE_API4 _CRT_SECURE_NO_WARNINGS)
        if(MSVC)
            target_compile_options(framework_features_app PRIVATE /W4 /WX /utf-8)
        endif()
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/framework_features.uapp"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/features_package"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:framework_features_app>" "${CMAKE_CURRENT_BINARY_DIR}/features_package/framework_features_app.dll"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/examples/generic_components/icon.png" "${CMAKE_CURRENT_BINARY_DIR}/features_package/icon.png"
            COMMAND $<TARGET_FILE:uapp_pack> "${CMAKE_CURRENT_SOURCE_DIR}/examples/framework_features/manifest.ini" "${CMAKE_CURRENT_BINARY_DIR}/features_package" "${CMAKE_CURRENT_BINARY_DIR}/framework_features.uapp"
            DEPENDS framework_features_app uapp_pack examples/framework_features/manifest.ini examples/generic_components/icon.png VERBATIM)
        add_custom_target(framework_features_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/framework_features.uapp")
        ui_enable_light_web(ui_framework_shared)
    endif()
    add_executable(light_web_probe examples/light_web_probe/probe.c)
    target_link_libraries(light_web_probe PRIVATE ui_framework)
    target_compile_definitions(light_web_probe PRIVATE
        UI_WEB_PARITY_PAGE_PATH="web_parity.html")
    if(TARGET ui_framework_shared)
        add_library(web_counter_app MODULE examples/web_counter/app.c)
        target_link_libraries(web_counter_app PRIVATE ui_framework_shared)
        if(MSVC)
            target_compile_options(web_counter_app PRIVATE /W4 /WX /utf-8)
            set_property(TARGET web_counter_app PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/web_counter.uapp"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/counter_package"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:web_counter_app>"
                "${CMAKE_CURRENT_BINARY_DIR}/counter_package/web_counter_app.dll"
            COMMAND $<TARGET_FILE:uapp_pack>
                "${CMAKE_CURRENT_SOURCE_DIR}/examples/web_counter/manifest.ini"
                "${CMAKE_CURRENT_BINARY_DIR}/counter_package"
                "${CMAKE_CURRENT_BINARY_DIR}/web_counter.uapp"
            DEPENDS web_counter_app uapp_pack examples/web_counter/manifest.ini VERBATIM)
        add_custom_target(web_counter_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/web_counter.uapp")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/examples/web_counter/manifest.ini" _counter_manifest)
        string(REPLACE "framework_api_version=9" "framework_api_version=6"
            _counter_mismatch "${_counter_manifest}")
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/counter_mismatch.ini" "${_counter_mismatch}")
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/counter_mismatch.uapp"
            COMMAND $<TARGET_FILE:uapp_pack> "${CMAKE_CURRENT_BINARY_DIR}/counter_mismatch.ini"
                "${CMAKE_CURRENT_BINARY_DIR}/counter_package"
                "${CMAKE_CURRENT_BINARY_DIR}/counter_mismatch.uapp"
            DEPENDS web_counter_package uapp_pack "${CMAKE_CURRENT_BINARY_DIR}/counter_mismatch.ini" VERBATIM)
        add_custom_target(counter_mismatch_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/counter_mismatch.uapp")
    endif()
endif()

if(WIN32)
    add_executable(minimal_eda WIN32
        examples/minimal_eda/main.c
    )
else()
    add_executable(minimal_eda
        examples/minimal_eda/headless.c
    )
endif()

target_link_libraries(minimal_eda PRIVATE ui_framework)

if(WIN32)
    target_compile_definitions(minimal_eda PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(UI_BUILD_TESTS)
    enable_testing()
    add_executable(ui_language_core_test tests/language_core.c)
    target_link_libraries(ui_language_core_test PRIVATE ui_framework)
    if(MSVC)
        target_compile_options(ui_language_core_test PRIVATE /W4 /WX /utf-8)
    endif()
    add_test(NAME ui_language_core COMMAND ui_language_core_test)
    add_executable(ui_package_format_test tests/package_format.c)
    target_link_libraries(ui_package_format_test PRIVATE ui_framework)
    add_test(NAME ui_package_format COMMAND ui_package_format_test)
    add_library(ui_framework_headless STATIC src/ui.c src/language.c src/assistant.c src/images.c src/components.c src/menus.c
        src/platform/headless_surface.c src/platform/headless_shell.c)
    target_include_directories(ui_framework_headless PUBLIC include PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/generated)
    if(WIN32)
        target_link_libraries(ui_framework_headless PUBLIC user32 gdi32 imm32 usp10 msimg32 ole32 windowscodecs uuid)
        add_executable(ui_images_windows_test tests/images_windows.c)
        target_link_libraries(ui_images_windows_test PRIVATE ui_framework_headless)
        add_test(NAME ui_images_windows COMMAND ui_images_windows_test)
    endif()
    foreach(test_name IN ITEMS responsive_layout assistant_lifecycle dpi_web_failure components_core)
        add_executable(ui_${test_name}_test tests/${test_name}.c)
        target_link_libraries(ui_${test_name}_test PRIVATE ui_framework_headless)
        add_test(NAME ui_${test_name} COMMAND ui_${test_name}_test)
    endforeach()
    add_executable(ui_headless_example examples/minimal_eda/headless.c)
    target_link_libraries(ui_headless_example PRIVATE ui_framework_headless)
    add_test(NAME ui_headless_example COMMAND ui_headless_example)
    add_executable(ui_public_headers_c_test tests/public_headers.c)
    target_link_libraries(ui_public_headers_c_test PRIVATE ui_framework_headless)
    add_test(NAME ui_public_headers_c COMMAND ui_public_headers_c_test)
    include(CheckLanguage)
    check_language(CXX)
    if(CMAKE_CXX_COMPILER)
        enable_language(CXX)
        add_executable(ui_public_headers_cpp_test tests/public_headers.cpp)
        target_compile_features(ui_public_headers_cpp_test PRIVATE cxx_std_11)
        target_link_libraries(ui_public_headers_cpp_test PRIVATE ui_framework_headless)
        add_test(NAME ui_public_headers_cpp COMMAND ui_public_headers_cpp_test)
    endif()
    if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
        add_test(NAME ui_webview2_parity COMMAND webview2_probe)
        set_tests_properties(ui_webview2_parity PROPERTIES TIMEOUT 25 SKIP_RETURN_CODE 77)
        add_executable(ui_webview2_messages_test tests/webview2_messages.c)
        target_link_libraries(ui_webview2_messages_test PRIVATE ui_framework)
        if(MSVC)
            target_compile_options(ui_webview2_messages_test PRIVATE /W4 /WX /utf-8)
        endif()
        add_test(NAME ui_webview2_messages COMMAND ui_webview2_messages_test)
        add_executable(ui_webview2_render_test tests/webview2_render.c)
        target_link_libraries(ui_webview2_render_test PRIVATE ui_framework_shared)
        add_test(NAME ui_webview2_render COMMAND ui_webview2_render_test)
        set_tests_properties(ui_webview2_render PROPERTIES TIMEOUT 30)
        set_tests_properties(ui_webview2_messages PROPERTIES TIMEOUT 25 SKIP_RETURN_CODE 77)
    endif()
    if(WIN32)
        add_executable(ui_workspace_layout_test tests/workspace_layout.c)
        target_link_libraries(ui_workspace_layout_test PRIVATE ui_framework)
        add_test(NAME ui_workspace_layout COMMAND ui_workspace_layout_test)
        if(UI_FRAMEWORK_ENABLE_LIGHT_WEB)
            add_executable(ui_component_experience_test tests/component_experience.c)
            add_executable(ui_component_widths_test tests/component_widths.c)
            add_executable(ui_component_layout_test tests/component_layout.c)
            target_link_libraries(ui_component_layout_test PRIVATE ui_framework)
            add_test(NAME ui_component_layout COMMAND ui_component_layout_test)
            set_tests_properties(ui_component_layout PROPERTIES TIMEOUT 60)
            target_link_libraries(ui_component_widths_test PRIVATE ui_framework)
            add_test(NAME ui_component_widths COMMAND ui_component_widths_test)
            target_link_libraries(ui_component_experience_test PRIVATE ui_framework)
            add_test(NAME ui_component_experience COMMAND ui_component_experience_test)
            if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
                target_compile_definitions(ui_component_layout_test PRIVATE UI_EXPERIENCE_WEBVIEW2=1)
                add_test(NAME ui_component_layout_webview2 COMMAND ui_component_layout_test --webview2)
                set_tests_properties(ui_component_layout_webview2 PROPERTIES TIMEOUT 60)
                target_compile_definitions(ui_component_experience_test PRIVATE UI_EXPERIENCE_WEBVIEW2=1)
                add_test(NAME ui_component_experience_webview2 COMMAND ui_component_experience_test --webview2)
                set_tests_properties(ui_component_experience_webview2 PROPERTIES TIMEOUT 60)
            endif()
            add_executable(ui_component_scroll_test tests/component_scroll.c)
            add_executable(ui_component_scroll_native_test tests/component_scroll_native.c)
            target_link_libraries(ui_component_scroll_native_test PRIVATE ui_framework_shared)
            add_test(NAME ui_component_scroll_native COMMAND ui_component_scroll_native_test)
            set_tests_properties(ui_component_scroll_native PROPERTIES TIMEOUT 90 RUN_SERIAL TRUE)
            add_executable(ui_light_glyph_padding_test tests/light_glyph_padding.c)
            target_link_libraries(ui_light_glyph_padding_test PRIVATE ui_framework_shared)
            add_test(NAME ui_light_glyph_padding COMMAND ui_light_glyph_padding_test)
            add_executable(ui_light_scroll_test tests/light_scroll.c)
            target_link_libraries(ui_light_scroll_test PRIVATE ui_framework)
            add_test(NAME ui_light_scroll COMMAND ui_light_scroll_test)
            target_link_libraries(ui_component_scroll_test PRIVATE ui_framework)
            add_test(NAME ui_component_scroll COMMAND ui_component_scroll_test)
            if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
                target_compile_definitions(ui_component_widths_test PRIVATE UI_EXPERIENCE_WEBVIEW2=1)
                add_test(NAME ui_component_widths_webview2 COMMAND ui_component_widths_test --webview2)
                set_tests_properties(ui_component_widths_webview2 PROPERTIES TIMEOUT 90)
                target_compile_definitions(ui_component_scroll_test PRIVATE UI_SCROLL_WEBVIEW2=1)
                add_test(NAME ui_component_scroll_webview2 COMMAND ui_component_scroll_test --webview2)
                set_tests_properties(ui_component_scroll_webview2 PROPERTIES TIMEOUT 120)
            endif()
            add_library(ui_api7_fixture MODULE tests/api7_fixture.c)
            target_link_libraries(ui_api7_fixture PRIVATE ui_framework_shared)
            add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture.uapp"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture_package"
                COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:ui_api7_fixture>" "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture_package/ui_api7_fixture.dll"
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/tests/api7-resources/marker.txt" "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture_package/marker.txt"
                COMMAND $<TARGET_FILE:uapp_pack> "${CMAKE_CURRENT_SOURCE_DIR}/tests/api7_fixture.ini" "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture_package" "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture.uapp"
                DEPENDS ui_api7_fixture uapp_pack tests/api7_fixture.ini tests/api7-resources/marker.txt VERBATIM)
            add_custom_target(ui_api7_fixture_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture.uapp")
            add_executable(ui_api7_integration_test tests/api7_integration.c)
            target_link_libraries(ui_api7_integration_test PRIVATE ui_framework_shared ole32)
            add_dependencies(ui_api7_integration_test ui_api7_fixture_package)
            if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
                target_compile_definitions(ui_api7_fixture PRIVATE UI_API7_WEBVIEW2=1)
            endif()
            if(UI_OSMESA_LIBRARY)
                add_library(stateful_components_app MODULE examples/stateful_components/app.c)
                target_link_libraries(stateful_components_app PRIVATE ui_framework_shared)
                target_compile_definitions(stateful_components_app PRIVATE _CRT_SECURE_NO_WARNINGS)
                if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
                    target_compile_definitions(stateful_components_app PRIVATE UI_API7_WEBVIEW2=1)
                endif()
                add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/stateful_components.uapp"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/stateful_components_package"
                    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:stateful_components_app>" "${CMAKE_CURRENT_BINARY_DIR}/stateful_components_package/stateful_components_app.dll"
                    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/tests/api7-resources/marker.txt" "${CMAKE_CURRENT_BINARY_DIR}/stateful_components_package/marker.txt"
                    COMMAND $<TARGET_FILE:uapp_pack> "${CMAKE_CURRENT_SOURCE_DIR}/examples/stateful_components/app.ini" "${CMAKE_CURRENT_BINARY_DIR}/stateful_components_package" "${CMAKE_CURRENT_BINARY_DIR}/stateful_components.uapp"
                    DEPENDS stateful_components_app uapp_pack examples/stateful_components/app.ini tests/api7-resources/marker.txt VERBATIM)
                add_custom_target(stateful_components_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/stateful_components.uapp")
                foreach(state_test IN ITEMS host_repaint stateful_components instance_language)
                    add_executable(ui_${state_test}_test tests/${state_test}.c "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
                    target_include_directories(ui_${state_test}_test PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
                    target_link_libraries(ui_${state_test}_test PRIVATE ui_framework_shared comdlg32 shell32 dwmapi ole32)
                    target_compile_definitions(ui_${state_test}_test PRIVATE _CRT_SECURE_NO_WARNINGS)
                    add_dependencies(ui_${state_test}_test stateful_components_package)
                    if(MSVC)
                        target_compile_options(ui_${state_test}_test PRIVATE /W4 /WX /utf-8)
                    endif()
                endforeach()
                add_dependencies(ui_host_repaint_test ui_api7_fixture_package)
                if(MSVC)
                    target_compile_options(stateful_components_app PRIVATE /W4 /WX /utf-8)
                endif()
                add_test(NAME ui_host_repaint COMMAND ui_host_repaint_test "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture.uapp" "${UI_OSMESA_LIBRARY}")
                set_tests_properties(ui_host_repaint PROPERTIES TIMEOUT 45)
                find_program(UI_STATE_POWERSHELL NAMES pwsh powershell REQUIRED)
                add_test(NAME ui_stateful_components_light COMMAND "${UI_STATE_POWERSHELL}" -NoProfile -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/run-stateful-components.ps1" -BuildDirectory "${CMAKE_CURRENT_BINARY_DIR}" -Provider "${UI_OSMESA_LIBRARY}" -Backend light)
                set_tests_properties(ui_stateful_components_light PROPERTIES TIMEOUT 180 RUN_SERIAL TRUE)
                add_test(NAME ui_instance_language_light COMMAND "${UI_STATE_POWERSHELL}" -NoProfile -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/run-instance-language.ps1" -BuildDirectory "${CMAKE_CURRENT_BINARY_DIR}" -Provider "${UI_OSMESA_LIBRARY}" -Backend light)
                set_tests_properties(ui_instance_language_light PROPERTIES TIMEOUT 240 RUN_SERIAL TRUE)
                if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
                    add_test(NAME ui_stateful_components_webview2 COMMAND "${UI_STATE_POWERSHELL}" -NoProfile -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/run-stateful-components.ps1" -BuildDirectory "${CMAKE_CURRENT_BINARY_DIR}" -Provider "${UI_OSMESA_LIBRARY}" -Backend webview2)
                    set_tests_properties(ui_stateful_components_webview2 PROPERTIES TIMEOUT 240 RUN_SERIAL TRUE)
                    add_test(NAME ui_instance_language_webview2 COMMAND "${UI_STATE_POWERSHELL}" -NoProfile -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/run-instance-language.ps1" -BuildDirectory "${CMAKE_CURRENT_BINARY_DIR}" -Provider "${UI_OSMESA_LIBRARY}" -Backend webview2)
                    set_tests_properties(ui_instance_language_webview2 PROPERTIES TIMEOUT 300 RUN_SERIAL TRUE)
                endif()
                add_executable(ui_visual_ui_test tests/visual_ui.c "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
                target_include_directories(ui_visual_ui_test PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
                target_link_libraries(ui_visual_ui_test PRIVATE ui_framework_shared comdlg32 shell32 dwmapi ole32)
                target_compile_definitions(ui_visual_ui_test PRIVATE _CRT_SECURE_NO_WARNINGS)
                add_dependencies(ui_visual_ui_test ui_api7_fixture_package)
                if(MSVC)
                    target_compile_options(ui_visual_ui_test PRIVATE /W4 /WX /utf-8)
                endif()
                add_test(NAME ui_visual_ui_light COMMAND ui_visual_ui_test "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture.uapp" "${UI_OSMESA_LIBRARY}" light)
                set_tests_properties(ui_visual_ui_light PROPERTIES TIMEOUT 90)
                if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
                    add_test(NAME ui_visual_ui_webview2 COMMAND ui_visual_ui_test "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture.uapp" "${UI_OSMESA_LIBRARY}" webview2)
                    set_tests_properties(ui_visual_ui_webview2 PROPERTIES TIMEOUT 120)
                endif()
                add_test(NAME ui_api7_integration COMMAND ui_api7_integration_test "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture.uapp" "${UI_OSMESA_LIBRARY}" light)
                set_tests_properties(ui_api7_integration PROPERTIES TIMEOUT 90)
                if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
                    add_test(NAME ui_api7_integration_webview2 COMMAND ui_api7_integration_test "${CMAKE_CURRENT_BINARY_DIR}/api7_fixture.uapp" "${UI_OSMESA_LIBRARY}" webview2)
                    set_tests_properties(ui_api7_integration_webview2 PROPERTIES TIMEOUT 120)
                endif()
            endif()
            if(MSVC)
                foreach(test IN ITEMS ui_api7_fixture ui_api7_integration_test ui_component_widths_test ui_component_experience_test ui_component_scroll_test ui_light_scroll_test)
                    target_compile_options(${test} PRIVATE /W4 /WX /utf-8)
                    target_compile_definitions(${test} PRIVATE _CRT_SECURE_NO_WARNINGS)
                endforeach()
            endif()
        endif()
        add_library(ui_session0_gl_app MODULE tests/session0_gl_app.c)
        target_link_libraries(ui_session0_gl_app PRIVATE ui_framework_shared)
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/session0_gl_app.uapp"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/session0_gl_package"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:ui_session0_gl_app>" "${CMAKE_CURRENT_BINARY_DIR}/session0_gl_package/ui_session0_gl_app.dll"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/tests/session0-resources/marker.txt" "${CMAKE_CURRENT_BINARY_DIR}/session0_gl_package/marker.txt"
            COMMAND $<TARGET_FILE:uapp_pack> "${CMAKE_CURRENT_SOURCE_DIR}/tests/session0_gl_app.ini" "${CMAKE_CURRENT_BINARY_DIR}/session0_gl_package" "${CMAKE_CURRENT_BINARY_DIR}/session0_gl_app.uapp"
            DEPENDS ui_session0_gl_app uapp_pack tests/session0_gl_app.ini tests/session0-resources/marker.txt VERBATIM)
        add_custom_target(ui_session0_gl_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/session0_gl_app.uapp")
        add_executable(ui_session0_gl_test tests/session0_gl.c)
        target_link_libraries(ui_session0_gl_test PRIVATE ui_framework_shared secur32 wtsapi32 psapi advapi32)
        add_dependencies(ui_session0_gl_test ui_session0_gl_package)
        if(UI_OSMESA_LIBRARY)
            file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/session0-control")
            add_test(NAME ui_session0_interactive_control COMMAND ui_session0_gl_test
                "${CMAKE_CURRENT_BINARY_DIR}/session0_gl_app.uapp" "${UI_OSMESA_LIBRARY}"
                "${CMAKE_CURRENT_BINARY_DIR}/session0-control" --allow-interactive)
            set_tests_properties(ui_session0_interactive_control PROPERTIES TIMEOUT 120)
        endif()
        add_executable(ui_offscreen_msaa_test tests/offscreen_msaa.c)
        target_link_libraries(ui_offscreen_msaa_test PRIVATE ui_framework_shared)
        add_test(NAME ui_offscreen_msaa COMMAND ui_offscreen_msaa_test)
        if(UI_OSMESA_LIBRARY)
            add_test(NAME ui_windowless_gl COMMAND ui_offscreen_msaa_test "${UI_OSMESA_LIBRARY}")
        endif()
    endif()
    if(UI_FRAMEWORK_ENABLE_LIGHT_WEB)
        add_executable(ui_offscreen_gl_test tests/offscreen_gl.c)
        target_link_libraries(ui_offscreen_gl_test PRIVATE ui_framework)
        add_test(NAME ui_offscreen_gl COMMAND ui_offscreen_gl_test)
        set_tests_properties(ui_offscreen_gl PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 20)
        add_executable(ui_workspace_offscreen_test tests/workspace_offscreen.c)
        target_link_libraries(ui_workspace_offscreen_test PRIVATE ui_framework_shared)
        add_dependencies(ui_workspace_offscreen_test framework_features_package)
        add_test(NAME ui_workspace_offscreen COMMAND ui_workspace_offscreen_test "${CMAKE_CURRENT_BINARY_DIR}/framework_features.uapp")
        set_tests_properties(ui_workspace_offscreen PROPERTIES TIMEOUT 20)
        if(MSVC)
            foreach(test IN ITEMS ui_offscreen_gl_test ui_workspace_offscreen_test)
                target_compile_options(${test} PRIVATE /W4 /WX /utf-8)
            endforeach()
        endif()
        add_executable(ui_framework_features_host_test tests/framework_features_host.c "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
        target_include_directories(ui_framework_features_host_test PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
        target_compile_definitions(ui_framework_features_host_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_link_libraries(ui_framework_features_host_test PRIVATE ui_framework_shared comdlg32 shell32 dwmapi)
        add_dependencies(ui_framework_features_host_test framework_features_package)
        add_test(NAME ui_framework_features_host COMMAND ui_framework_features_host_test "${CMAKE_CURRENT_BINARY_DIR}/framework_features.uapp" "${CMAKE_CURRENT_BINARY_DIR}/features-host.bmp")
        set_tests_properties(ui_framework_features_host PROPERTIES TIMEOUT 30)
        add_executable(ui_menu_desktop_test tests/menu_desktop.c "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
        target_include_directories(ui_menu_desktop_test PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
        target_compile_definitions(ui_menu_desktop_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_link_libraries(ui_menu_desktop_test PRIVATE ui_framework_shared comdlg32 shell32 dwmapi)
        add_dependencies(ui_menu_desktop_test framework_features_package)
        add_test(NAME ui_menu_desktop COMMAND ui_menu_desktop_test "${CMAKE_CURRENT_BINARY_DIR}/framework_features.uapp")
        set_tests_properties(ui_menu_desktop PROPERTIES TIMEOUT 30)
        if(MSVC)
            target_compile_options(ui_framework_features_host_test PRIVATE /W4 /WX /utf-8)
            target_compile_options(ui_menu_desktop_test PRIVATE /W4 /WX /utf-8)
        endif()
        add_executable(ui_menu_offscreen_test tests/menu_offscreen.c)
        target_link_libraries(ui_menu_offscreen_test PRIVATE ui_framework)
        add_test(NAME ui_menu_offscreen COMMAND ui_menu_offscreen_test)
        add_executable(ui_menu_cascade_offscreen_test tests/menu_cascade_offscreen.c)
        target_link_libraries(ui_menu_cascade_offscreen_test PRIVATE ui_framework)
        add_test(NAME ui_menu_cascade_offscreen COMMAND ui_menu_cascade_offscreen_test)
        if(MSVC)
            target_compile_options(ui_menu_cascade_offscreen_test PRIVATE /W4 /WX /utf-8)
        endif()
        add_executable(ui_menu_access_test tests/menu_access.c)
        target_link_libraries(ui_menu_access_test PRIVATE ui_framework)
        add_test(NAME ui_menu_access COMMAND ui_menu_access_test)
        add_executable(ui_web_text_edit_test tests/web_text_edit.c)
        target_link_libraries(ui_web_text_edit_test PRIVATE ui_framework)
        add_test(NAME ui_web_text_edit COMMAND ui_web_text_edit_test)
        add_executable(ui_generic_web_host_test tests/generic_web_host.c "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
        target_include_directories(ui_generic_web_host_test PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
        target_compile_definitions(ui_generic_web_host_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_link_libraries(ui_generic_web_host_test PRIVATE ui_framework_shared comdlg32 shell32 dwmapi)
        add_dependencies(ui_generic_web_host_test generic_components_package)
        add_test(NAME ui_generic_web_host COMMAND ui_generic_web_host_test
            "${CMAKE_CURRENT_BINARY_DIR}/generic_components.uapp"
            "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp" "${CMAKE_CURRENT_BINARY_DIR}/web_counter.uapp")
        set_tests_properties(ui_generic_web_host PROPERTIES TIMEOUT 45)
        add_executable(ui_component_resilience_test tests/component_resilience.c "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
        target_include_directories(ui_component_resilience_test PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
        target_compile_definitions(ui_component_resilience_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_link_libraries(ui_component_resilience_test PRIVATE ui_framework_shared comdlg32 shell32 dwmapi psapi)
        add_dependencies(ui_component_resilience_test framework_features_package)
        add_test(NAME ui_component_resilience COMMAND ui_component_resilience_test "${CMAKE_CURRENT_BINARY_DIR}/framework_features.uapp")
        set_tests_properties(ui_component_resilience PROPERTIES TIMEOUT 30)
        if(MSVC)
            target_compile_options(ui_component_resilience_test PRIVATE /W4 /WX /utf-8)
        endif()
        add_executable(ui_application_contract_test tests/application_versions.c)
        target_link_libraries(ui_application_contract_test PRIVATE ui_framework_shared)
        target_compile_definitions(ui_application_contract_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        add_dependencies(ui_application_contract_test minimal_eda_package web_counter_package counter_mismatch_package)
        add_test(NAME ui_application_contract COMMAND ui_application_contract_test
            "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp" "${CMAKE_CURRENT_BINARY_DIR}/web_counter.uapp"
            "${CMAKE_CURRENT_BINARY_DIR}/counter_mismatch.uapp")
        set_tests_properties(ui_application_contract PROPERTIES TIMEOUT 45)
        add_executable(ui_light_web_dynamic_test tests/light_web_dynamic.c)
        # Desktop SendInput/continuous-frame acceptance is an explicit serial run.
        add_executable(ui_interaction_probe tests/interaction_probe.c)
        add_executable(ui_display_probe tests/display_probe.c)
        target_link_libraries(ui_display_probe PRIVATE ui_framework)
        target_compile_definitions(ui_display_probe PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_link_libraries(ui_interaction_probe PRIVATE ui_framework)
        target_compile_definitions(ui_interaction_probe PRIVATE _CRT_SECURE_NO_WARNINGS)
        if(UI_FRAMEWORK_ENABLE_WEBVIEW2)
            target_compile_definitions(ui_interaction_probe PRIVATE UI_REPAIR_WEBVIEW2=1)
            add_executable(ui_readonly_navigation_probe tests/readonly_navigation_probe.c)
            target_link_libraries(ui_readonly_navigation_probe PRIVATE ui_framework_shared)
            target_compile_definitions(ui_readonly_navigation_probe PRIVATE _CRT_SECURE_NO_WARNINGS)
            add_executable(ui_presentation_transition_probe tests/presentation_transition_probe.c)
            target_link_libraries(ui_presentation_transition_probe PRIVATE ui_framework_shared)
            target_compile_definitions(ui_presentation_transition_probe PRIVATE _CRT_SECURE_NO_WARNINGS)
            add_executable(ui_runtime_boundary_probe tests/runtime_boundary_probe.c)
            target_link_libraries(ui_runtime_boundary_probe PRIVATE ui_framework_shared)
            target_compile_definitions(ui_runtime_boundary_probe PRIVATE _CRT_SECURE_NO_WARNINGS)
        endif()
        target_link_libraries(ui_light_web_dynamic_test PRIVATE ui_framework)
        add_test(NAME ui_light_web_dynamic COMMAND ui_light_web_dynamic_test)
        set_tests_properties(ui_light_web_dynamic PROPERTIES TIMEOUT 30)
        add_executable(ui_web_shell_slots_test tests/web_shell_slots.c)
        target_link_libraries(ui_web_shell_slots_test PRIVATE ui_framework)
        target_compile_definitions(ui_web_shell_slots_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        add_test(NAME ui_web_shell_slots COMMAND ui_web_shell_slots_test)
        set_tests_properties(ui_web_shell_slots PROPERTIES TIMEOUT 30)
        add_executable(ui_floating_web_dpi_test tests/floating_web_dpi.c)
        target_link_libraries(ui_floating_web_dpi_test PRIVATE ui_framework)
        add_test(NAME ui_floating_web_dpi COMMAND ui_floating_web_dpi_test)
        set_tests_properties(ui_floating_web_dpi PROPERTIES TIMEOUT 30)
        if(MSVC)
            target_compile_options(ui_application_contract_test PRIVATE /W4 /WX /utf-8)
            set_property(TARGET ui_application_contract_test PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
            target_compile_options(ui_light_web_dynamic_test PRIVATE /W4 /WX /utf-8)
            target_compile_options(ui_web_shell_slots_test PRIVATE /W4 /WX /utf-8)
            target_compile_options(ui_floating_web_dpi_test PRIVATE /W4 /WX /utf-8)
        endif()
        target_compile_definitions(light_web_probe PRIVATE
            UI_WEB_PARITY_PAGE_PATH="web_parity.html")
        add_test(NAME ui_light_web_parity COMMAND light_web_probe)
        set_tests_properties(ui_light_web_parity PROPERTIES TIMEOUT 15)
    endif()
    if(WIN32)
        if(TARGET ui_framework_shared)
            add_library(ui_lifecycle_fixture MODULE tests/app_lifecycle_fixture.c)
            target_link_libraries(ui_lifecycle_fixture PRIVATE ui_framework_shared)
            target_compile_definitions(ui_lifecycle_fixture PRIVATE _CRT_SECURE_NO_WARNINGS)
            add_executable(ui_workspace_integration_test tests/workspace_integration.c)
            target_link_libraries(ui_workspace_integration_test PRIVATE ui_framework_shared)
            target_compile_definitions(ui_workspace_integration_test PRIVATE _CRT_SECURE_NO_WARNINGS)
            add_dependencies(ui_workspace_integration_test minimal_eda_package ui_lifecycle_fixture)
            if(CMAKE_CXX_COMPILER)
                add_library(ui_application_cpp MODULE tests/application_cpp.cpp)
                target_link_libraries(ui_application_cpp PRIVATE ui_framework_shared)
                target_compile_features(ui_application_cpp PRIVATE cxx_std_11)
                add_dependencies(ui_workspace_integration_test ui_application_cpp)
                set(workspace_cpp_argument "$<TARGET_FILE:ui_application_cpp>")
                file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.ini"
                    "[application]\napp_id=org.ui.fixture.cpp\nname=C++ Counter\nversion=1.0.0\narchitecture=x64\nabi_version=1\nframework_api_version=9\nmodule=ui_application_cpp.dll\nmultiple_instances=true\n")
                add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.uapp"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/cpp_package"
                    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:ui_application_cpp>"
                        "${CMAKE_CURRENT_BINARY_DIR}/cpp_package/ui_application_cpp.dll"
                    COMMAND $<TARGET_FILE:uapp_pack> "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.ini"
                        "${CMAKE_CURRENT_BINARY_DIR}/cpp_package" "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.uapp"
                    DEPENDS ui_application_cpp uapp_pack "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.ini" VERBATIM)
                add_custom_target(ui_cpp_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.uapp")
                set(frontend_cpp_argument "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.uapp")
            endif()
            add_test(NAME ui_workspace_integration COMMAND ui_workspace_integration_test
                "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp" "$<TARGET_FILE:ui_lifecycle_fixture>"
                ${workspace_cpp_argument})
            set_tests_properties(ui_workspace_integration PROPERTIES TIMEOUT 45)
            # Current host behavior is validated with current SDK fixtures.
            if(TARGET framework_host)
                add_executable(ui_browser_host_test tests/browser_host.c "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
                target_include_directories(ui_browser_host_test PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
                target_link_libraries(ui_browser_host_test PRIVATE ui_framework_shared comdlg32 shell32 dwmapi)
                target_compile_definitions(ui_browser_host_test PRIVATE _CRT_SECURE_NO_WARNINGS)
                add_dependencies(ui_browser_host_test minimal_eda_package web_counter_package ui_lifecycle_fixture)
                if(MSVC)
                    target_compile_options(ui_browser_host_test PRIVATE /W4 /WX /utf-8)
                    set_property(TARGET ui_browser_host_test PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
                endif()
                add_test(NAME ui_browser_host COMMAND ui_browser_host_test
                    "${CMAKE_CURRENT_BINARY_DIR}/web_counter.uapp" "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp" "$<TARGET_FILE:ui_lifecycle_fixture>")
                set_tests_properties(ui_browser_host PROPERTIES TIMEOUT 60)
                add_executable(ui_web_host_frontend_test tests/web_host_frontend.c
                    "${CMAKE_CURRENT_BINARY_DIR}/generated/host.rc")
                target_include_directories(ui_web_host_frontend_test PRIVATE src examples/framework_host "${CMAKE_CURRENT_BINARY_DIR}/generated")
                target_link_libraries(ui_web_host_frontend_test PRIVATE ui_framework_shared comdlg32 shell32 dwmapi)
                target_compile_definitions(ui_web_host_frontend_test PRIVATE _CRT_SECURE_NO_WARNINGS)
                add_dependencies(ui_web_host_frontend_test minimal_eda_package web_counter_package)
                if(TARGET ui_cpp_package)
                    add_dependencies(ui_web_host_frontend_test ui_cpp_package)
                endif()
                if(MSVC)
                    target_compile_options(ui_web_host_frontend_test PRIVATE $<$<COMPILE_LANGUAGE:C>:/W4;/WX;/utf-8>)
                    set_property(TARGET ui_web_host_frontend_test PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
                endif()
                add_test(NAME ui_web_host_frontend COMMAND ui_web_host_frontend_test
                    "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp" ${frontend_cpp_argument})
                set_tests_properties(ui_web_host_frontend PROPERTIES TIMEOUT 60)
            endif()
            if(MSVC)
                foreach(new_target IN ITEMS ui_lifecycle_fixture ui_workspace_integration_test ui_application_cpp)
                    if(TARGET ${new_target})
                        target_compile_options(${new_target} PRIVATE /W4 /WX /utf-8)
                        set_property(TARGET ${new_target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
                    endif()
                endforeach()
            endif()
        endif()
        add_executable(ui_native_activation_test tests/native_activation.c)
        target_link_libraries(ui_native_activation_test PRIVATE ui_framework)
        target_compile_definitions(ui_native_activation_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        add_test(NAME ui_native_activation COMMAND ui_native_activation_test)
        add_executable(ui_dpi_integration_test
            tests/dpi_integration.c
        )
        target_link_libraries(ui_dpi_integration_test PRIVATE ui_framework)
        target_compile_definitions(ui_dpi_integration_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        add_test(NAME ui_dpi_integration COMMAND ui_dpi_integration_test)

        add_executable(ui_opengl_integration_test tests/opengl_integration.c)
        target_link_libraries(ui_opengl_integration_test PRIVATE ui_framework)
        target_compile_definitions(ui_opengl_integration_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        add_test(NAME ui_opengl_integration COMMAND ui_opengl_integration_test)

        add_executable(ui_surface_input_test tests/surface_input.c)
        target_link_libraries(ui_surface_input_test PRIVATE ui_framework)
        add_test(NAME ui_surface_input COMMAND ui_surface_input_test)

        add_executable(ui_monitor_transition_test tests/monitor_transition.c)
        target_link_libraries(ui_monitor_transition_test PRIVATE ui_framework)
        add_test(NAME ui_monitor_transition COMMAND ui_monitor_transition_test)
        set_tests_properties(ui_monitor_transition PROPERTIES TIMEOUT 20 SKIP_RETURN_CODE 77)

        add_executable(ui_workspace7_test tests/workspace7.c)
        target_link_libraries(ui_workspace7_test PRIVATE ui_framework)
        add_test(NAME ui_workspace7 COMMAND ui_workspace7_test)
        if(UI_FRAMEWORK_ENABLE_LIGHT_WEB)
            target_compile_definitions(ui_workspace7_test PRIVATE UI_WORKSPACE7_WEB=1)
        endif()
        add_executable(ui_native_shell_integration_test
            tests/native_shell_integration.c
        )
        target_link_libraries(ui_native_shell_integration_test PRIVATE ui_framework)
        target_compile_definitions(ui_native_shell_integration_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        add_test(NAME ui_native_shell_integration COMMAND ui_native_shell_integration_test)

        add_executable(ui_native_responsive_test tests/native_responsive.c)
        target_link_libraries(ui_native_responsive_test PRIVATE ui_framework)
        target_compile_definitions(ui_native_responsive_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        add_test(NAME ui_native_responsive COMMAND ui_native_responsive_test)

        add_executable(ui_assistant_protocol_test
            tests/assistant_protocol.c
        )
        target_link_libraries(ui_assistant_protocol_test PRIVATE ui_framework)
        target_compile_definitions(ui_assistant_protocol_test PRIVATE _CRT_SECURE_NO_WARNINGS)
        add_test(NAME ui_assistant_protocol COMMAND ui_assistant_protocol_test)
    else()
        add_executable(ui_assistant_protocol_test
            tests/assistant_protocol.c
        )
        target_link_libraries(ui_assistant_protocol_test PRIVATE ui_framework)
        add_test(NAME ui_assistant_protocol COMMAND ui_assistant_protocol_test)
    endif()
endif()

# CMake can double-decode Chinese MSVC /showIncludes diagnostics on a UTF-8
if(MSVC)
    foreach(_api5_target ui_menu_access_test ui_offscreen_msaa_test ui_webview2_render_test ui_session0_gl_app ui_session0_gl_test)
        if(TARGET ${_api5_target})
            target_compile_options(${_api5_target} PRIVATE /W4 /WX /utf-8)
            target_compile_definitions(${_api5_target} PRIVATE _CRT_SECURE_NO_WARNINGS)
        endif()
    endforeach()
endif()

# CMake can double-decode Chinese MSVC /showIncludes diagnostics on a UTF-8
# Windows console. Recover the actual prefix so Ninja tracks header changes.
if(MSVC AND CMAKE_CL_SHOWINCLUDES_PREFIX MATCHES "娉")
    set(CMAKE_CL_SHOWINCLUDES_PREFIX "注意: 包含文件:")
endif()
