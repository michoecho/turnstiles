add_executable(trivial_test trivial_test.cpp)
target_link_libraries(trivial_test turnstile_lib)
target_link_libraries(trivial_test "${CMAKE_THREAD_LIBS_INIT}")
add_test(trivial_test trivial_test)

