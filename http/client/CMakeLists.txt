add_library(http
    http.cpp
)
target_include_directories(http
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/.."     # include as <client/...>
)
target_link_libraries(http
    utils
)

add_executable(http_client
    http_client.cpp
)
target_link_libraries(http_client
    http
    utils
)
