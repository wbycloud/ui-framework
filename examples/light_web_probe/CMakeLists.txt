cmake_minimum_required(VERSION 3.20)
project(light_web_probe C)

set(UI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." framework)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/light_web.cmake")
ui_enable_light_web(ui_framework)

add_executable(light_web_probe probe.c)
target_include_directories(light_web_probe PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../include")
target_link_libraries(light_web_probe PRIVATE ui_framework user32 gdi32)
target_compile_features(light_web_probe PRIVATE c_std_11)
target_compile_definitions(light_web_probe PRIVATE
    UI_WEB_PARITY_PAGE_PATH="${CMAKE_CURRENT_LIST_DIR}/../web_common/assistant.html")
if(MSVC)
    target_compile_options(light_web_probe PRIVATE /W4 /WX /utf-8)
endif()
enable_testing()
add_test(NAME light_web_probe COMMAND light_web_probe)
