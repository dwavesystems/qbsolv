#!/bin/bash
set -e -x

# Install dependencies
### pip install --user cpp-coveralls;
export CC=$C_COMPILER;
export CXX=$CXX_COMPILER;

# Build and install gtest
mkdir ~/gtest;
pushd ~/gtest;
export LIBRARY_PATH=$(pwd);
wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz
tar -xzf release-1.8.0.tar.gz;
cd ./googletest-release-1.8.0; mkdir build; cd build
cmake ..
make CC=$CC CXX=$CXX
sudo make install
export CPLUS_INCLUDE_PATH=/usr/local/include
export LIBRARY_PATH=/usr/local/lib
popd

# Build the tests
mkdir build;
cd build;
cmake .. -DQBSOLV_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug;
make CC=$CC CXX=$CXX;

# Run the tests
./tests/all_tests;

# Stop after running the unit tests, the linux builds will submit the coverage data
#
# Gather the test coverage files
### find . \( -name '*.gcno' -or -name '*.gcda' \) -exec mv {} .. \;
### cd -;

# Submit the traces to coveralls
### coveralls --exclude tests -E '.*gtest.*' --gcov-options '\-lp';
