macro(BoostTest name sources)
    add_executable(ut_${name}
        ${sources}
    )

    target_compile_definitions(
        ut_${name}
            PRIVATE BOOST_TEST_DYN_LINK
    )

    target_link_libraries(ut_${name}
        ${ARGN}
        Boost::unit_test_framework
    )

    add_test(
        NAME ${name}
        COMMAND $<TARGET_FILE:ut_${name}>
    )
endmacro()
