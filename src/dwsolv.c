/*
 Copyright 2016 D-Wave Systems Inc.

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

/*
 * More information needed, please provide
 *
 * Why is this only used when SubMatrix_ == 0?
*/

#include "include.h"
#include "extern.h" // qubo header file: global variable declarations
#include <stdio.h>
#include <locale.h>

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

char       *workspace, *connection, *wspath, *solver, ws_tmp_path[256], tmp_path[256];
const char *DW_INTERNAL__WORKSPACE = "DW_INTERNAL__WORKSPACE";
const char *DW_INTERNAL__CONNECTION = "DW_INTERNAL__CONNECTION";
const char *DW_INTERNAL__WSPATH = "DW_INTERNAL__WSPATH";
const char *DW_INTERNAL__SOLVER = "DW_INTERNAL__SOLVER";

// TODO: this should be made into a const and done correctly
#define MINGWGINK C:/ MinGW / msys / 1.0
static char *mingw_base = "MINGWGINK"; // ridiculous MinGw workaround
int         mingw_base_len;

int  DW_setup_error = false;
int  sysResult;
int  start_;
int  my_pid_;
char DWcommand[256];
FILE *f, *fr, *fs;
int  i, j, k, l; // working integer set

void dw_init()
{
	// checks to see that all environment variables are in place, along with
	// .epqmi file and path needed to embed problems to the solver
	char filename_max_full[256];
	char linebuf[256];

	mingw_base_len = strlen("MINGWGINK"); // more Ridiculous MinGW workaround
	my_pid_ = getpid(); // will use for /tmp filename base

	workspace = getenv(DW_INTERNAL__WORKSPACE);
	if ( workspace == NULL ) {
		printf(" dw workspace not set up \n");
		DW_setup_error = true;
	}
	connection = getenv(DW_INTERNAL__CONNECTION);
	if ( connection == NULL ) {
		printf(" dw connection not set up \n");
		DW_setup_error = true;
	}
	wspath = getenv(DW_INTERNAL__WSPATH);
	if ( wspath == NULL ) {
		printf(" dw wspath not set up \n");
		DW_setup_error = true;
	} else {

		if (strncmp(wspath, mingw_base, mingw_base_len) == 0)
			wspath += mingw_base_len; // completed RMWorkaround

		sprintf(filename_max_full, "%s/.max_full", workspace);

		if ((fs = fopen(filename_max_full, "r")) == NULL) {
			printf(" no file %s\n", filename_max_full);
			exit(9);
		}

		if ( fscanf(fs, "%s", linebuf) == 0 ) {
			DL; printf("fscanf error");
			exit(9);
		}

		fclose(fs);
		sprintf(ws_tmp_path, "/%s", linebuf);
	}
	solver = getenv(DW_INTERNAL__SOLVER);
	if ( solver == NULL ) {
		printf(" dw solver not set up \n");
		DW_setup_error = true;
	}
	if ( DW_setup_error == true ) {
		printf(" dw not set up not complete, and -S 0 set \n");
		DL; printf(" Exiting\n");
		exit(9);
	}

	char filename_epqmi_max[256];
	sprintf(filename_epqmi_max, "%s/%s/.epqmi_max", workspace, ws_tmp_path); // find the size of embeded file
	if ((fs = fopen(filename_epqmi_max, "r")) == NULL) {
		printf(" no file %s\n", filename_epqmi_max);
		exit(9);
	}

	int  S_embed;
	if ( fscanf(fs, "%d", &S_embed) == 0 ) {
		DL; printf("fscanf error");
		exit(9);
	}
	SubMatrix_ = S_embed;
	fclose(fs);

	if ( (sysResult = setenv(DW_INTERNAL__WSPATH, ws_tmp_path, 1)) != 0 ) {
		printf(" result of call %d\n", sysResult);
		printf(" Error making setenv call %s %s \n", "DW_INTERNAL_WSPATH", ws_tmp_path);
		DL; printf(" setenv command failed \n");
		exit(9);
	}
	strcpy(tmp_path, "/tmp");

	wspath = getenv(DW_INTERNAL__WSPATH);  // read again, as it was set

	if ( (SubMatrix_ < 10) | (SubMatrix_ > 100) ) {
		DL; printf(" Retrieved a incorrect embedding size, %d  \n", SubMatrix_);
		printf(" Exiting\n");
		exit(9);
	}

	if (Verbose_ > 2) {
		DLT; DL;
		if ( UseDwave_ ) {
			printf(" UseDwave = true\n");
		} else{
			printf(" UseDwave = false\n");
		}
		printf(" SubMatrix_ = %d\n", SubMatrix_);
		printf(" %s %s \n", DW_INTERNAL__WORKSPACE, workspace);
		printf(" %s %s \n", DW_INTERNAL__CONNECTION, connection);
		printf(" %s %s \n", DW_INTERNAL__WSPATH, wspath);
		printf(" %s %s \n", DW_INTERNAL__SOLVER, solver);
	}
}

