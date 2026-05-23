enable_testing()

set(SIPHON_TEST_WORKDIR ${CMAKE_SOURCE_DIR})

add_executable(test_rarc test_rarc.c)
target_link_libraries(test_rarc PRIVATE siphon)
add_test(
    NAME    test_rarc
    COMMAND test_rarc
    WORKING_DIRECTORY ${SIPHON_TEST_WORKDIR}
)

add_executable(test_aes test_aes.c)
target_link_libraries(test_aes PRIVATE siphon)
target_include_directories(test_aes PRIVATE ${CMAKE_SOURCE_DIR}/src/disc)
add_test(NAME test_aes COMMAND test_aes)

macro(add_disc_test name)
    add_executable(test_${name} test_${name}.c test_disc_common.c)
    target_link_libraries(test_${name} PRIVATE siphon)
    target_compile_definitions(test_${name} PRIVATE
        SIPHON_TEST_ROOT="${CMAKE_SOURCE_DIR}")
    add_test(NAME test_${name} COMMAND test_${name})
endmacro()

add_disc_test(iso_gc)
add_disc_test(ciso_gc)
add_disc_test(gcm_gc)
add_disc_test(gcz_gc)
add_disc_test(iso_wii)
add_disc_test(ciso_wii)
add_disc_test(gcz_wii)
add_disc_test(wbfs_wii)