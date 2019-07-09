StaticCheck(
    NAME
        http_client
    HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/client
)

add_subdirectory(client)

if (Boost_FOUND)
    add_subdirectory(test)
endif()
