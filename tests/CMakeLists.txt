

# LD_FLAGS = -lgtest -pthread

# Set compiler flags for gcc
if(CMAKE_C_COMPILER_ID MATCHES GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11 -fprofile-arcs -ftest-coverage")
endif()

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

add_executable(solver_reduce solver_reduce.cpp)
target_link_libraries(solver_reduce gtest pthread libqbsolv)
add_test(solver_reduce solver_reduce)

add_executable(util_malloc util_malloc.cpp)
target_link_libraries(util_malloc gtest pthread libqbsolv)
add_test(util_malloc util_malloc)
