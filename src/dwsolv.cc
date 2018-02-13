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
// this is conditionally compiled
#ifdef QOP
#include "epqmi.h"
static DW_epqmi *epqmi_;
#endif
#include <locale.h>
#include <stdio.h>
#include "dwsolv.h"
#include "extern.h"  // qubo header file: global variable declarations
#include "macros.h"

#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

char *workspace, *connection, *wspath, *solver, ws_tmp_path[256], tmp_path[256];

// this is conditionally compiled as it needs externals supplied by Dwave tools group

int DW_setup_error = false;
int sysResult;
int start_;
int my_pid_;
char DWcommand[256];
FILE *f, *fr, *fs;
int i, j, k, l;  // working integer set
//
// common declare complete, start function

// checks to see if a DW connection has been established, if yes, return true, else false;
bool dw_established() {
    connection = getenv("DW_INTERNAL__CONNECTION");
    if (connection == NULL) {
        return false;
    }
    return true;
}
// checks to see that all environment variables are in place, along with
// .epqmi file and path needed to embed problems to the solver
int dw_init() {
    char filename_max_full[256];
    char linebuf[256];

    my_pid_ = getpid();  // will use for /tmp filename base

    workspace = getenv("DW_INTERNAL__WORKSPACE");
    if (workspace == NULL) {
        printf(" dw workspace not set \n");
        DW_setup_error = true;
    }
    connection = getenv("DW_INTERNAL__CONNECTION");
    if (connection == NULL) {
        printf(" dw connection not set \n");
        DW_setup_error = true;
    }
    wspath = getenv("DW_INTERNAL__WSPATH");
    if (wspath == NULL) {
        printf(" dw wspath not set \n");
        DW_setup_error = true;
    } else {
        sprintf(filename_max_full, "%s/.max_full", workspace);

        if ((fs = fopen(filename_max_full, "r")) == NULL) {
            printf(" no file %s\n", filename_max_full);
            exit(9);
        }

        if (fscanf(fs, "%s", linebuf) == 0) {
            DL;
            printf("fscanf error");
            exit(9);
        }

        fclose(fs);
        sprintf(ws_tmp_path, "/%s", linebuf);
    }
    solver = getenv("DW_INTERNAL__SOLVER");
    if (solver == NULL) {
        printf(" dw solver not set \n");
        DW_setup_error = true;
    }
    if (DW_setup_error == true) {
        printf(" dw setup not complete, and -S 0 set \n");
        printf(" if your pre-embeddings are not set up correctly contact \n");
        printf(" your system adminstrator.\n");
        DL;
        printf(" Exiting\n");
        exit(9);
    }

    char filename_epqmi_max[256];
    sprintf(filename_epqmi_max, "%s/%s/.epqmi_max", workspace, ws_tmp_path);  // find the size of embedded file
    if ((fs = fopen(filename_epqmi_max, "r")) == NULL) {
        printf("No pre-embedding found, and hardware execution (-S 0) requested %s\n", filename_epqmi_max);
        exit(9);
    }

    int S_embed;
    if (fscanf(fs, "%d", &S_embed) == 0) {
        DL;
        printf("fscanf error");
        exit(9);
    }
    int subMatrix = S_embed;
    fclose(fs);
    // use putenv instead of setenv
    int len_put = strlen(ws_tmp_path) + 1 + strlen("DW_INTERNAL__WSPATH") + 1;
    char *put_str = (char *)malloc(len_put);
    sprintf(put_str, "%s=%s", "DW_INTERNAL__WSPATH", ws_tmp_path);

    // if ( (sysResult = setenv("DW_INTERNAL__WSPATH", ws_tmp_path, 1)) != 0 ) {
    if ((sysResult = putenv(put_str)) != 0) {
        printf(" result of call %d\n", sysResult);
        printf(" Error making putenv call to set DW path to pre embeddings %s \n", put_str);
        DL;
        printf(" putenv command failed \n");
        exit(9);
    }
    strcpy(tmp_path, "/tmp");
    if(put_str != NULL) {
      free(put_str);
      put_str = NULL;
    }
    

    wspath = getenv("DW_INTERNAL__WSPATH");  // read again, as it was set

    if ((subMatrix < 10) | (subMatrix > 100)) {
        DL;
        printf(" Retrieved an incorrect embedding size, %d  \n", subMatrix);
        printf(" Exiting\n");
        exit(9);
    }
// this is conditionally compiled as it needs externals supplied by Dwave tools group
#if QOP
    epqmi_ = DW_epqmi_read(NULL);  // this call loads the pre-embedded qmi "epqmi"
    if (epqmi_ == NULL) {
        DL;
        printf(" return from DW_epqmi_read was NULL\n");
        exit(9);
    }
#endif

    if (Verbose_ > 2) {
        DLT;
        DL;
        // if ( UseDwave_ ) {
        //     printf(" UseDwave = true\n");
        // } else{
        //     printf(" UseDwave = false\n");
        // }
        printf(" SubMatrix_ = %d\n", subMatrix);
        printf(" %s %s \n", "DW_INTERNAL__WORKSPACE", workspace);
        printf(" %s %s \n", "DW_INTERNAL__CONNECTION", connection);
        printf(" %s %s \n", "DW_INTERNAL__WSPATH", wspath);
        printf(" %s %s \n", "DW_INTERNAL__SOLVER", solver);
    }

    return subMatrix;
}

void dw_solver(double **val, int maxNodes, int8_t *Q) {
#if LOCAL
    DL;
    printf(" The program was called when it wasn't compiled with qOp libraries. \n"
           " The request to call either the Dwave or the simulator cannot happen\n");
    val[1] = val[2];
    Q[1] = Q[maxNodes];  /// these statements will do nothing but kill off warnings from compiler
    exit(9);
    return;
//
#endif
#if QOP

    float param_values[maxNodes * maxNodes];

    // dwave finds minimum, while our tabu finds max, so send negative to dwave
    int k = 0;
    for (int i = 0; i < maxNodes; i++) {
        param_values[k++] = -val[i][i];
    }  // move the QUBO into param_values
    for (int i = 0; i < maxNodes; i++) {
        for (int j = i + 1; j < maxNodes; j++) {
            param_values[k++] = -val[i][j];
        }
    }
    param_values[k] = 15.0;

    // bind to .epqmi
    DW_epqmi_bind(epqmi_, param_values);

    if (DW_epqmi_exec(epqmi_, 25)) {
        DL;
        printf(" error execution of DW_epqmi_bind \n");
        exit(9);
    }

    int solutions = 0;
    if (DW_epqmi_sols(epqmi_, &solutions)) {
        DL;
        printf(" error execution of DW_epqmi_sols \n");
        exit(9);
    }
    char var[maxNodes];
    int valid = 1;
    int sol_num = 0;

    for (sol_num = 0; sol_num < solutions; sol_num++) {
        if (DW_epqmi_sol_vars(epqmi_, sol_num, var, &valid)) {
            DL;
            printf(" DW error\n");
            exit(9);
        }
        if (valid) break;
    }
    if (!valid) {
        DW_epqmi_sol_vars(epqmi_, 0, var, &valid);
        if (Verbose_ > 3) {
            DL;
            printf(" DW invalid, no valid solutions\n");
            printf("\nBits set after Dwave solver   ");
            for (i = 0; i < maxNodes; i++) printf("%d", Q[i]);
            printf("\n");
        }
    }

    for (int i = 0; i < maxNodes; i++) {
        Q[i] = var[i];
    }

#endif

    return;
}

void dw_close() { return; }

#ifdef __cplusplus
}
#endif
