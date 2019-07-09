cmake_minimum_required(VERSION 3.14)
project(sandbox)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(checks)
include(utest)

enable_testing()

set(CMAKE_CXX_STANDARD 20)  # string_view::starts_with
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT UNIX)
    message(WARRNING "Non-UNIX systems haven't been tested")
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: Debug, Release." FORCE
    )
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 8)
        message(FATAL_ERROR "GNU C++ compiler is too old: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()

    add_compile_options(
        $<$<CONFIG:Debug>:-O0$<SEMICOLON>-g>    # just an example of configuration-specific options
        $<$<CONFIG:Release>:-O3>                # just an example of configuration-specific options
        -Wall
        -Wextra
        -pedantic
        # -Weffc++
        -Wcast-align
        -Wcast-qual
        -Wcomment
        -Wconversion
        -Werror=return-type
        -Werror=uninitialized
        -Winit-self
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wstrict-null-sentinel
    )

    add_compile_definitions(
        $<$<CONFIG:Debug>:_DEBUG=1>
        $<$<CONFIG:Release>:NDEBUG=1>
    )

    option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
    option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

    if (ENABLE_ASAN AND ENABLE_TSAN)
        message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer are mutually exclusive")
    endif()

    if (ENABLE_ASAN)
        message(STATUS "Enabling AddressSanitizer")
        add_compile_options(
            -fsanitize=address
            -g3
        )
        add_link_options(
            -fsanitize=address
            -lasan
        )
    endif()

    if (ENABLE_TSAN)
        message(STATUS "Enabling ThreadSanitizer")
        add_compile_options(
            -fsanitize=thread
            -g3
            -fPIE
        )
        add_link_options(
            -fsanitize=thread
            -ltsan
            -pie
        )
    endif()
elseif()
    message(WARNING "Non-GCC compiler! Command line options may be inconsistent.")
endif()

find_package(Boost 1.65 REQUIRED
    COMPONENTS
        unit_test_framework
)

StaticCheck(
    NAME
        utils
    HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/test
)

add_subdirectory(http)      # http-client demo

add_subdirectory(utils)     # generic utils
add_subdirectory(test)      # generic tests-suite
