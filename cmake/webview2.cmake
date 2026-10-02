# Optional WebView2 backend. The SDK is intentionally kept out of the core
# target and is downloaded/extracted under .deps by the project maintainer.
function(ui_framework_enable_webview2 target)
    if(NOT WIN32)
        message(FATAL_ERROR "WebView2 is only supported on Windows")
    endif()
    set(UI_WEBVIEW2_ROOT "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../.deps/Microsoft.Web.WebView2.1.0.4129.50")
    if(CMAKE_MSVC_C_ARCHITECTURE_ID STREQUAL "ARM64")
        set(loader_arch arm64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(loader_arch x64)
    else()
        set(loader_arch x86)
    endif()
    if(NOT EXISTS "${UI_WEBVIEW2_ROOT}/build/native/include/WebView2.h")
        message(FATAL_ERROR "WebView2 SDK not found at ${UI_WEBVIEW2_ROOT}")
    endif()
    target_sources(${target} PRIVATE
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../src/backends/webview2.c")
    target_include_directories(${target} PRIVATE
        "${UI_WEBVIEW2_ROOT}/build/native/include")
    target_link_libraries(${target} PRIVATE
        "${UI_WEBVIEW2_ROOT}/build/native/${loader_arch}/WebView2LoaderStatic.lib"
        shlwapi ole32 user32 advapi32)
    target_compile_definitions(${target} PRIVATE UI_FRAMEWORK_HAS_WEBVIEW2=1)

endfunction()
