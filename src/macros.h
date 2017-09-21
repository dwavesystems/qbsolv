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
#pragma once
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
