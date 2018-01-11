//  gcc ../src/util.o testroundit.c
//  this is not an automatic test run
//  it is for visual inspection to see if rounding is working as expected

#include "../src/util.c"
#include "../src/extern.h"
#include "../src/include.h"

FILE *outFile_;
int maxNodes_, nCouplers_, nNodes_, findMax_;
int Verbose_, SubMatrix_, UseDwave_, TargetSet_, WriteMatrix_, Tlist_;
char *outFileNm_, pgmName_[16];
double **val;
double Target_, Time_;
struct nodeStr_ *nodes_;
struct nodeStr_ *couplers_;

int main(int argc, char **argv) {
    double x, xd;
    x = 23.87345689887088;
    for (int n = 0; n < 17; n++) {
        xd = roundit(x, n);
        printf(" %.16e to %.16e diff %.6e ,n= %d\n", x, xd, xd - x, n);
    }
    x = 2399.87345689887088;
    for (int n = 0; n < 17; n++) {
        xd = roundit(x, n);
        printf(" %.16e to %.16e diff %.6e ,n= %d\n", x, xd, xd - x, n);
    }
    x = 0.0087345689883457088;
    for (int n = 0; n < 17; n++) {
        xd = roundit(x, n);
        printf(" %.16e to %.16e diff %.6e ,n= %d\n", x, xd, xd - x, n);
    }
    x = -23.87345689887034588;
    for (int n = 0; n < 17; n++) {
        xd = roundit(x, n);
        printf(" %.16e to %.16e diff %.6e ,n= %d\n", x, xd, xd - x, n);
    }
}
