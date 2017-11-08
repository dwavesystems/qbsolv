#!/bin/bash
set -e -x

# Install dependencies
pip install --user cpp-coveralls;
export CC=$C_COMPILER;
export CXX=$CXX_COMPILER;

# Build and install gtest
mkdir ~/gtest;
pushd ~/gtest;
export LIBRARY_PATH=$(pwd);
wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz
tar -xzvf release-1.8.0.tar.gz;
cd ./googletest-release-1.8.0; mkdir build; cd build
cmake -D CMAKE_INSTALL_PREFIX:PATH=./ ..
make CC=$CC CXX=$CXX
popd

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
