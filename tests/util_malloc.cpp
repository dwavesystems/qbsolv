#include "../src/util.h"
#include "../src/extern.h"
#include "gtest/gtest.h"

// FILE            *outFile_;
// FILE            *solution_input_;
// int             maxNodes_, nCouplers_, nNodes_, findMax_,numsolOut_;
// int             Verbose_, TargetSet_, WriteMatrix_, Tlist_;
// char            *outFileNm_, pgmName_[16], algo_[4];
// double          **val;
// double          Target_, Time_;
// struct nodeStr_ *nodes_;
// struct nodeStr_ *couplers_;

template <class Type>
void checkMatrix(size_t rows, size_t cols) {
    // Allocate
    Type** matrix = (Type**)malloc2D(rows, cols, sizeof(Type));

    // Fill the matrix with predictable numbers
    int counter = 0;
    for (size_t rr = 0; rr < rows; rr++) {
        for (size_t cc = 0; cc < cols; cc++) {
            matrix[rr][cc] = counter++;
        }
    }

    // Disect the matrix
    char** lookup = (char**)matrix;
    Type* data_block = (Type*)(lookup + rows);

    // Check the data buffer
    for (size_t ii = 0; ii < rows * cols; ii++) {
        // Make sure the numerical entries in the matrix are in the order we
        // expect (row major)
        EXPECT_EQ(data_block[ii], ii);

        // Make sure the lookup table points to the beginning of every row
        if (ii % cols == 0) {
            EXPECT_EQ((void*)lookup[(ii / cols)], (void*)&data_block[ii]);
        }
    }
}

TEST(util_malloc, memory_layout) {
    checkMatrix<uint32_t>(1, 1);
    checkMatrix<uint32_t>(2, 2);
    checkMatrix<uint32_t>(5, 5);
    checkMatrix<uint32_t>(100, 100);
}

TEST(util_malloc, non_square) {
    checkMatrix<uint32_t>(1, 5);
    checkMatrix<uint32_t>(5, 1);
    checkMatrix<uint32_t>(10, 100);
    checkMatrix<uint32_t>(100, 10);
}

TEST(util_malloc, types) {
    checkMatrix<uint8_t>(1, 5);
    checkMatrix<uint8_t>(5, 1);
    checkMatrix<uint8_t>(5, 5);

    checkMatrix<int8_t>(1, 5);
    checkMatrix<int8_t>(5, 1);
    checkMatrix<int8_t>(5, 5);

    checkMatrix<uint16_t>(1, 5);
    checkMatrix<uint16_t>(5, 1);
    checkMatrix<uint16_t>(5, 5);

    checkMatrix<uint32_t>(1, 5);
    checkMatrix<uint32_t>(5, 1);
    checkMatrix<uint32_t>(5, 5);

    checkMatrix<int32_t>(1, 5);
    checkMatrix<int32_t>(5, 1);
    checkMatrix<int32_t>(5, 5);

#ifdef INT64_MAX
    checkMatrix<uint64_t>(1, 5);
    checkMatrix<uint64_t>(5, 1);
    checkMatrix<uint64_t>(5, 5);
#endif

    checkMatrix<float>(1, 5);
    checkMatrix<float>(5, 1);
    checkMatrix<float>(5, 5);

    checkMatrix<double>(1, 5);
    checkMatrix<double>(5, 1);
    checkMatrix<double>(5, 5);
}
