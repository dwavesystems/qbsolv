cmake_minimum_required(VERSION 3.0)

project(qbsolv)
# set(VERSION_MAJOR 2)
# set(VERSION_MINOR 5)
# set(VERSION_PATCH 0)

option(QBSOLV_BUILD_CMD "Build compiled command line interface." ON)
option(QBSOLV_BUILD_TESTS "Build c library unit tests." ON)

# Set compiler flags for gcc
if(CMAKE_C_COMPILER_ID MATCHES GNU)
    set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wall -Wextra -std=gnu99")
    set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
endif()

# location of header files
include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

# static library
add_library(libqbsolv STATIC src/solver.c src/util.c src/dwsolv.c)
set_target_properties(libqbsolv PROPERTIES PREFIX "")

if(QBSOLV_BUILD_CMD)
    # compile main executable
    add_executable(qbsolv cmd/main.c cmd/readqubo.c)

    # link library
    target_link_libraries(qbsolv libqbsolv)
endif()

if(QBSOLV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
