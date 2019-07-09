add_library(testlib
    test_io.cpp
)

target_include_directories(testlib
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/.."     # include as <test/...>
)

target_link_libraries(testlib
    utils
)
