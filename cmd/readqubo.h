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

#include <stdio.h>
#include "stdheaders_shim.h"

#ifdef __cplusplus
extern "C" {
#endif

struct nodeStr_ {
    int32_t n1, n2;
    double value;
};

//  zero out and fill 2d arrary val from nodes and couplers (negate if looking for minimum)
void fill_qubo(double **qubo, int maxNodes, struct nodeStr_ *nodes, int nNodes, struct nodeStr_ *couplers,
               int nCouplers);

int read_qubo(const char *inFileName, FILE *inFile);

#ifdef __cplusplus
}
#endif
