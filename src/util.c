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
#include "include.h"
#include "extern.h"

// create and pointer fill a 2d array of "size" for
// X[rows][cols] addressing. Using only a single malloc
void **malloc2D(uint rows, uint cols, uint size)
{
    // the total amount of memory required to hold both the matrix and the lookup table
    uintptr_t space = rows * (sizeof(char *) + (cols * size));
    char ** big_array = (char**)malloc(space);
    if (big_array == NULL) {
        DL;
        printf("\n\t%s error - memory request for X[%d][%d], %ld Mbytes  "
               "denied\n\n", pgmName_, rows, cols, (long)space / 1024 / 1024 );
        exit(9);
    }

    // Build a table of pointers in the first rows * sizeof(pointer)
    // bytes of the memory, that point to the rows in the data matrix
    space = cols * size;                     // space is now equal to the size of a row
    char * ptr   = (char*)&big_array[rows];  // ie, &big_array[nRows]
    for (uint i = 0; i < rows; ++i) {        // assign pointer to row, then increment pointer by size of row
        big_array[i] = ptr;
        ptr         += space;
    }
    return (void**)big_array;
}

// this randomly sets the bit vector to 1 or 0
void randomize_solution(int8_t *solution, int nbits)
{
    for (int i = 0; i < nbits; i++) {
        solution[i] = rand() % 2;
    }
}

// this randomly sets the bit vector to 1 or 0, with index
void randomize_solution_by_index(int8_t *solution, int nbits, int *indices)
{
    for (int i = 0; i < nbits; i++) {
        solution[indices[i]] = rand() % 2;
    }
}

// shuffle the index vector before sort
void shuffle_index(int *indices, int length)
{
    for (int i = length - 1; i > 0; i--) {
        int j   = rand() % (i + 1);
        // swap values
        int tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }
}

//  print out the bit vector as row and column, surrounding the Qubo in triangular form  used in the -w option
void print_solution_and_qubo(int8_t *solution, int maxNodes, double **qubo)
{
    double sign = findMax_ ? 1.0 : -1.0;

    fprintf(outFile_, "ij, ");
    for (int i = 0; i < maxNodes; i++) fprintf(outFile_, ",%d", i);
    fprintf(outFile_, "\n");

    fprintf(outFile_, "Q,");
    for (int i = 0; i < maxNodes; i++) fprintf(outFile_, ",%d", solution[i]);
    fprintf(outFile_, "\n");

    for (int i = 0; i < maxNodes; i++) {
        fprintf(outFile_, "%d,%d,", i, solution[i]);
        for (int j = 0; j < i; j++) fprintf(outFile_, ",");
        for (int j = i; j < maxNodes; j++) {
            if (qubo[i][j] != 0.0 ) {
                fprintf(outFile_, "%6.4lf,", (qubo[i][j] * sign));
            }else {
                fprintf(outFile_, ",");
            }
        }
        fprintf(outFile_, "\n");
    }

    /*  print out the bit vector as row and column, surrounding the
     *  Qubo where both the row and col bit is set in triangular form */
    fprintf(outFile_, "  Values that have a Q of 1 ");

    fprintf(outFile_, "ij, ");
    for (int i = 0; i < maxNodes; i++) fprintf(outFile_, ",%d", i);
    fprintf(outFile_, "\n");

    fprintf(outFile_, "Q,");
    for (int i = 0; i < maxNodes; i++) fprintf(outFile_, ",%d", solution[i]);
    fprintf(outFile_, "\n");

    for (int i = 0; i < maxNodes; i++) {
        fprintf(outFile_, "%d,%d,", i, solution[i]);
        for (int j = 0; j < i; j++) fprintf(outFile_, ",");
        for (int j = i; j < maxNodes; j++) {
            if (((double)solution[i] * solution[j]) * qubo[i][j] != 0 ) {
                fprintf(outFile_, "%6.4lf,", qubo[i][j] * sign * solution[i] * solution[j]);
            }else {
                fprintf(outFile_, ",");
            }
        }
        fprintf(outFile_, "\n");
    }
}

