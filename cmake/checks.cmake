include(utils)

# Static/dynamic code analysis
if (NOT TARGET check)
    add_custom_target(check)
endif()

# FlawFinder
find_program(FLAWFINDER_PATH flawfinder DOC "flawfinder path")
function(FlawFinder)
    if (FLAWFINDER_PATH)
        cmake_parse_arguments(FLAWFINDER
            ""
            "NAME"
            "SOURCES"
            ${ARGN}
        )
        if (NOT FLAWFINDER_NAME)
            set(FLAWFINDER_NAME "flawfinder")
        else()
            set(FLAWFINDER_NAME "flawfinder_${FLAWFINDER_NAME}")
        endif()

        add_custom_target(${FLAWFINDER_NAME}
            COMMAND ${FLAWFINDER_PATH}
                --quiet
                ${FLAWFINDER_SOURCES}
            COMMENT "${FLAWFINDER_NAME}"
        )
        add_dependencies(check ${FLAWFINDER_NAME})
    endif()
endfunction()

# CppCheck
find_program(CPPCHECK_PATH cppcheck DOC "cppcheck path")
function(CppCheck)
    if (CPPCHECK_PATH)
        cmake_parse_arguments(CPPCHECK
            ""
            "NAME;ENABLE"
            "HEADERS;SOURCES"
            ${ARGN}
        )
        if (NOT CPPCHECK_NAME)
            set(CPPCHECK_NAME "cppcheck")
        else()
            set(CPPCHECK_NAME "cppcheck_${CPPCHECK_NAME}")
        endif()
        SetIfNot(CPPCHECK_ENABLE "all")

        set(header_args)
        foreach(path IN LISTS CPPCHECK_HEADERS)
            list(APPEND header_args -I ${path})
        endforeach()

        add_custom_target(${CPPCHECK_NAME}
            COMMAND ${CPPCHECK_PATH}
                --quiet
                --enable=${CPPCHECK_ENABLE}
                ${header_args}
                ${CPPCHECK_SOURCES}
            COMMENT "${CPPCHECK_NAME}"
        )
        add_dependencies(check ${CPPCHECK_NAME})
    endif()
endfunction()

# Shortcut for FlawFinder(...), CppCheck(...)
function(StaticCheck)
    cmake_parse_arguments(CHECK
        ""
        "NAME"
        "HEADERS;SOURCES"
        ${ARGN}
    )
    CppCheck(
        NAME    ${CHECK_NAME}
        HEADERS ${CHECK_HEADERS}
        SOURCES ${CHECK_SOURCES}
    )

    FlawFinder(
        NAME    ${CHECK_NAME}
        SOURCES ${CHECK_SOURCES}
    )
endfunction()

# Valgrind
find_program(VALGRIND_PATH valgrind DOC "valgrind path")
# TODO
