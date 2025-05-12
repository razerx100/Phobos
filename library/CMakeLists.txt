cmake_minimum_required(VERSION 3.21)

file(GLOB_RECURSE SRC src/*.cpp includes/*.hpp)

add_library(PhobosLib STATIC
    ${SRC}
)

target_include_directories(PhobosLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes/)

if(MSVC)
    target_compile_options(PhobosLib PRIVATE /fp:fast /MP /Ot /W4 /Gy /std:c++latest /Zc:__cplusplus)
endif()