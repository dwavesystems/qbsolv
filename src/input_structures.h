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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nodeStr_ {
    int32_t n1, n2;
    double value;
};

// The pointer type for subsolver.
// Its arguments are:
// - a 2d double array that is the sub-problem,
// - the size of the sub problem
// - a state vector: on input is the current best state, and should be set to the output state
typedef void (*SubSolver)(double**, int, int8_t*);

typedef struct parameters_t {
    int32_t repeats;
    SubSolver sub_sampler;
    int32_t sub_size;
} parameters_t;
parameters_t default_parameters();

#ifdef __cplusplus
}
#endif
