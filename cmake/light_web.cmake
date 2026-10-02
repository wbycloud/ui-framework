# Integration for the controlled Web backend used by the standalone host.
#
# Usage after ui_framework has been created:
#   include(cmake/light_web.cmake)
#   ui_enable_light_web(ui_framework)
#
# Dependency projects are linked statically. Native embedding builds can
# explicitly disable the standalone host and this backend.
set(_UI_LIGHT_WEB_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ui_enable_light_web target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "ui_enable_light_web: target '${target}' does not exist")
    endif()
    if(NOT WIN32)
        message(FATAL_ERROR "ui_enable_light_web currently supports Windows only")
    endif()
    get_target_property(_light_web_already_enabled ${target} UI_LIGHT_WEB_ENABLED)
    if(_light_web_already_enabled)
        return()
    endif()

    set(UI_LEXBOR_SOURCE_DIR "${_UI_LIGHT_WEB_CMAKE_DIR}/../.deps/lexbor" CACHE PATH
        "Bundled Lexbor source directory")
    set(UI_QUICKJS_SOURCE_DIR "${_UI_LIGHT_WEB_CMAKE_DIR}/../.deps/quickjs" CACHE PATH
        "Bundled QuickJS-NG source directory")
    if(NOT EXISTS "${UI_LEXBOR_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "Lexbor source was not found: ${UI_LEXBOR_SOURCE_DIR}")
    endif()
    if(NOT EXISTS "${UI_QUICKJS_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "QuickJS-NG source was not found: ${UI_QUICKJS_SOURCE_DIR}")
    endif()

    set(LEXBOR_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(LEXBOR_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(LEXBOR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LEXBOR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(LEXBOR_BUILD_UTILS OFF CACHE BOOL "" FORCE)
    set(QJS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(QJS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    if(NOT TARGET lexbor_static)
        add_subdirectory("${UI_LEXBOR_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/light_web/lexbor" EXCLUDE_FROM_ALL)
    endif()
    if(NOT TARGET qjs)
        add_subdirectory("${UI_QUICKJS_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/light_web/quickjs" EXCLUDE_FROM_ALL)
    endif()

    target_sources(${target} PRIVATE
        "${_UI_LIGHT_WEB_CMAKE_DIR}/../src/backends/light_web.c")
    target_include_directories(${target} PRIVATE
        "${UI_LEXBOR_SOURCE_DIR}/source"
        "${UI_QUICKJS_SOURCE_DIR}")
    target_compile_definitions(${target} PRIVATE UI_FRAMEWORK_ENABLE_LIGHT_WEB=1)
    target_link_libraries(${target} PRIVATE lexbor_static qjs)
    set_property(TARGET ${target} PROPERTY UI_LIGHT_WEB_ENABLED TRUE)
endfunction()
