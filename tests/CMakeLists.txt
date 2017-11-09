

set(CMAKE_C_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

# Set compiler flags for gcc
if(CMAKE_C_COMPILER_ID MATCHES GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=gnu11 -fprofile-arcs -ftest-coverage")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()

add_executable(solver_reduce solver_reduce.cpp ../python/globals.cc ../src/util.cc ../src/solver.cc ../src/dwsolv.cc)
target_link_libraries(solver_reduce gtest gtest_main pthread)
add_test(solver_reduce solver_reduce)

add_executable(util_malloc util_malloc.cpp ../python/globals.cc ../src/util.cc)
target_link_libraries(util_malloc gtest gtest_main pthread)
add_test(util_malloc util_malloc)

add_executable(all_tests util_malloc.cpp solver_reduce.cpp ../python/globals.cc ../src/solver.cc ../src/dwsolv.cc ../src/util.cc)
target_link_libraries(all_tests gtest gtest_main pthread)
