/*
 Copyright 2016 D-Wave Systems Inc.

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

// ---------misc stuff used by habit
#ifndef QBSOLV_INCLUDE_H
#define QBSOLV_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#define  VERSION    "open source 2.3"

#define TRUE   1
#define FALSE  0
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define EPSILON 1.0e-7
#define BIGNEGFP -1.225E308
#define flt_equals(a, b) (fabs((a) - (b)) < EPSILON)

#define GETMEM(P, T, N) (P = (T*)malloc(sizeof(T) * N))
#define BADMALLOC printf("\n  ------> Exit(%d) called by %s(%s.%d)\n\n", 9, __func__, __FILE__, __LINE__); exit(9);
#define DL printf("-----> AT %s(%s.%d)\n",  __func__, __FILE__, __LINE__);
#define DLT printf("%lf seconds ", (double)(clock() - start_) / CLOCKS_PER_SEC);

// ----------------------- STRUCTs -------------------------------------
struct nodeStr_ {
	short n1, n2;
	double value;
};

// ------------------- Prototypes --------------------------------------

#ifdef __cplusplus
extern "C" {
#endif


int   main( int argc,  char *argv[]);
int   read_qubo(const char * inFileName);
void  write_qubo(double **val, int nMax, const char *filename);
void  solve( double **val,  int maxNodes);
void  **malloc2D(int rows, int cols, int size  );
void  fill_val(double **val, int maxNodes, struct nodeStr_ *nodes, int nNodes, struct nodeStr_ *couplers, int nCouplers);
void  print_qubo_format( void);
void  print_help( void);
int   DoesFileExist( const char *filename);
void  print_V_Q_Qval(short *Q, int maxNodes, double **val);
int   check_corrupt_Q(short *Q, int N);
int   check_corrupt_tabu(int *Q, int N, int nTabu);
void  set_bit(short *Q, int nbits);
void  shuffle_index(int *index, int n);
void  shuffle_indexR(int *index, int n);
double just_evaluate(short *Q, int maxNodes, double **val);
void  check_row_col_qval(short *Q, int maxNodes, double **val, double *Qval, double *Row, double *Col);
void  check_Qval(short *Q, int maxNodes, double **val, double *Qval, double *Row, double *Col);
void  print_output(int maxNodes, short *Q, long numPartCalls, double energy, double seconds);
void  quick_sort_iterative_index(double arr[], int ind[], int n);
void  val_index_sort(int *index, double *val, int n);
void  val_index_sort_ns(int *index, double *val, int n);
void  index_sort(int *index, int n, short FWD);
int manage_Q( short *Qnow, short **Qlist, double Vnow, double *QVs, int *Qcounts, int *Qindex, int nMax, int nbits);
int is_Q_equal( short *Qnow, short *Qcompare, int nbits);
void dw_init( );
void dw_solver( double **val, int maxNodes, short *Q );
void dw_close();
void reduce(int *Icompress, double **val, int subMatrix, int maxNodes, double **val_s, short *Q, short *Q_s);

#ifdef __cplusplus
}
#endif
#endif
