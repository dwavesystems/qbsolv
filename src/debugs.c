/*
 Copyright 2017 D-Wave Systems Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include "extern.h"
#include "util.h"

// this DEBUG function just checks for a corrupt Q bit vector
int check_corrupt_Q(int8_t *Q, int N) {
    int i;
    int ret;

    ret = false;
    for (i = 0; i < N; i++) {
        if (Q[i] / 2 != 0) {  // this is not a 1 or zero
            printf("Q not 1 or zero , Q %d i %d\n", Q[i], i);
            exit(9);
            ret = true;
        }
    }
    return ret;
}

// this DEBUG function just checks for a corrupt tabu list
int check_corrupt_tabu(int *T, int N, int nTabu) {
    int i;
    int ret;

    ret = false;
    for (i = 0; i < N; i++) {
        if ((T[i] > nTabu) | (T[i] < 0)) {  // this is not out of Range
            printf("Tabu is not in Range , Tabu[%d] = %d\n", i, T[i]);
            exit(9);
            ret = true;
        }
    }
    return ret;
}

// this DEBUG function evaluates the objective function for a given Q, with no optimizations ( the long way )
double just_evaluate(int8_t *Q, int maxNodes, double **val) {
    int i, j;
    double result = 0.0;

    for (i = 0; i < maxNodes; i++) {
        for (j = i; j < maxNodes; j++) {
            result += val[i][j] * (double)Q[j] * (double)Q[i];
        }
    }
    return result;
}

// this DEBUG function evaluates the validity of the Qval difference vector Qval, and it will abort if there is a
// problem
// Qval = the change if a Q bit is flipped
void check_row_col_qval(int8_t *Q, int maxNodes, double **val, double *Qval, double *Row, double *Col) {
    int i, j;
    double Cols, Rows, Qvals;

    for (i = 0; i < maxNodes; i++) {
        Rows = 0.0;
        Cols = 0.0;

        for (j = i + 1; j < maxNodes; j++) {
            Rows += val[i][j] * (double)Q[j];
        }
        for (j = 0; j < i; j++) {
            Cols += val[j][i] * (double)Q[j];
        }

        if (Q[i] == 1) {
            Qvals = -(Rows + Cols + val[i][i]);
        } else {
            Qvals = (Rows + Cols + val[i][i]);
        }
        if ((Rows != Row[i]) | (Cols != Col[i]) | (Qvals != Qval[i])) {
            printf(" i = %d  Q[i] = %d  Row to Row[i] %.10e  %.10e  Col to Col[i] %.10e  %.10e  Qval to Qval[i] %.10e  "
                   "%.10e \n",
                   i, Q[i], Rows, Row[i], Cols, Col[i], Qvals, Qval[i]);
        }
    }
    return;
}

// this DEBUG function checks the Qval vector
void check_Qval(int8_t *Q, int maxNodes, double **val, double *Qval) {
    //  checking the Qval vector
    double result, just_result;
    int i;

    check_corrupt_Q(Q, maxNodes);
    result = just_evaluate(Q, maxNodes, val);
    check_corrupt_Q(Q, maxNodes);
    int fail = false;
    for (i = 0; i < maxNodes; i++) {
        Q[i] = 1 - Q[i];
        just_result = just_evaluate(Q, maxNodes, val);
        check_corrupt_Q(Q, maxNodes);
        if (just_result != result + Qval[i]) {
            DL;
            printf("Qval failure i=%d, just_result=%.10e, Qval + result = %.10e,Qval[i]= %.10e ,Q=%d\n", i, just_result,
                   result + Qval[i], Qval[i], Q[i]);
            fail = true;
        }
        Q[i] = 1 - Q[i];
    }
    if (fail) {
        print_solution_and_qubo(Q, maxNodes, val);
        exit(9);
    }
}
