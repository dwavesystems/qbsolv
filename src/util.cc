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
#include "util.h"
#include <math.h>
#include "extern.h"
#include "qbsolv.h"

#ifdef __cplusplus
extern "C" {
#endif

// create and pointer fill a 2d array of "size" for
// X[rows][cols] addressing. Using only a single malloc
void **malloc2D(uint rows, uint cols, uint size) {
    // the total amount of memory required to hold both the matrix and the lookup table
    uintptr_t space = rows * (sizeof(char *) + (cols * size));
    char **big_array = (char **)malloc(space);
    if (big_array == NULL) {
        DL;
        printf("\n\t%s error - memory request for X[%d][%d], %ld Mbytes  "
               "denied\n\n",
               pgmName_, rows, cols, (long)space / 1024 / 1024);
        exit(9);
    }

    // Build a table of pointers in the first rows * sizeof(pointer)
    // bytes of the memory, that point to the rows in the data matrix
    space = cols * size;                   // space is now equal to the size of a row
    char *ptr = (char *)&big_array[rows];  // ie, &big_array[nRows]
    for (uint i = 0; i < rows; ++i) {      // assign pointer to row, then increment pointer by size of row
        big_array[i] = ptr;
        ptr += space;
    }
    return (void **)big_array;
}

// this randomly sets the bit vector to 1 or 0
void randomize_solution(int8_t *solution, int nbits) {
    for (int i = 0; i < nbits; i++) {
        solution[i] = rand() % 2;
    }
}

// this circular rotates of the bit vector 1,2,3 or 4 positions
void rotate_solution(int8_t *solution, int nbits) {
    int rotate=1+rand()%4;
    for (int i = 0; i < nbits-rotate; i++) {
        solution[i] = solution[i+rotate];
    }
    int j=0;
    for (int i = nbits-rotate; i < nbits; i++ ) {
        solution[i] = solution[j++] ;
    }
}
// this randomly flips the bit vector to 1 or 0, favoring turning 0s to 1s
void flip_solution(int8_t *solution, int nbits) {
    for (int i = 0; i < nbits; i++) {
        if ( solution [i] == 1 && rand() %2 == 1 ) {
            solution [ i] = 0;
        }else {
            solution [ i] = 1;
        }
    }
}
// this randomly sets the bit vector to 1 or 0, with index
void randomize_solution_by_index(int8_t *solution, int nbits, int *indices) {
    for (int i = 0; i < nbits; i++) {
        solution[indices[i]] = rand() % 2;
    }
}
// this flips the bit vector to 1 or 0, with index, favoring turning 0s to 1s
void flip_solution_by_index(int8_t *solution, int nbits, int *indices) {
    for (int i = 0; i < nbits; i++) {
        if ( solution [indices[i]] == 1 && rand() %2 == 1 ) {
            solution [ indices [i]] = 0;
        }else {
            solution [ indices [i]] = 1;
        }
    }
}
// this randomly sets the bit vector to 1 or 0, with similar population counts
void randomize_pop_solution(int8_t *solution, int nbits) {
    double pop_ratio;
    int pop = 0, pop_ran = 0;
    for (int i = 0; i < nbits; i++) {
        pop += solution[i];
    }
    pop_ratio = (double)((double)pop / (double)nbits);
    pop_ran = (int)((double)RAND_MAX * pop_ratio);

    for (int i = 0; i < nbits; i++) {
        solution[i] = (rand() < pop_ran) ? 1 : 0;
    }
}
// this randomly sets the bit vector to 1 or 0, with similar population counts with index
void randomize_pop_solution_by_index(int8_t *solution, int nbits, int *indices) {
    double pop_ratio;
    int pop = 0, pop_ran = 0;
    for (int i = 0; i < nbits; i++) {
        pop += solution[indices[i]];
    }
    pop_ratio = (double)((double)pop / (double)nbits);
    pop_ran = (int)((double)RAND_MAX * pop_ratio);

    for (int i = 0; i < nbits; i++) {
        solution[indices[i]] = (rand() < pop_ran) ? 1 : 0;
    }
}
// shuffle the index vector using Durstenfeld's version of the Fisher-Yates
// shuffle algorithm.  Take care to avoid bias
void shuffle_index(int *indices, int length) {
    for (int i = length - 1; i > 0; i--) {
        int max_usable_rand = (RAND_MAX / (i + 1)) * (i + 1) - 1;  // integer div
        int j = 0;
        do {
            j = rand();
        } while (j > max_usable_rand);
        j %= (i + 1);
        if (j != i) {
            // swap values
            int tmp = indices[i];
            indices[i] = indices[j];
            indices[j] = tmp;
        }
    }
}

//  print out the bit vector as row and column, surrounding the Qubo in triangular form  used in the -w option
void print_solution_and_qubo(int8_t *solution, int maxNodes, double **qubo) {
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
            if (qubo[i][j] != 0.0) {
                fprintf(outFile_, "%6.4lf,", (qubo[i][j] * sign));
            } else {
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
            if (((double)solution[i] * solution[j]) * qubo[i][j] != 0) {
                fprintf(outFile_, "%6.4lf,", qubo[i][j] * sign * solution[i] * solution[j]);
            } else {
                fprintf(outFile_, ",");
            }
        }
        fprintf(outFile_, "\n");
    }
}
//  This routine prints without \n the options for the run
//
void print_opts(int maxNodes, parameters_t *param) {
    fprintf(outFile_, "%d bits, ", maxNodes);
    // if ( UseDwave_ ) {
    //     fprintf(outFile_,"Quantum solver,");
    // }else {
    //     fprintf(outFile_,"Classical tabu solver,");
    // }
    if (findMax_) {
        fprintf(outFile_, " find Max,");
    } else {
        fprintf(outFile_, " find Min,");
    }
    fprintf(outFile_, " SubMatrix= %d,", param->sub_size);
    fprintf(outFile_, " -a %s,", algo_);
    if (TargetSet_) fprintf(outFile_, " Target of %8.5f,", Target_);
    fprintf(outFile_, " timeout=%9.1f sec\n", Time_);
}

