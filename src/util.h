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
#pragma once

#include "macros.h"
#include "stdheaders_shim.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declare parameter type
typedef struct parameters_t parameters_t;

enum                                // of codes for sol_rslt.code
{ NOTHING = 0,                      // nothing new, do nothing
  NEW_HIGH_ENERGY_UNIQUE_SOL = 1,   // solution is unique, highest new energy
  DUPLICATE_HIGHEST_ENERGY = 2,     // two cases, solution is unique, duplicate energy
  DUPLICATE_ENERGY = 3,             // two cases, solution is duplicate highest energy
  DUPLICATE_ENERGY_UNIQUE_SOL = 4,  // two cases, solution is unique, highest energy
  NEW_ENERGY_UNIQUE_SOL = 5         // solution is unique, new highest energy
};

struct sol_man_rslt {
    int code;
    int count;
    int pos;
};

// create and pointer fill a 2d array of "size"
void **malloc2D(uint rows, uint cols, uint size);

// this randomly sets the bit vector to 1 or 0
void randomize_solution(int8_t *solution, int nbits);

// this randomly flips the bit vector to 1 or 0
void flip_solution(int8_t *solution, int nbits);

// this rotates bit vector to 1 or 0
void rotate_solution(int8_t *solution, int nbits);

// this randomly sets the bit vector to 1 or 0, with index
void randomize_solution_by_index(int8_t *solution, int nbits, int *indices);

// this flips the bit vector to 1 or 0, with index
void flip_solution_by_index(int8_t *solution, int nbits, int *indices);

// this randomly sets the bit vector to 1 or 0, with similar population counts
void randomize_pop_solution(int8_t *solution, int nbits);

// this randomly sets the bit vector to 1 or 0, with similar population counts with index
void randomize_pop_solution_by_index(int8_t *solution, int nbits, int *indices);

// shuffle the index vector before sort
void shuffle_index(int *indices, int length);

// shuffle the solution vector 
void shuffle_solution(int8_t *solution, int length);

//  print out the bit vector as row and column, surrounding the Qubo in triangular form  used in the -w option
void print_solution_and_qubo(int8_t *solution, int maxNodes, double **qubo);

//  This routine prints without \n the options for the run
void print_opts(int maxNodes, parameters_t *param);

//  This routine performs the standard output for qbsolv
void print_output(int maxNodes, int8_t *solution, long numPartCalls, double energy, double seconds,
                  parameters_t *param);

/* val[] --> Array to be sorted,
   arr[] --> index to point to order from largest to smallest
   n     --> number of elements in arrays */
void quick_sort_iterative_index(double val[], int arr[], int n, int *stack);

// routine to check the sort on index'ed sort
int is_index_sorted(double data[], int index[], int size);

//  find the position within the sorted(index) array for a value
int val_index_pos(int *index, double *val, int n, double compare);

//  fill an ordered by size index array based on sizes of val
void val_index_sort(int *index, double *val, int n);

void val_index_sort_ns(int *index, double *val, int n);

// sort an index array
void index_sort(int *index, int n, short forward);

// compares two vectors, bit by bit
bool is_array_equal(int8_t *solution_a, int8_t *solution_b, int nbits);

//  compare, bit by bit solution_A with solution_B and save the index of the value where
//  they differ in index[].     Return the number of values in the index vector
int index_solution_diff(int8_t *solution_A, int8_t *solution_B, int nbits, int *index);

//  count, bit by bit between solutions and return the solution of the value where
//  they differ in index[] ( any one of the solutions not same as any other ).
void solution_population(int8_t *popularSol, int8_t **solution, int num_solutions, int nbits, int *sol_index, int bias);

//  compare, bit by bit between solutions and save the index of the value where
//  they differ in index[] ( any one of the solutions not same as any other ).
int mul_index_solution_diff(int8_t **solution, int num_solutions, int nbits, int *index, int delta_bits,
                            int *sol_index);

//  print out each solution in index order per qbsolv output format
void print_solutions(int8_t **solution, double *energy_list, int *solutions_counts, int num_solutions, int nbits,
                     int *index);

struct sol_man_rslt manage_solutions(int8_t *solution_now, int8_t **solution_list, double energy_now,
                                     double *energy_list, int *solution_counts, int *list_order, int nMax, int nbits,
                                     int *num_nq_solutions);

// write qubo file to *filename
void write_qubo(double **qubo, int nMax, const char *filename);

#if _WIN32
size_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#ifdef __cplusplus
}
#endif