//  This routine performs the standard output for qbsolv
//
void print_output(int maxNodes, int8_t *solution, long numPartCalls, double energy, double seconds)
{
    int i;

    fprintf(outFile_, "%d Number of bits in solution\n", maxNodes);
    for (i = 0; i < maxNodes; i++) {
        fprintf(outFile_, "%d", solution[i]);
    }
    fprintf(outFile_, "\n");
    fprintf(outFile_, "%8.5f Energy of solution\n", energy);
    fprintf(outFile_, "%ld Number of Partitioned calls\n", numPartCalls);
    fprintf(outFile_, "%8.5f seconds of classic cpu time", seconds);
    if ( TargetSet_ ) {
        fprintf(outFile_, " ,Target of %8.5f\n", Target_);
    } else {
        fprintf(outFile_, "\n");
    }
}

//  zero out and fill 2d arrary val from nodes and couplers (negate if looking for minimum)
//
void fill_qubo(double **qubo, int maxNodes, struct nodeStr_ *nodes,
	int nNodes, struct nodeStr_ *couplers, int nCouplers)
{
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

int partition(double val[], int arr[], int l, int h)
{
    int    i, j, t;
    double x;

    x = val[arr[h]];
    i = (l - 1);

    for (j = l; j <= h - 1; j++) {
        if (val[arr[j]] >= x) {
            i++;
            t = arr[i]; arr[i] = arr[j]; arr[j] = t; // swap
        }
    }
    t = arr[i + 1]; arr[i + 1] = arr[h]; arr[h] = t; // swap
    return (i + 1);
}

/* val[] --> Array to be sorted,
   arr[] --> index to point to order from largest to smallest
   n     --> number of elements in arrays */
void quick_sort_iterative_index(double val[], int arr[], int n, int *stack)
{
    int h, l;

    h = n - 1; // last index
    l = 0; // first index

    // initialize top of stack
    int top = -1;

    // push initial values of l and h to stack
    stack[ ++top ] = l;
    stack[ ++top ] = h;

    // Keep popping from stack while is not empty
    while ( top >= 0 ) {
        // Pop h and l
        h = stack[ top-- ];
        l = stack[ top-- ];

        // Set pivot element at its correct position
        // in sorted array
        int p = partition(val, arr, l, h );

        // If there are elements on left side of pivot,
        // then push left side to stack
        if ( p - 1 > l ) {
            stack[ ++top ] = l;
            stack[ ++top ] = p - 1;
        }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if ( p + 1 < h ) {
            stack[ ++top ] = p + 1;
            stack[ ++top ] = h;
        }
    }
}

// routine to check the sort on index'ed sort
//
int is_index_sorted(double data[], int index[], int size)
{
    int i;

    for (i = 0; i < (size - 1); i++) {
        if (data[index[i]] < data[index[i + 1]]) {
            return false;
        }
    }
    return true;
}

//
//  fill an ordered by size index array based on sizes of val
//  a quick sort is used so that I can get speed and do a value
//  index sort  ( measured as 2x faster than a qsort with
//  tricks to do a index sort)
//
void val_index_sort(int *index, double *val, int n)
{
    int i;
    int *stack; // temp space = n + 1
    // Create an auxiliary stack
    if ((GETMEM(stack, int, n + 1)) == NULL) {
        BADMALLOC
    }

    for (i = 0; i < n; i++) index[i] = i;
    shuffle_index(index, n);
    quick_sort_iterative_index(val, index, n, stack);
    free(stack);
    // check code:
    // for (i=0;i<n-1;i++) { if (val[index[i]]<val[index[i+1]]) { DL; exit(9); } }
    return;
}

void val_index_sort_ns(int *index, double *val, int n)
{
    int i;
    int *stack; // temp space = n + 1
    // Create an auxiliary stack
    if ((GETMEM(stack, int, n + 1)) == NULL) {
        BADMALLOC
    }

    // Assure that the index array covers val[] completely
    for (i = 0; i < n; i++) index[i] = i;
    quick_sort_iterative_index(val, index, n, stack);
    free(stack);
    // check code:
    // for (i=0;i<n-1;i++) { if (val[index[i]]<val[index[i+1]]) { DL; exit(9); } }
    return;
}

int compare_intsAsc( const void *p, const void *q)
{
    int x = *(const int*)p;
    int y = *(const int*)q;

    if (x < y) return -1; // return -1 if you want ascending, 1 if you want descending order.
    else if (x > y) return 1; // return 1 if you want ascending, -1 if you want descending order.
    return 0;
}

int compare_intsDes( const void *p, const void *q)
{
    int x = *(const int*)p;
    int y = *(const int*)q;

    if (x < y) return 1;       // return -1 if you want ascending, 1 if you want descending order.
    else if (x > y) return -1; // return 1 if you want ascending, -1 if you want descending order.
    return 0;
}

//
// sort an index array
//
void index_sort(int *index, int n, short forward)
{
    if (forward) {
        qsort(index, n, sizeof *index, &compare_intsAsc);
    } else {
        qsort(index, n, sizeof *index, &compare_intsDes);
    }
}

// compares two vectors, bit by bit
bool is_array_equal( int8_t *solution_a, int8_t *solution_b, int nbits)
{
    for (int i = 0; i < nbits; i++) {
        if (solution_a[i] != solution_b[i]) {
            return false;
        }
    }
    return true;
}

// solution_now is the Q vector being looked at
// solution_list is the 2d array of Q vectors being stored
// energy_now is the energy of the Q vector being looked at
// energy_list is the 1d array of energies corresponding to solution_lists
// solution_counts is the 1d array of hits on the corresponding solution_lists
// list_order is the order of solution_list based upon energies
// nMax is size of the arrays (solution_list, energy_list...)
// if solution_now is unique, and is better than or equal to the worst solution add it to solution_list
// if solution_now is not unique ( equal energy )  increment number of times found
int manage_solutions( int8_t *solution_now, int8_t **solution_list, double energy_now,
	double *energy_list, int *solution_counts, int *list_order, int nMax, int nbits)
{
    // return codes
    enum
    {
        NOTHING = 0,                     // nothing new, do nothing
        NEW_HIGH_ENERGY_UNIQUE_SOL = 10, // solution is unique, highest new energy
        DUPLICATE_HIGHEST_ENERGY = 20,   // two cases, solution is unique, duplicate energy
        //            solution is duplicate, highest energy, add the number of repeats to this value
        DUPLICATE_ENERGY = 30,           // two cases, solution is unique, highest energy
        //            solution is duplicate, not highest, add the number of repeats to this value
        NEW_ENERGY_UNIQUE_SOL = 40       // solution is unique, new highest energy
    };

    val_index_sort_ns(list_order, energy_list, nMax); // index array of sorted energies

    // new high value,
    if (energy_now > energy_list[list_order[0]] ) {
        // we will add it to the space to an empty queue, Sort will fix it later
        int empty_row = list_order[nMax - 1];

        // save the bits to Qlist first entry
        for (int i = 0; i < nbits; i++) {
            solution_list[empty_row][i] = solution_now[i];
        }

        energy_list[empty_row] = energy_now;
        solution_counts[empty_row] = 1;

        // index array of sorted energies
        val_index_sort_ns(list_order, energy_list, nMax);

        // we have added this Qnow to the collective, might have overwritten an old one
        return NEW_HIGH_ENERGY_UNIQUE_SOL;
    }

    // list energies are all higher than this, do nothing
    if (energy_now < energy_list[list_order[nMax - 1]] ) {
        return NOTHING;
    }
    
    // new energy is in the range of our list
    // search thru the list to see if there is an equal energy
    for (int i = 0; i < nMax; i++) {
        if (energy_now == energy_list[list_order[i]]) {

            int j = i; // now have a common energy, but it could be that we have a different Q

            // look thru all Q's of common energy (they are ordered)
            while (j < nMax && energy_list[list_order[j]] == energy_now) {
                if (is_array_equal(solution_list[list_order[j]], solution_now, nbits)) {
                    // simply mark this Q and energy as a duplicate find
                    solution_counts[list_order[j]]++;

                    if (energy_now == energy_list[list_order[0]]) {
                        // duplicate energy matching another Q and equal to best energy
                        return DUPLICATE_HIGHEST_ENERGY + solution_counts[list_order[j]];
                    } else {
                        // duplicate energy matching older lower energy Q
                        return DUPLICATE_HIGHEST_ENERGY;
                    }
                }
                j++;
            }
            
            // fallen thru equal energies so we need to add it to the list
            j = list_order[nMax - 1]; // add it to the worst energy position ( prefilled with worst possible value )
            energy_list[j] = energy_now; // save energy
            solution_counts[j] = 1; // set number of hits as this is a first

            // save the bits to solution_list
            for (int i = 0; i < nbits; i++) {
                solution_list[j][i] = solution_now[i];
            }

            // Create index array of sorted energies
            val_index_sort_ns(list_order, energy_list, nMax);
            if (energy_now == energy_list[list_order[0]]) {
                // duplicate highest energy unique Q and equal to best energy
                return DUPLICATE_ENERGY;
            } else {
                // duplicate energy matching older lower energy Q
                return DUPLICATE_ENERGY + solution_counts[j];
            }
        }

        // we have spilled off the list of energies and need to add this one
        else if (energy_now < energy_list[list_order[i]]) {
            int j = list_order[nMax - 1]; // add it to the worst energy position as it is unique but within the list
            energy_list[j] = energy_now; // save energy
            solution_counts[j] = 1; // set number of hits as this is a first

            // save the bits to solution_list
            for (int i = 0; i < nbits; i++) {
                solution_list[j][i] = solution_now[i];
            }

            // create index array of sorted energies
            val_index_sort_ns(list_order, energy_list, nMax);
            return NEW_ENERGY_UNIQUE_SOL;
        }
    }

    for (int iL = 0; iL < nMax; iL++)
        printf(" %d %d %lf %d \n", list_order[iL], iL,
               energy_list[list_order[iL]], solution_counts[list_order[iL]]);
    
    exit(9);
}

// write qubo file to *filename
void write_qubo(double **qubo, int nMax, const char *filename)
{
    // Try to open the file
    FILE *file;
    if ((file = fopen(filename, "w")) == 0) {
        DL; printf(" failed to open %s\n", filename); exit(9);
    }

    // count the non-zero couplers and nodes
    int nNodes = 0, nCouplers = 0;
    for (int i = 0; i < nMax; i++) {
        if (qubo[i][i] != 0.0) nNodes++;
        for (int j = i + 1; j < nMax; j++ ) {
            if (qubo[i][j] != 0.0) nCouplers++;
        }
    }

    // Write out the header line
    fprintf(file, "p qubo 0 %d %d %d\n", nMax, nNodes, nCouplers);

    // Write out the details for all non zero linear/quadratic elements
    for (int i = 0; i < nMax; i++) {
        if (qubo[i][i] != 0) fprintf(file, "%d %d %lf\n", i, i, qubo[i][i]);
    }
    for (int i = 0; i < nMax; i++) {
        for (int j = i + 1; j < nMax; j++) {
            if (qubo[i][j] != 0.0) fprintf(file, "%d %d %lf\n", i, j, qubo[i][j]);
        }
    }
    fclose(file);
}
