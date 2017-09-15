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

// ---------misc stuff used by habit
#ifndef QBSOLV_INCLUDE_H
#define QBSOLV_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#define  VERSION    "open source 2.5"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define EPSILON 1.0e-7
#define BIGNEGFP -1.225E308
#define flt_equals(a, b) (fabs((a) - (b)) < EPSILON)

#define GETMEM(P, T, N) (P = (T*)malloc((sizeof(T) * N)))
#define BADMALLOC {printf("\n  ------> Exit(%d) called by %s(%s.%d)\n\n", 9, __func__, __FILE__, __LINE__); exit(9); }
#define DL printf("-----> AT %s(%s.%d)\n",  __func__, __FILE__, __LINE__);
#define CPSECONDS ((double)(clock() - start_) / CLOCKS_PER_SEC)
#define DLT printf("%lf seconds ", CPSECONDS);
#define uint unsigned int
#if _WIN32
#define LONGFORMAT "lld"
#elif defined (__unix__) || defined (__HAIKU__)
#define LONGFORMAT "ld"
#elif defined (__APPLE__)
#define LONGFORMAT "lld"
#endif


// ----------------------- STRUCTs -------------------------------------

struct nodeStr_ {
    int32_t n1, n2;
    double value;
};

struct sol_man_rslt {
    int code;
    int count;
    int pos;
};

// typedef struct sub_parameters_t {
//     // int8_t * current_best;
// } sub_parameters_t;

typedef void (*SubSampler)(double**, int, int8_t*);
typedef struct parameters_t {
    int32_t repeats;
    SubSampler sub_sampler;
    int32_t sub_size;
} parameters_t;
parameters_t default_parameters();

// ------------------- Prototypes --------------------------------------
enum  // of codes for sol_rslt.code
{
    NOTHING = 0,                    // nothing new, do nothing
    NEW_HIGH_ENERGY_UNIQUE_SOL = 1, // solution is unique, highest new energy
    DUPLICATE_HIGHEST_ENERGY = 2,   // two cases, solution is unique, duplicate energy
    DUPLICATE_ENERGY = 3,           // two cases, solution is duplicate highest energy
    DUPLICATE_ENERGY_UNIQUE_SOL = 4,// two cases, solution is unique, highest energy
    NEW_ENERGY_UNIQUE_SOL = 5       // solution is unique, new highest energy
};

#ifdef __cplusplus
extern "C" {
#endif

int   read_qubo(const char *inFileName, FILE *inFile);
void  write_qubo(double **val, int nMax, const char *filename);

void  solve( double **val,  int maxNodes, int8_t **solution_list, double *energy_list, int *solution_counts, int *Qindex, int QLEN, parameters_t);
void  dw_sub_sample(double**, int, int8_t*);
void  tabu_sub_sample(double**, int, int8_t*);

void  **malloc2D(uint rows, uint cols, uint size  );
void  fill_qubo(double **qubo, int maxNodes, struct nodeStr_ *nodes, int nNodes, struct nodeStr_ *couplers, int nCouplers);
void  print_qubo_format( void);
void  print_help( void);
int   DoesFileExist( const char *filename);
void  print_solution_and_qubo(int8_t *solution, int maxNodes, double **qubo);
int   check_corrupt_Q(int8_t *Q, int N);
int   check_corrupt_tabu(int *Q, int N, int nTabu);
void  randomize_solution(int8_t *solution, int nbits);
void  randomize_solution_by_index(int8_t *solution, int nbits, int *index);
void  randomize_pop_solution(int8_t *solution, int nbits);
void  randomize_pop_solution_by_index(int8_t *solution, int nbits, int *index);
void  shuffle_index(int *index, int n);
void  shuffle_indexR(int *index, int n);
double just_evaluate(int8_t *Q, int maxNodes, double **val);
double roundit(double x, int N);
void  check_row_col_qval(int8_t *Q, int maxNodes, double **val, double *Qval, double *Row, double *Col);
void  check_Qval(int8_t *Q, int maxNodes, double **val, double *Qval);
// void  print_opts(parameters_t*);
void  print_output(int maxNodes, int8_t *Q, long numPartCalls, double energy, double seconds, parameters_t*);
void  quick_sort_iterative_index(double arr[], int ind[], int n, int stack[]);
void  val_index_sort(int *index, double *val, int n);
void  val_index_sort_ns(int *index, double *val, int n);
void  index_sort(int *index, int n, short FWD);
int index_solution_diff( int8_t *solution_A, int8_t *solution_B, int nbits , int *index ) ;
void solution_population( int8_t *popularSol, int8_t **solution, int num_solutions, int nbits , int *sol_index, int bias );
int mul_index_solution_diff( int8_t **solution, int num_solutions, int nbits , int *index, int delta_bits, int *sol_index );
void print_solutions( int8_t **solution, double *energy_list,int *solutions_counts, int num_solutions, int nbits , int *index );
struct sol_man_rslt manage_solutions( int8_t *new_solution, int8_t **solution_list, double new_energy,
    double *QVs, int *solution_counts, int *Qindex, int nMax, int nbits, int *num_nq_solutions);
bool is_array_equal( int8_t *solution_now, int8_t *solution_other, int nbits);
bool dw_established();
void dw_init( );
void dw_solver( double **val, int maxNodes, int8_t *Q );
void dw_close();
void reduce(int *Icompress, double **qubo, uint sub_qubo_size, uint qubo_size, double **sub_qubo, int8_t *solution, int8_t *sub_solution);
#if _WIN32
size_t getline_win(char **lineptr, size_t *n, FILE *stream) ;
#endif
#ifdef __cplusplus
}
#endif
#endif
