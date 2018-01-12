/*
 Copyright 2017 D-Wave Systems Inc
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

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

// This function Simply evaluates the objective function for a given solution.
double Simple_evaluate(const int8_t *const solution, const uint qubo_size, const double **const qubo);

// This function evaluates the objective function for a given solution.
double evaluate(int8_t *const solution, const uint qubo_size, const double **const qubo, double *const flip_cost);

// Flips a given bit in the solution, and calculates the new energy.
double evaluate_1bit(const double old_energy, const uint bit, int8_t *const solution, const uint qubo_size,
                     const double **const qubo, double *const flip_cost);

// Tries to improve the current solution Q by flipping single bits.
double local_search_1bit(double energy, int8_t *solution, uint qubo_size, double **qubo, double *flip_cost,
                         int64_t *bit_flips);

// Performs a local Max search improving the solution and returning the last evaluated value
double local_search(int8_t *solution, int qubo_size, double **qubo, double *flip_cost, int64_t *bit_flips);

// This function is called by solve to execute a tabu search
double tabu_search(int8_t *solution, int8_t *best, uint qubo_size, double **qubo, double *flip_cost, int64_t *bit_flips,
                   int64_t iter_max, int *TabuK, double target, bool target_set, int *index, int nTabu);

// reduce() computes a subQUBO (val_s) from large QUBO (val)
void reduce(int *Icompress, double **qubo, uint sub_qubo_size, uint qubo_size, double **sub_qubo, int8_t *solution,
            int8_t *sub_solution);

// solv_submatrix() performs QUBO optimization on a subregion.
double solv_submatrix(int8_t *solution, int8_t *best, uint qubo_size, double **qubo, double *flip_cost,
                      int64_t *bit_flips, int *TabuK, int *index);

// reduce_solv_projection reduces from a submatrix solves the QUBO projects the solution and
//      returns the number of changes
int reduce_solve_projection(int *Icompress, double **qubo, int qubo_size, int subMatrix, int8_t *solution,
                            parameters_t *param);

#ifdef __cplusplus
}
#endif
