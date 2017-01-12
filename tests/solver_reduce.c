
#include "../src/include.h"
#include "../src/extern.h"

void reduce(int *Icompress, double **val, int subMatrix,int maxNodes,double **val_s,short *Q,short *Q_s);

FILE   *inFile_, *outFile_;
int    maxNodes_,nCouplers_,nNodes_,nRepeats_,findMax_;
int    Verbose_,SubMatrix_,UseDwave_,TargetSet_,RepeatPass_,WriteMatrix_,Tlist_;
char   *inFileNm_,*outFileNm_,pgmName_[16];
double **val;
double Target_,Time_;
struct nodeStr_  *nodes_;
struct nodeStr_  *couplers_;

void test_eq(int a, int b){
    if(a == b){
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }
}

void test_eqf(double a, double b){
    if(a == b){
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }
}

void testOne(){
    // -------------------------------------------------------------------------
    // TEST 1:  Extracting one conditioned variable from a two variable system
    printf("Single variable from two variable system:\n");

    // -- Bootstrap
    // Declare the full QUBO
    int maxNodes = maxNodes_ = 2;
    double ** quboMat = (double**)malloc2D(2, 2, sizeof(double));

    // Encode simple 2 variable system
    // E(a, b) = 2a + 2ab + 3b
    quboMat[0][0] = 2;
    quboMat[1][1] = 3;
    quboMat[0][1] = 2;

    // selection variables
    int selectionMapping[1];
    short globalState[2];

    // output variables
    short selectionState[1];
    double ** selectionMat = (double**)malloc2D(1, 1, sizeof(double));

    // -- part 1a
    // E(a, b=0) = 2a + 2a(0) + 3(0)
    //           = 2a
    selectionMapping[0] = 0;
    globalState[0] = 0;
    globalState[1] = 0;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(2, selectionMat[0][0]);
    test_eq(0, selectionState[0]);

    // -- part 1b, same but with change in initialization of selected variable
    // E(a, b=0) = 2a + 2a(0) + 3(0)
    //           = 2a
    selectionMapping[0] = 0;
    globalState[0] = 1;
    globalState[1] = 0;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(2, selectionMat[0][0]);
    test_eq(1, selectionState[0]);

    // -- part 2a
    // E(a, b=1) = 2a + 2a(1) + 3(1)
    //           = 4a + c
    selectionMapping[0] = 0;
    globalState[0] = 0;
    globalState[1] = 1;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(4, selectionMat[0][0]);
    test_eq(0, selectionState[0]);

    // -- part 2b, same but with change in initialization of selected variable
    // E(a, b=1) = 2a + 2a(1) + 3(1)
    //           = 4a + c
    selectionMapping[0] = 0;
    globalState[0] = 1;
    globalState[1] = 1;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(4, selectionMat[0][0]);
    test_eq(1, selectionState[0]);

    // -- part 3a
    // E(a=0, b) = 2(0) + 2(0)b + 3b
    //           = 3b
    selectionMapping[0] = 1;
    globalState[0] = 0;
    globalState[1] = 0;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(3, selectionMat[0][0]);
    test_eq(0, selectionState[0]);

    // -- part 3b, same but with change in initialization of selected variable
    selectionMapping[0] = 1;
    globalState[0] = 0;
    globalState[1] = 1;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(3, selectionMat[0][0]);
    test_eq(1, selectionState[0]);

    // -- part 4a
    // E(a=1, b) = 2(1) + 2(1)b + 3b
    //           = 5b + c
    selectionMapping[0] = 1;
    globalState[0] = 1;
    globalState[1] = 0;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(5, selectionMat[0][0]);
    test_eq(0, selectionState[0]);

    // -- part 4b, same but with change in initialization of selected variable
    selectionMapping[0] = 1;
    globalState[0] = 1;
    globalState[1] = 1;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(5, selectionMat[0][0]);
    test_eq(1, selectionState[0]);
}

void testTwo(){
    // -------------------------------------------------------------------------
    // TEST 2:  Same tests as test 1 but with some unused variables inserted
    // in the beginning of the matrix, should be ignored.
    //
    // This test should run EXACTLY the same as test 1, but with the index
    // of `a` being 2, and the index of `b` being 3
    printf("Single variable from two variable system, extra variables:\n");

    // -- Bootstrap
    // Declare the full QUBO
    int maxNodes = maxNodes_ = 4;
    double ** quboMat = (double**)malloc2D(4, 4, sizeof(double));

    // Encode simple 2 variable system
    // E(a, b) = 2a + 2ab + 3b
    quboMat[2][2] = 2;
    quboMat[3][3] = 3;
    quboMat[2][3] = 2;

    // selection variables
    int selectionMapping[1];
    short globalState[2];

    // output variables
    short selectionState[1];
    double ** selectionMat = (double**)malloc2D(1, 1, sizeof(double));

    // -- part 1a
    // E(a, b=0) = 2a + 2a(0) + 3(0)
    //           = 2a
    selectionMapping[0] = 2;
    globalState[2] = 0;
    globalState[3] = 0;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(2, selectionMat[0][0]);
    test_eq(0, selectionState[0]);

    // -- part 1b, same but with change in initialization of selected variable
    // E(a, b=0) = 2a + 2a(0) + 3(0)
    //           = 2a
    selectionMapping[0] = 2;
    globalState[2] = 1;
    globalState[3] = 0;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(2, selectionMat[0][0]);
    test_eq(1, selectionState[0]);

    // -- part 2a
    // E(a, b=1) = 2a + 2a(1) + 3(1)
    //           = 4a + c
    selectionMapping[0] = 2;
    globalState[2] = 0;
    globalState[3] = 1;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(4, selectionMat[0][0]);
    test_eq(0, selectionState[0]);

    // -- part 2b, same but with change in initialization of selected variable
    // E(a, b=1) = 2a + 2a(1) + 3(1)
    //           = 4a + c
    selectionMapping[0] = 2;
    globalState[2] = 1;
    globalState[3] = 1;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(4, selectionMat[0][0]);
    test_eq(1, selectionState[0]);

    // -- part 3a
    // E(a=0, b) = 2(0) + 2(0)b + 3b
    //           = 3b
    selectionMapping[0] = 3;
    globalState[2] = 0;
    globalState[3] = 0;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(3, selectionMat[0][0]);
    test_eq(0, selectionState[0]);

    // -- part 3b, same but with change in initialization of selected variable
    selectionMapping[0] = 3;
    globalState[2] = 0;
    globalState[3] = 1;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(3, selectionMat[0][0]);
    test_eq(1, selectionState[0]);

    // -- part 4a
    // E(a=1, b) = 2(1) + 2(1)b + 3b
    //           = 5b + c
    selectionMapping[0] = 3;
    globalState[2] = 1;
    globalState[3] = 0;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(5, selectionMat[0][0]);
    test_eq(0, selectionState[0]);

    // -- part 4b, same but with change in initialization of selected variable
    selectionMapping[0] = 3;
    globalState[2] = 1;
    globalState[3] = 1;

    reduce(selectionMapping, quboMat, 1, maxNodes, selectionMat, globalState, selectionState);

    test_eqf(5, selectionMat[0][0]);
    test_eq(1, selectionState[0]);
}

int main(int argc, char *argv[]){

    testOne();
    testTwo();

    //
    return 0;
}