void dw_solver(double **val, int maxNodes, int8_t *Q)
{
	// bind to .epqmi
	char filename_b[256];
	char filename_result[256];

	sprintf(filename_b, "%s/qbs%d.b", tmp_path, my_pid_); // we will need to write b file to /tmp directory

	if ((f = fopen(filename_b, "w")) == NULL ) {
		DL; printf("  Failed open %s\n", filename_b);
		exit(9);
	}

	// dwave finds minimum, while our tabu finds max, so send negative to dwave
	for (i = 0; i < maxNodes; i++) {
		fprintf(f, "a_%d = %6.3f\n", i + 1, -val[i][i]);
	}
	for (i = 0; i < maxNodes; i++) {
		for (j = i + 1; j < maxNodes; j++) {
			fprintf(f, "b_%d_%d = %6.3f\n", i + 1, j + 1, -val[i][j]);
		}
	}
	fclose(f);

	// What does this do?
	sprintf(DWcommand, "bash -c \'dw bind param_chain=15.0 /tmp/qbs%d.b  > /dev/null;"
	        "dw exec num_reads=10 qbs%d.qmi  > /dev/null;"
	        "dw val -s 1 qbs%d.sol |tail -n%d|cut -f3 -d\" \" > /tmp/qbs%d.xB\' \n",
	        my_pid_, my_pid_, my_pid_, maxNodes, my_pid_);
	sysResult = system(DWcommand);
	if (sysResult != 0 ) {
		DL; printf("system call error %d\n %s\n", sysResult, DWcommand);
		exit(9);
	}
	if (Verbose_ > 2) printf("%s\n", DWcommand);
	sprintf(filename_result, "%s/qbs%d.xB", tmp_path, my_pid_);
	if ((fr = fopen(filename_result, "r")) == NULL ) {
		DL; printf("  Failed open %s\n", filename_result); exit(9);
	}
	int8_t qtmp;
	for (i = 0; i < maxNodes; i++) {
		if ((fscanf(fr, "%hhd", &qtmp)) == 0 ) {
			DL; printf("fscanf error %s", filename_result); exit(9);
		}
		Q[i] = qtmp;
	}
	if (Verbose_ > 3) {
		printf("\nBits set after Dwave solver   ");
		for (i = 0; i < maxNodes; i++) printf("%d", Q[i]);
		printf("\n");
	}
	fclose(fr);

	return;
}

void dw_close()
{

	sprintf(DWcommand, "rm -f %s/qbs%d* \n", tmp_path, my_pid_);

	if (Verbose_ > 2) printf("%s", DWcommand);

	if (system(DWcommand) != 0 ) {
		DL; printf("system call error,%s", DWcommand);
	} // clean up the /tmp directory

	sprintf(DWcommand, "rm -f %s/%s/qbs%d.* \n", workspace, ws_tmp_path, my_pid_);

	if (system(DWcommand) != 0 ) {
		DL; printf("system call error,%s", DWcommand);
	} // clean up the dw full.x directory

	if (Verbose_ > 2) printf("%s", DWcommand);

	return;
}
