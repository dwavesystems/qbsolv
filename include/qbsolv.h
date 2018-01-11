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

#include "stdheaders_shim.h"

#ifdef __cplusplus
extern "C" {
#endif

// The pointer type for sub-solver.
// Its arguments are:
// - a 2d double array that is the sub-problem,
// - the size of the sub problem
// - a state vector: on input is the current best state, and should be set to the output state
typedef void (*SubSolver)(double**, int, int8_t*, void*);

// A parameter structure used to pass in optional arguments to the qbsolv: solve method.
typedef struct parameters_t {
    // The number of iterations without improvement before giving up
    int32_t repeats;
    // Callback function to solve the sub-qubo
    SubSolver sub_sampler;
    // The maximum size of problem that sub_sampler is willing to accept
    int32_t sub_size;
    // Extra parameter data passed to sub_sampler for callback specific data.
    void* sub_sampler_data;
} parameters_t;

// Get the default values for the optional parameters structure
parameters_t default_parameters(void);

// Callback for `solve` to use one of the `dw` calling methods
void dw_sub_sample(double** sub_qubo, int subMatrix, int8_t* sub_solution, void*);

// Callback for `solve` to use tabu on subproblems
void tabu_sub_sample(double** sub_qubo, int subMatrix, int8_t* sub_solution, void*);

// Entry into the overall solver from the main program
void solve(double** qubo, const int qubo_size, int8_t** solution_list, double* energy_list, int* solution_counts,
           int* Qindex, int QLEN, parameters_t* param);

#ifdef __cplusplus
}
#endif
