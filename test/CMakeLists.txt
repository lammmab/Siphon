enable_testing()

set(SIPHON_TEST_WORKDIR ${CMAKE_SOURCE_DIR})

add_executable(test_rarc test_rarc.c)
target_link_libraries(test_rarc PRIVATE siphon)
add_test(
    NAME    test_rarc
    COMMAND test_rarc
    WORKING_DIRECTORY ${SIPHON_TEST_WORKDIR}
)

add_executable(test_ciso_gc test_ciso_gc.c)
target_link_libraries(test_ciso_gc PRIVATE siphon)
add_test(
    NAME    test_ciso_gc
    COMMAND test_ciso_gc
    WORKING_DIRECTORY ${SIPHON_TEST_WORKDIR}
)