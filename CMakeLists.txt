cmake_minimum_required(VERSION 3.21)

project(Phobos
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ADD_TEST_PHOBOS "If test should be built" OFF)

add_subdirectory(library)

if(ADD_TEST_PHOBOS)
    enable_testing()
    add_subdirectory(test)
endif()

add_library(razer::phobos ALIAS PhobosLib)
