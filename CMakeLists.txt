cmake_minimum_required(VERSION 3.20)

project(ui_framework VERSION 0.1.0 LANGUAGES C)

option(UI_FRAMEWORK_ENABLE_WEBVIEW2
       "Build the optional WebView2 backend (requires .deps SDK)" OFF)
option(UI_FRAMEWORK_ENABLE_LIGHT_WEB
       "Build the optional Lexbor/QuickJS controlled Web backend" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(UI_BUILD_TESTS "Build framework integration tests" ON)
configure_file(examples/web_common/assistant.html web_parity.html COPYONLY)

add_library(ui_framework STATIC
    src/ui.c
    src/assistant.c
    src/package.c
)

target_include_directories(ui_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(WIN32)
    target_sources(ui_framework PRIVATE
        src/platform/win32_surface.c
        src/platform/win32_shell.c)
    target_link_libraries(ui_framework PUBLIC user32 gdi32 opengl32 comctl32)
else()
    target_sources(ui_framework PRIVATE
        src/platform/headless_surface.c
        src/platform/headless_shell.c)
endif()

if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_library(ui_framework_shared SHARED src/ui.c src/assistant.c src/package.c
        src/workspace.c src/platform/win32_surface.c src/platform/win32_shell.c)
    set_target_properties(ui_framework_shared PROPERTIES
        OUTPUT_NAME ui_framework ARCHIVE_OUTPUT_NAME ui_framework_runtime)
    target_include_directories(ui_framework_shared PUBLIC include PRIVATE src)
    target_compile_definitions(ui_framework_shared PUBLIC UI_FRAMEWORK_BUILD_SHARED
        PRIVATE UI_FRAMEWORK_BUILDING _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(ui_framework_shared PUBLIC user32 gdi32 opengl32 comctl32)
    if(MSVC)
        target_compile_options(ui_framework_shared PRIVATE /W4 /WX /utf-8)
    endif()
    add_executable(framework_host WIN32 examples/framework_host/main.c)
    target_link_libraries(framework_host PRIVATE ui_framework_shared comdlg32 shell32)
    target_compile_definitions(framework_host PRIVATE _CRT_SECURE_NO_WARNINGS)
    add_library(minimal_eda_app MODULE examples/minimal_eda/app.c)
    target_link_libraries(minimal_eda_app PRIVATE ui_framework_shared)
    target_compile_definitions(minimal_eda_app PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(MSVC)
        target_compile_options(framework_host PRIVATE /W4 /WX /utf-8)
        target_compile_options(minimal_eda_app PRIVATE /W4 /WX /utf-8)
        set_target_properties(ui_framework_shared framework_host minimal_eda_app
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
        ui_enable_light_web(ui_framework_shared)
    endif()
    add_executable(light_web_probe examples/light_web_probe/probe.c)
    target_link_libraries(light_web_probe PRIVATE ui_framework)
    target_compile_definitions(light_web_probe PRIVATE
        UI_WEB_PARITY_PAGE_PATH="web_parity.html")
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
    add_executable(ui_package_format_test tests/package_format.c)
    target_link_libraries(ui_package_format_test PRIVATE ui_framework)
    add_test(NAME ui_package_format COMMAND ui_package_format_test)
    add_library(ui_framework_headless STATIC src/ui.c src/assistant.c
        src/platform/headless_surface.c src/platform/headless_shell.c)
    target_include_directories(ui_framework_headless PUBLIC include PRIVATE src)
    foreach(test_name IN ITEMS responsive_layout assistant_lifecycle dpi_web_failure)
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
    endif()
    if(UI_FRAMEWORK_ENABLE_LIGHT_WEB)
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
            add_executable(ui_host_frontend_test tests/host_frontend.c)
            target_link_libraries(ui_host_frontend_test PRIVATE user32 comctl32)
            target_compile_definitions(ui_host_frontend_test PRIVATE _CRT_SECURE_NO_WARNINGS)
            add_dependencies(ui_host_frontend_test framework_host minimal_eda_package)
            if(CMAKE_CXX_COMPILER)
                add_library(ui_application_cpp MODULE tests/application_cpp.cpp)
                target_link_libraries(ui_application_cpp PRIVATE ui_framework_shared)
                target_compile_features(ui_application_cpp PRIVATE cxx_std_11)
                add_dependencies(ui_workspace_integration_test ui_application_cpp)
                set(workspace_cpp_argument "$<TARGET_FILE:ui_application_cpp>")
                file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.ini"
                    "[application]\napp_id=org.ui.fixture.cpp\nname=C++ Counter\nversion=1.0.0\narchitecture=x64\nabi_version=1\nframework_api_version=1\nmodule=ui_application_cpp.dll\nmultiple_instances=true\n")
                add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.uapp"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/cpp_package"
                    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:ui_application_cpp>"
                        "${CMAKE_CURRENT_BINARY_DIR}/cpp_package/ui_application_cpp.dll"
                    COMMAND $<TARGET_FILE:uapp_pack> "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.ini"
                        "${CMAKE_CURRENT_BINARY_DIR}/cpp_package" "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.uapp"
                    DEPENDS ui_application_cpp uapp_pack "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.ini" VERBATIM)
                add_custom_target(ui_cpp_package ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.uapp")
                add_dependencies(ui_host_frontend_test ui_cpp_package)
                set(frontend_cpp_argument "${CMAKE_CURRENT_BINARY_DIR}/cpp_fixture.uapp")
            endif()
            add_test(NAME ui_workspace_integration COMMAND ui_workspace_integration_test
                "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp" "$<TARGET_FILE:ui_lifecycle_fixture>"
                ${workspace_cpp_argument})
            set_tests_properties(ui_workspace_integration PROPERTIES TIMEOUT 45)
            add_test(NAME ui_host_frontend COMMAND ui_host_frontend_test
                "$<TARGET_FILE:framework_host>" "${CMAKE_CURRENT_BINARY_DIR}/minimal_eda.uapp"
                ${frontend_cpp_argument})
            set_tests_properties(ui_host_frontend PROPERTIES TIMEOUT 60)
            if(MSVC)
                foreach(new_target IN ITEMS ui_lifecycle_fixture ui_workspace_integration_test ui_host_frontend_test ui_application_cpp)
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
# Windows console. Recover the actual prefix so Ninja tracks header changes.
if(MSVC AND CMAKE_CL_SHOWINCLUDES_PREFIX MATCHES "娉")
    set(CMAKE_CL_SHOWINCLUDES_PREFIX "注意: 包含文件:")
endif()
