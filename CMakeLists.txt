cmake_minimum_required(VERSION 3.16)
project(catbox VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -O2
    )
endif()

include_directories(include)

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(LIBCAP QUIET libcap)
endif()

set(SOURCES
    src/main.cpp
    src/cli.cpp
    src/sync.cpp
    src/isolation.cpp
)

add_executable(catbox ${SOURCES})

if (LIBCAP_FOUND)
    target_link_libraries(catbox PRIVATE ${LIBCAP_LIBRARIES})
    target_include_directories(catbox PRIVATE ${LIBCAP_INCLUDE_DIRS})
    target_compile_definitions(catbox PRIVATE HAVE_LIBCAP=1)
else()
    find_library(CAP_LIB cap)
    if (CAP_LIB)
        target_link_libraries(catbox PRIVATE ${CAP_LIB})
        target_compile_definitions(catbox PRIVATE HAVE_LIBCAP=1)
    endif()
endif()