//  This routine performs the standard output for qbsolv
//
void print_output(int maxNodes, int8_t *solution, long numPartCalls, double energy, double seconds,
                  parameters_t *param) {
    int i;

    if (numsolOut_ > 0) {
        print_opts(maxNodes, param);
    }
    numsolOut_++;
    for (i = 0; i < maxNodes; i++) {
        fprintf(outFile_, "%d", solution[i]);
    }
    fprintf(outFile_, "\n");
    fprintf(outFile_, "%8.5f Energy of solution\n", energy);
    fprintf(outFile_, "%ld Number of Partitioned calls, %d output sample \n", numPartCalls, numsolOut_);
    fprintf(outFile_, "%8.5f seconds of classic cpu time", seconds);
    if (TargetSet_) {
        fprintf(outFile_, " ,Target of %8.5f\n", Target_);
    } else {
        fprintf(outFile_, "\n");
    }
}

int partition(double val[], int arr[], int l, int h) {
    int i, j, t;
    double x;

    x = val[arr[h]];
    i = (l - 1);

    for (j = l; j <= h - 1; j++) {
        if (val[arr[j]] >= x) {
            i++;
            t = arr[i];
            arr[i] = arr[j];
            arr[j] = t;  // swap
        }
    }
    t = arr[i + 1];
    arr[i + 1] = arr[h];
    arr[h] = t;  // swap
    return (i + 1);
}

/* val[] --> Array to be sorted,
   arr[] --> index to point to order from largest to smallest
   n     --> number of elements in arrays */
void quick_sort_iterative_index(double val[], int arr[], int n, int *stack) {
    int h, l;

    h = n - 1;  // last index
    l = 0;      // first index

    // initialize top of stack
    int top = -1;

    // push initial values of l and h to stack
    stack[++top] = l;
    stack[++top] = h;

    // Keep popping from stack while it is not empty
    while (top >= 0) {
        // Pop h and l
        h = stack[top--];
        l = stack[top--];

        // Set pivot element at its correct position
        // in sorted array
        int p = partition(val, arr, l, h);

        // If there are elements on left side of pivot,
        // then push left side to stack
        if (p - 1 > l) {
            stack[++top] = l;
            stack[++top] = p - 1;
        }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if (p + 1 < h) {
            stack[++top] = p + 1;
            stack[++top] = h;
        }
    }
}

