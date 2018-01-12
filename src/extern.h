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
// -------  GLOBAL Variables  ------------------------------------------
#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nodeStr_;

extern FILE *outFile_;
extern FILE *solution_input_;
extern int maxNodes_, nCouplers_, nNodes_, findMax_, start_, numsolOut_;
extern int Verbose_, TargetSet_, WriteMatrix_, Tlist_;
extern char *outFileNm_, pgmName_[16], algo_[4];
extern double Target_, Time_;

extern struct nodeStr_ *nodes_;
extern struct nodeStr_ *couplers_;

#ifdef __cplusplus
}
#endif
