#!/bin/bash
set -e -x

# Install dependencies
pip install --user cpp-coveralls;
export CC=$C_COMPILER;
export CXX=$CXX_COMPILER;

# Build and install gtest
mkdir ~/gtest
cd ~/gtest
export LIBRARY_PATH=$(pwd)
cmake -D CMAKE_INSTALL_PREFIX:PATH=./ /usr/src/gtest/
make CC=$CC CXX=$CXX
cd -

# Build the tests
mkdir build;
cd build;
cmake .. -DQBSOLV_BUILD_TESTS=ON;
make CC=$CC CXX=$CXX;

# Run the tests
make test;

# Gather the test coverage files
find . \( -name '*.gcno' -or -name '*.gcda' \) -exec mv {} .. \;
cd -;

# Submit the traces to coveralls
coveralls --exclude tests -E '.*gtest.*' --gcov-options '\-lp';
