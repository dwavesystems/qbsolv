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

//
// Here are some shims to replace stdint and stdbool for old compilers where
// they may not exist.
//

#if defined(_MSC_VER) && !defined(_MSC_STDINT_H_) && _MSC_VER < 1700
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

static void __test_block() {
    BUILD_BUG_ON(sizeof(int8_t) != 1);
    BUILD_BUG_ON(sizeof(int32_t) != 4);
    BUILD_BUG_ON(sizeof(int64_t) != 8);
}
#else
#include <stdint.h>
#endif

// Don't need to include/find bool in c++ context.
#ifndef __cplusplus
#if defined(_MSC_VER) && !defined(_MSC_STDBOOL_H_)
typedef int bool;
#define false 0
#define true 1
#else
// For c99 compilers _Bool is actually a built in type (macroed to bool).
#include <stdbool.h>
#endif
#endif
