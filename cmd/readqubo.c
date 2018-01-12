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

#include <stdint.h>

#include "extern.h"
#include "macros.h"
#include "readqubo.h"

// read from inFile and parse the qubo file

int pFound = false;
int lineNm = 0;
const char s[2] = " ";  // TODO: please comment what this variable is for
int i, j, k;            // standard scratch ints
int inode = 0, icoupler = 0;
double f;

int read_qubo(const char *inFileName, FILE *inFile) {
    int lineLen;
    size_t linecap = 0;
    char *line = NULL;
    char token[50], tokenp[50];
#define MAX_TOPOLOGY_LEN 49  // more than big enough for "0", "unconstrained", or "chimeraXXX"
    char topology[MAX_TOPOLOGY_LEN + 1];

    memset(topology, '\0', MAX_TOPOLOGY_LEN + 1);

#if _WIN32
    while ((lineLen = getline_win(&line, &linecap, inFile)) > 0) {
#else
    while ((lineLen = getline(&line, &linecap, inFile)) > 0) {
#endif
        lineNm++;
        if (strncmp(line, "c", 1) == 0 || strncmp(line, "C", 1) == 0) {
            continue;  // comment line in file
        }
        if (!pFound) {
            if (strncmp(line, "p", 1) == 0 || strncmp(line, "P", 1) == 0) {  // found program line
                sscanf(line, " %s %s %s %d %d %d", tokenp, token, topology, &maxNodes_, &nNodes_, &nCouplers_);
                if (strncmp(token, "qubo", 4) != 0) {
                    fprintf(stderr, " P line in %s is not a qubo, it lists as %s\n", inFileName, token);
                    exit(9);
                } else if (0 != strncmp(topology, "0", 1) && 0 != strncmp(topology, "unconstrained", 13)) {
                    fprintf(stderr,
                            " P line in %s specifies unknown topology \"%s\".\n"
                            " Only \"0\" and \"unconstrained\" (which are equivalent) are supported currently\n",
                            inFileName, topology);
                    exit(9);
                } else {  // it is a p qubo line :-)
                    // The p line is a header in the qubo file format
                    // now we can allocate node and coupler memory
                    if (GETMEM(nodes_, struct nodeStr_, nNodes_) == NULL) {
                        BADMALLOC
                    }
                    if (GETMEM(couplers_, struct nodeStr_, nCouplers_) == NULL) {
                        BADMALLOC
                    }
                }
                pFound = true;
            } else {
                continue;  // not a comment line not a p line, so unknown, so skip it
            }

        } else {  // p has been found and parsed, not a comment line so must be an input line or blank
            if (sscanf(line, "%d %d %lf", &i, &j, &f) == 3) {
                if (i == j) {
                    if (inode > nNodes_) {
                        fprintf(stderr, " Number of nodes exceeded at line %d %s,\n nodes= %d\n", lineNm, line, inode);
                        exit(9);
                    }
                    nodes_[inode].n1 = i;
                    nodes_[inode].n2 = j;
                    nodes_[inode++].value = f;
                } else {
                    if (i > j) {
                        fprintf(stderr,
                                " couplers first value must be > second value; at line %d:  %s"
                                " coordinates  %d > %d \n",
                                lineNm, line, i, j);
                        exit(9);
                    }

                    if (icoupler > nCouplers_) {
                        fprintf(stderr, " Number of couplers exceeded at line %d %s,\n Couplers= %d\n", lineNm, line,
                                icoupler);
                        exit(9);
                    }
                    couplers_[icoupler].n1 = i;
                    couplers_[icoupler].n2 = j;
                    couplers_[icoupler++].value = f;
                }
                if ((i + 1 > maxNodes_) || (j + 1 > maxNodes_)) {
                    fprintf(stderr, " Coordinates out of bounds ( 0 to %d )  at line %d %s,\n %d %d\n", maxNodes_,
                            lineNm, line, i, j);
                    exit(9);
                }
            }
        }
    }

    int errors = 0;
    if (icoupler != nCouplers_) {
        fprintf(stderr, " Number of couplers too small: couplers = %d, Ncouplers =%d\n", icoupler, nCouplers_);
        errors++;
    }
    if (inode != nNodes_) {
        fprintf(stderr, " Number of nodes too small: nodes = %d, Nnodes =%d\n", inode, nNodes_);
        errors++;
    }
    if (errors > 0) {
        exit(9);
    }

    return errors;
}

//  zero out and fill 2d arrary val from nodes and couplers (negate if looking for minimum)
//
void fill_qubo(double **qubo, int maxNodes, struct nodeStr_ *nodes, int nNodes, struct nodeStr_ *couplers,
               int nCouplers) {
    // Zero out the qubo
    for (int i = 0; i < maxNodes; i++) {
        for (int j = 0; j < maxNodes; j++) {
            qubo[i][j] = 0.0;
        }
    }

    // Copy the structs into the matrix, flip the sign if we are looking for the
    // minimum value during the optimization.
    if (findMax_) {
        for (int i = 0; i < nNodes; i++) {
            qubo[nodes[i].n1][nodes[i].n1] = nodes[i].value;
        }
        for (int i = 0; i < nCouplers; i++) {
            qubo[couplers[i].n1][couplers[i].n2] = couplers[i].value;
        }
    } else {
        for (int i = 0; i < nNodes; i++) {
            qubo[nodes[i].n1][nodes[i].n1] = -nodes[i].value;
        }
        for (int i = 0; i < nCouplers; i++) {
            qubo[couplers[i].n1][couplers[i].n2] = -couplers[i].value;
        }
    }
}