// routine to check the sort on index'ed sort
//
int is_index_sorted(double data[], int index[], int size) {
    int i;

    for (i = 0; i < (size - 1); i++) {
        if (data[index[i]] < data[index[i + 1]]) {
            return false;
        }
    }
    return true;
}
//
//  find the position within the sorted(index) array for a value
//  index  how to traverse the array
//  val  values to compare
//  n  size of index array
//  compare value to compare to  first value = > than val[index[i]]
//
int val_index_pos(int *index, double *val, int n, double compare) {
    int i;
    for (i = 0; i < n; i++) {
        if (compare >= val[index[i]]) break;
    }
    return i;
}

//
//  fill an ordered by size index array based on sizes of val
//  a quick sort is used so that I can get speed and do a value
//  index sort  ( measured as 2x faster than a qsort with
//  tricks to do an index sort)
//
void val_index_sort(int *index, double *val, int n) {
    int i;
    int *stack;  // temp space = n + 1
    // Create an auxiliary stack
    if ((GETMEM(stack, int, (n + 1))) == NULL) {
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

void val_index_sort_ns(int *index, double *val, int n) {
    int i;
    int *stack;  // temp space = n + 1
    // Create an auxiliary stack
    if ((GETMEM(stack, int, (n + 1))) == NULL) {
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

int compare_intsAsc(const void *p, const void *q) {
    int x = *(const int *)p;
    int y = *(const int *)q;

    if (x < y)
        return -1;  // return -1 if you want ascending, 1 if you want descending order.
    else if (x > y)
        return 1;  // return 1 if you want ascending, -1 if you want descending order.
    return 0;
}

int compare_intsDes(const void *p, const void *q) {
    int x = *(const int *)p;
    int y = *(const int *)q;

    if (x < y)
        return 1;  // return -1 if you want ascending, 1 if you want descending order.
    else if (x > y)
        return -1;  // return 1 if you want ascending, -1 if you want descending order.
    return 0;
}

//
// sort an index array
//
void index_sort(int *index, int n, short forward) {
    if (forward) {
        qsort(index, n, sizeof *index, &compare_intsAsc);
    } else {
        qsort(index, n, sizeof *index, &compare_intsDes);
    }
}

// compares two vectors, bit by bit
bool is_array_equal(int8_t *solution_a, int8_t *solution_b, int nbits) {
    for (int i = 0; i < nbits; i++) {
        if (solution_a[i] != solution_b[i]) {
            return false;
        }
    }
    return true;
}

//  compare, bit by bit solution_A with solution_B and save the index of the value where
//  they differ in index[].     Return the number of values in the index vector
//@param  solution_A = bit vector solution
//@param  solution_B = bit vector solution
//@param  nbits = length of the solution vectors
//@param  index is integer index vector of (index_solution_diff) length, will be ordered
//      small to large
//  ndiff number of differences between solution_A and B,, returned value
//
int index_solution_diff(int8_t *solution_A, int8_t *solution_B, int nbits, int *index) {
    int i, ndiff = 0;
    for (i = 0; i < nbits; i++) {
        if (solution_A[i] != solution_B[i]) {
            index[ndiff++] = i;
        }
    }
    for (i = ndiff; i < nbits; i++) {  // clean out the rest of the vector
        index[i] = 0;
    }
    return ndiff;
}

//  count, bit by bit between solutions and return the solution of the value where
//  they differ in index[] ( any one of the solutions not same as any other ).
//@param  popularSol[nbits] = bit vector solution, most popular setting on a bit
//@param  solution[num_solutions][nbits] = bit vector solution
//@param  num_solutions number of solutions in solution
//@param  nbits = length of the solution vectors
//@param  sol_index is integer index vector of solution differences, will be ordered
//@param  bias   between 0 and (num_solutions/2)   pattern = (5,2) (6,3) (7,3) (8,4)
//               ex bias=0 only all set to X, set to X, any set to Y , set to Y ( Y, X can be 0 or 1)
//               ex bias=1 all but 1 set to X, set to X , more than 1 set to Y, set to Y
void solution_population(int8_t *popularSol, int8_t **solution, int num_solutions, int nbits, int *sol_index,
                         int bias) {
    int i, j, sum_bits;
    for (i = 0; i < nbits; i++) {
        sum_bits = 0;
        for (j = 0; j < num_solutions; j++) {
            sum_bits += solution[sol_index[j]][i];
        }
        //  ex. all bits set to 1, sum_bits = num_solutions, if all 0 sum_bits = 0
        //    if > num_solutions/2 it is mirroring differences
        popularSol[i] = 0;
        if (sum_bits >= num_solutions / 2) popularSol[i] = 1;  // more than 1/2 1's
        if (sum_bits > (int)((num_solutions + 1) / 2) - 1) {
            sum_bits = num_solutions - sum_bits;
        }
        if (sum_bits > bias) {  // so if bias is greater than favor flipping the bit
            if (popularSol[i] == 1) {
                popularSol[i] = 0;
            } else {
                popularSol[i] = 1;
            }
        }
        // now sum_bits = number of differences,,
    }
    return;
}
//  compare, bit by bit between solutions and save the index of the value where
//  they differ in index[] ( any one of the solutions not same as any other ).
//      Return the number of values in the index vector
//@param  solution[num_solutions][nbits] = bit vector solution
//@param  num_solutions number of solutions in solution
//@param  nbits = length of the solution vectors
//@param  index is integer index vector of solution differences, will be ordered
//@param  delta_bits is integer to compare with to establish backbone in index.  if = 0 , all bits must be same, if =1,
//all but
//            one bit must be the same for that column of  bits, and so on.  delta_bits  >= 0 and <= num_solutions/2
//@param  sol_index is integer index vector of (index_solution_diff)  it is used to index into which solutions to
//              look at,, so that you might compare only a few of the solutions
//      small to large
//  ndiff number of differences between solution(s),, returned value
//
int mul_index_solution_diff(int8_t **solution, int num_solutions, int nbits, int *index, int delta_bits,
                            int *sol_index) {
    int i, j, ndiff = 0, sum_bits;
    for (i = 0; i < nbits; i++) {
        sum_bits = 0;
        for (j = 0; j < num_solutions; j++) {
            sum_bits += solution[sol_index[j]][i];
        }
        //  ex. all bits set to 1, sum_bits = num_solutions, if all 0 sum_bits = 0
        //    if > num_solutions/2 it is mirroring differences
        if (sum_bits > (int)((num_solutions + 1) / 2) - 1) sum_bits = num_solutions - sum_bits;
        // now sum_bits = number of differences,,
        if (sum_bits > delta_bits) {
            index[ndiff++] = i;  // this bit is different by more than delta_bits
        }
    }
    for (i = ndiff; i < nbits; i++) {  // clean out the rest of the vector
        index[i] = 0;
    }
    return ndiff;
}
//  print out each solution in index order per qbsolv output format
//      Return the number of values in the index vector
//@param  solution[num_solutions][nbits] = bit vector solution
//@param  energy_list is the 1d array of energies corresponding to solution_lists
//@param  num_solutions number of solutions in solution
//@param  nbits = length of the solution vectors
//@param  index is integer index vector of (index_solution_diff) length, will be ordered
//      small to large
//  ndiff number of differences between solution(s),, returned value
//
void print_solutions(int8_t **solution, double *energy_list, int *solutions_counts, int num_solutions, int nbits,
                     int *index) {
    int i, j, k;
    double delta, energy, top_energy;
    fprintf(outFile_, "delta energy  Energy of solution\tnfound\tindex\t i\t");
    fprintf(outFile_, " number of unique solutions %d\n", num_solutions);
    k = index[0];
    top_energy = energy_list[k];
    for (i = num_solutions - 1; i > -1; i--) {
        k = index[i];
        energy = energy_list[k];
        delta = top_energy - energy_list[k];
        fprintf(outFile_, "%8.5f \t  %8.5f \t %d \t %d \t %d \t", delta, energy, solutions_counts[k], k, i);
        for (j = 0; j < nbits; j++) {
            fprintf(outFile_, "%d", solution[k][j]);
        }
        fprintf(outFile_, "\n");
    }
    return;
}
//@param solution_now is the Q vector being looked at
//@param solution_list is the 2d array of Q vectors being stored
//@param energy_now is the energy of the Q vector being looked at
//@param energy_list is the 1d array of energies corresponding to solution_lists
//@param solution_counts is the 1d array of hits on the corresponding solution_lists
//@param list_order is the order of solution_list based upon energies
//@param nMax is size of the arrays (solution_list, energy_list...)
//@param num_nq_solutions is the number of unique solutions in the solution_list...)
// if solution_now is unique, and is better than or equal to the worst solution add it to solution_list
// if solution_now is not unique ( equal energy )  increment number of times found
struct sol_man_rslt manage_solutions(int8_t *solution_now, int8_t **solution_list, double energy_now,
                                     double *energy_list, int *solution_counts, int *list_order, int nMax, int nbits,
                                     int *num_nq_solutions) {
    struct sol_man_rslt result;
    val_index_sort_ns(list_order, energy_list, nMax);  // index array of sorted energies

    // printf(" %d ",(*num_nq_solutions));
    // new high value,
    if (energy_now > energy_list[list_order[0]]) {
        // we will add it to the space to an empty queue, Sort will fix it later
        int empty_row = list_order[nMax - 1];

        // save the bits to Qlist first entry
        for (int i = 0; i < nbits; i++) {
            solution_list[empty_row][i] = solution_now[i];
        }
        (*num_nq_solutions) = MIN((*num_nq_solutions) + 1, nMax);

        energy_list[empty_row] = energy_now;
        solution_counts[empty_row] = 1;

        // index array of sorted energies
        val_index_sort_ns(list_order, energy_list, nMax);

        // we have added this Qnow to the collective, might have overwritten an old one
        result.code = NEW_HIGH_ENERGY_UNIQUE_SOL;
        result.count = 1;
        result.pos = val_index_pos(list_order, energy_list, nMax, energy_now);
        if (Verbose_ > 3) {
            printf(" NEW_HIGH_ENERGY_UNIQUE_SOL   %lf %d %d\n", energy_now, result.count, result.pos);
        }
        return result;
    }

    // list energies are all higher than this, do nothing
    if (energy_now < energy_list[list_order[nMax - 1]]) {
        val_index_sort(list_order, energy_list, nMax);  // index array of sorted energies
        result.code = NOTHING;
        result.count = 0;
        result.pos = val_index_pos(list_order, energy_list, nMax, energy_now);
        if (Verbose_ > 3) {
            printf(" NOTHING                      %lf %d %d\n", energy_now, result.count, result.pos);
        }
        return result;
    }

    // new energy is in the range of our list
    // search thru the list to see if there is an equal energy
    for (int i = 0; i < nMax; i++) {
        if (energy_now == energy_list[list_order[i]]) {
            int j = i;  // now have a common energy, but it could be that we have a different Q

            // look thru all Q's of common energy (they are ordered)
            while (j < nMax && energy_list[list_order[j]] == energy_now) {
                if (is_array_equal(solution_list[list_order[j]], solution_now, nbits)) {
                    // simply mark this Q and energy as a duplicate find
                    solution_counts[list_order[j]]++;

                    if (energy_now == energy_list[list_order[0]]) {
                        // duplicate energy matching another Q and equal to best energy
                        result.code = DUPLICATE_HIGHEST_ENERGY;
                        result.count = solution_counts[list_order[0]];
                        result.pos = val_index_pos(list_order, energy_list, nMax, energy_now);
                        if (Verbose_ > 3) {
                            printf(" DUPLICATE_HIGHEST_ENERGY     %lf %d %d\n", energy_now, result.count, result.pos);
                        }
                        return result;
                    } else {
                        // duplicate energy matching older lower energy Q
                        result.code = DUPLICATE_ENERGY;
                        result.count = solution_counts[list_order[j]];
                        result.pos = val_index_pos(list_order, energy_list, nMax, energy_now);
                        if (Verbose_ > 3) {
                            printf(" DUPLICATE_ENERGY             %lf %d %d\n", energy_now, result.count, result.pos);
                        }
                        return result;
                    }
                }
                j++;
            }

            // fallen thru equal energies so we need to add it to the list
            j = list_order[nMax - 1];     // add it to the worst energy position ( prefilled with worst possible value )
            energy_list[j] = energy_now;  // save energy
            solution_counts[j] = 1;       // set number of hits as this is a first

            // save the bits to solution_list
            for (int i = 0; i < nbits; i++) {
                solution_list[j][i] = solution_now[i];
            }
            (*num_nq_solutions) = MIN((*num_nq_solutions) + 1, nMax);

            // Create index array of sorted energies
            result.count = solution_counts[j];
            val_index_sort(list_order, energy_list, nMax);
            if (energy_now == energy_list[list_order[0]]) {
                // duplicate highest energy unique Q and equal to best energy
                result.code = DUPLICATE_HIGHEST_ENERGY;
                result.pos = val_index_pos(list_order, energy_list, nMax, energy_now);
                if (Verbose_ > 3) {
                    printf(" DUPLICATE_ENERGY             %lf %d %d\n", energy_now, result.count, result.pos);
                }
                return result;
            } else {
                // duplicate energy matching older lower energy Q
                result.code = DUPLICATE_ENERGY_UNIQUE_SOL;
                result.pos = val_index_pos(list_order, energy_list, nMax, energy_now);
                if (Verbose_ > 3) {
                    printf(" DUPLICATE_ENERGY_UNIQUE_SOL  %lf %d %d\n", energy_now, result.count, result.pos);
                }
                return result;
            }
        }

        // we have spilled off the list of energies and need to add this one
        else if (energy_now > energy_list[list_order[i]]) {
            int j = list_order[nMax - 1];  // add it to the worst energy position as it is unique but within the list
            energy_list[j] = energy_now;   // save energy
            solution_counts[j] = 1;        // set number of hits as this is a first

            // save the bits to solution_list
            for (int i = 0; i < nbits; i++) {
                solution_list[j][i] = solution_now[i];
            }
            (*num_nq_solutions) = MIN((*num_nq_solutions) + 1, nMax);

            // create index array of sorted energies
            val_index_sort(list_order, energy_list, nMax);
            result.code = NEW_ENERGY_UNIQUE_SOL;
            result.count = solution_counts[list_order[j]];
            result.pos = val_index_pos(list_order, energy_list, nMax, energy_now);
            if (Verbose_ > 3) {
                printf(" NEW_ENERGY_UNIQUE_SOL  %lf %d %d\n", energy_now, result.count, result.pos);
            }
            return result;
        }
    }

    for (int iL = 0; iL < nMax; iL++) {
        printf(" %d %d %lf %d \n", list_order[iL], iL, energy_list[list_order[iL]], solution_counts[list_order[iL]]);
    }
    exit(9);
}

// write qubo file to *filename
void write_qubo(double **qubo, int nMax, const char *filename) {
    // Try to open the file
    FILE *file;
    if ((file = fopen(filename, "w")) == 0) {
        DL;
        printf(" failed to open %s\n", filename);
        exit(9);
    }

    // count the non-zero couplers and nodes
    int nNodes = 0, nCouplers = 0;
    for (int i = 0; i < nMax; i++) {
        if (qubo[i][i] != 0.0) nNodes++;
        for (int j = i + 1; j < nMax; j++) {
            if (qubo[i][j] != 0.0) nCouplers++;
        }
    }

    // Write out the header line
    fprintf(file, "p qubo 0 %d %d %d\n", nMax, nNodes, nCouplers);

    // Write out the details for all non-zero linear/quadratic elements
    for (int i = 0; i < nMax; i++) {
        if (qubo[i][i] != 0.0) fprintf(file, "%d %d %lf\n", i, i, qubo[i][i]);
    }
    for (int i = 0; i < nMax; i++) {
        for (int j = i + 1; j < nMax; j++) {
            if (qubo[i][j] != 0.0) fprintf(file, "%d %d %lf\n", i, j, qubo[i][j]);
        }
    }
    fclose(file);
}

/*double roundit(double value, int digits)
{
        if (value == 0.0) // otherwise it will return 'nan' due to the log10() of zero
                    return 0.0;

            double factor = pow(10.0, digits - ceil(log10(fabs(value))));
                return round(value * factor) / factor;
}*/
#if _WIN32
/* This code is public domain -- Will Hartung 4/9/09 */
#include <stdio.h>
#include <stdlib.h>

size_t getline_win(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    int size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = (char *)malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while (c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = (char *)realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}
#endif

#ifdef __cplusplus
}
#endif
