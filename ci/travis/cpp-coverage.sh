#!/bin/bash
set -e -x

# Install dependencies
pip install --user cpp-coveralls;
export CC=$C_COMPILER;
export CXX=$CXX_COMPILER;

# Build the tests
mkdir build;
cd build;
cmake .. -DQBSOLV_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug;
make CC=$CC CXX=$CXX;

# Run the tests
./tests/all_tests;

# Gather the test coverage files
find . \( -name '*.gcno' -or -name '*.gcda' \) -exec mv {} .. \;
cd -;

# # Submit the traces to coveralls
# coveralls --exclude tests -E '.*gtest.*' --gcov-options '\-lp';
