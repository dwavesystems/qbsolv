/*
 *
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

#include "dwsolv.h"
#include "extern.h"
#include "macros.h"
#include "qbsolv.h"
#include "util.h"

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

// This function Simply evaluates the objective function for a given solution.
//  call this to double check that a solution = energy
// @param solution a current solution
// @param qubo_size the number of variables in the QUBO matrix
// @param qubo the QUBO matrix being solved
// @returns Energy of solution evaluated by qubo
//
// Notes about const:
// const int8_t * const solution
// const double ** const qubo
//     neither the pointer nor the data can be changed
//
double Simple_evaluate(const int8_t *const solution, const uint qubo_size, const double **const qubo) {
    double result = 0.0;

    for (uint ii = 0; ii < qubo_size; ii++) {
        double row_sum = 0.0;

        // qubo an upper triangular matrix, so start right of the diagonal
        // for the rows, and stop at the diagonal for the columns
        for (uint jj = ii + 1; jj < qubo_size; jj++) row_sum += qubo[ii][jj] * (double)solution[jj];

        if (solution[ii] == 1) result += row_sum + qubo[ii][ii];
    }

    return result;
}
// This function evaluates the objective function for a given solution.
//
// It is called when the search is starting over, such as after a projection
// in outer loop of solver.
//
// @param solution a current solution
// @param qubo_size the number of variables in the QUBO matrix
// @param qubo the QUBO matrix being solved
// @param[out] flip_cost The change in energy from flipping a bit
// @returns Energy of solution evaluated by qubo
//
// Notes about const:
// int8_t * const solution
// double * const flip_cost
//     the pointer cannot be changed, but the data pointed to can be changed
// const double ** const qubo
//     neither the pointer nor the data can be changed
//
double evaluate(int8_t *const solution, const uint qubo_size, const double **const qubo, double *const flip_cost) {
    double result = 0.0;

    for (uint ii = 0; ii < qubo_size; ii++) {
        double row_sum = 0.0;
        double col_sum = 0.0;

        // qubo an upper triangular matrix, so start right of the diagonal
        // for the rows, and stop at the diagonal for the columns
        for (uint jj = ii + 1; jj < qubo_size; jj++)
            if (solution[jj]) row_sum += qubo[ii][jj];

        for (uint jj = 0; jj < ii; jj++)
            if (solution[jj]) col_sum += qubo[jj][ii];

        // If the variable is currently 1, then by flipping it we lose
        // what it is currently contributing (so we negate the contribution),
        // when it is currently false, gain that ammount by flipping
        double contrib = row_sum + col_sum + qubo[ii][ii];
        if (solution[ii] == 1) {
            result += row_sum + qubo[ii][ii];
            flip_cost[ii] = -contrib;
        } else {
            flip_cost[ii] = contrib;
        }
    }

    return result;
}
// Flips a given bit in the solution, and calculates the new energy.
//
// All the auxiliary information (flip_cost) is updated.
//
// @param old_energy The current objective function value
// @param bit is the bit to be flipped
// @param[in,out] solution inputs a current solution, flips the given bit
// @param qubo_size is the number of variables in the QUBO matrix
// @param qubo the QUBO matrix being solved
// @param[out] flip_cost The change in energy from flipping a bit
// @returns New energy of the modified solution
//
// Notes about const:
// int8_t * const solution
// double * const flip_cost
//     the pointer cannot be changed, but the data pointed to can be changed
// const double ** const qubo
//     neither the pointer nor the data can be changed
//
double evaluate_1bit(const double old_energy, const uint bit, int8_t *const solution, const uint qubo_size,
                     const double **const qubo, double *const flip_cost) {
    double result = old_energy + flip_cost[bit];

    // Flip the bit and reverse its flip_cost
    solution[bit] = 1 - solution[bit];
    flip_cost[bit] = -flip_cost[bit];

    // Update the flip cost for all of the adjacent variables
    if (solution[bit] == 0) {
        // for rows ii up to bit, the flip_cost[ii] changes by qubo[bit][ii]
        // for columns ii from bit+1 and higher, flip_cost[ii] changes by qubo[ii][bit]
        // the sign of the change (positive or negative) depends on both solution[bit] and solution[ii].
        for (uint ii = 0; ii < bit; ii++) flip_cost[ii] += qubo[ii][bit] * (solution[ii] - !solution[ii]);

        for (uint ii = bit + 1; ii < qubo_size; ii++) flip_cost[ii] += qubo[bit][ii] * (solution[ii] - !solution[ii]);

    } else {
        // if solution[bit] was a 1 before, flip_cost[ii] changes in the other direction.
        for (uint ii = 0; ii < bit; ii++) flip_cost[ii] -= qubo[ii][bit] * (solution[ii] - !solution[ii]);

        for (uint ii = bit + 1; ii < qubo_size; ii++) flip_cost[ii] -= qubo[bit][ii] * (solution[ii] - !solution[ii]);
    }
    // proposed code to clean up some numerical noise
    // for ( uint ii=0; ii< qubo_size; ii++)
    //      flip_cost[ii] = roundit(flip_cost[ii],10) ;

    return result;
}

// Tries to improve the current solution Q by flipping single bits.
// It flips a bit whenever a bit flip improves the objective function value,
// terminating when a local optimum is found.
// It returns the objective function value for the new solution.
//
// This routine does not perform a full evalution of the the state or auxiliary
// information, it assumes it is already up to date.
//
// @param energy The current objective function value
// @param[in,out] solution inputs a current solution, modified by local search
// @param[in] size is the number of variables in the QUBO matrix
// @param[in] qubo the QUBO matrix being solved
// @param[out] flip_cost The change in energy from flipping a bit
// @param[in,out] bit_flips is the number of candidate bit flips performed in the entire algorithm so far
// @returns New energy of the modified solution
double local_search_1bit(double energy, int8_t *solution, uint qubo_size, double **qubo, double *flip_cost,
                         int64_t *bit_flips) {
    int kkstr = 0, kkend = qubo_size, kkinc;
    int *index;
    if (GETMEM(index, int, qubo_size) == NULL) BADMALLOC

    for (uint kk = 0; kk < qubo_size; kk++) {
        index[kk] = kk;
    }

    // The local search terminates at the local optima, so the moment we can't
    // improve with a single bit flip
    bool improve = true;
    while (improve) {
        improve = false;

        if (kkstr == 0) {  // sweep top to bottom
            shuffle_index(index, qubo_size);
            kkstr = qubo_size - 1;
            kkinc = -1;
            kkend = -1;
        } else {  // sweep bottom to top
            kkstr = 0;
            kkinc = 1;
            kkend = qubo_size;  // got thru it backwards then reshuffle
        }

        for (int kk = kkstr; kk != kkend; kk = kk + kkinc) {
            uint bit = index[kk];
            (*bit_flips)++;
            if (flip_cost[bit] > 0.0) {
                energy = evaluate_1bit(energy, bit, solution, qubo_size, (const double **)qubo, flip_cost);
                improve = true;
            }
        }
    }
    free(index);
    return energy;
}

// Performs a local Max search improving the solution and returning the last evaluated value
//
// Mostly the same as local_search_1bit, except it first evaluates the
// current solution and updates the auxiliary information (flip_cost)
//
// @param[in,out] solution inputs a current solution, modified by local search
// @param size is the number of variables in the QUBO matrix
// @param[out] flip_cost The change in energy from flipping a bit
// @param bit_flips is the number of candidate bit flips performed in the entire algorithm so far
// @returns New energy of the modified solution
double local_search(int8_t *solution, int qubo_size, double **qubo, double *flip_cost, int64_t *bit_flips) {
    double energy;

    // initial evaluate needed before evaluate_1bit can be used
    energy = evaluate(solution, qubo_size, (const double **)qubo, flip_cost);
    energy = local_search_1bit(energy, solution, qubo_size, qubo, flip_cost,
                               bit_flips);  // local search to polish the change
    return energy;
}

// This function is called by solve to execute a tabu search, This is THE Tabu search
//
// A tabu optimization algorithm tries to find an approximately maximal solution
// to a QUBO problem. Tabu is an optimization algorithm which performs single
// bit flips on a current solution in an attempt to improve it. For each
// candidate bit flip, the change in objective function value is examined.
// If this results in a new best solution, that change is accepted.
// If no bit flip results in a new best solution, we choose the best among
// the candidate bit flips.
//
// In order to avoid getting stuck in local optima, a list of "tabu" (not-allowed)
// moves is maintained (the vector "tabuK") After a bit has been flipped,
// it cannot be flipped again for another "nTabu" moves. The algorithm terminates
// after sufficiently many bit flips without improvment.
//
// @param[in,out] solution inputs a current solution and returns the best solution found
// @param[out] best stores the best solution found during the algorithm
// @param qubo_size is the number of variables in the QUBO matrix
// @param qubo is the QUBO matrix to be solved
// @param flip_cost is the impact vector (the change in objective function value that results from flipping each bit)
// @param bit_flips is the number of candidate bit flips performed in the entire algorithm so far
// @param iter_max is the maximum size of bit_flips allowed before terminating
// @param TabuK stores the list of tabu moves
// @param target Halt if this energy is reached and TargetSet is true
// @param target_set Do we have a target energy at which to terminate
// @param index is the order in which to perform candidate bit flips (determined by flip_cost).
double tabu_search(int8_t *solution, int8_t *best, uint qubo_size, double **qubo, double *flip_cost, int64_t *bit_flips,
                   int64_t iter_max, int *TabuK, double target, bool target_set, int *index, int nTabu) {
    uint last_bit = 0;   // Track what the previously flipped bit was
    bool brk;            // flag to mark a break and not a fall-thru of the loop
    double best_energy;  // best solution so far
    double Vlastchange;  // working solution variable
    double sign;
    int64_t thisIter;
    int64_t increaseIter;
    int numIncrease = 900;
    double howFar;

    // setup nTabu
    // these nTabu numbers might need to be adjusted to work correctly
    if (nTabu == 0) {  // nTabu not specified on call
        if (Tlist_ != -1) {
            nTabu = MIN(Tlist_, (int)qubo_size + 1);  // tabu use set tenure
        } else {
            if (qubo_size < 20)
                nTabu = 10;
            else if (qubo_size < 100)
                nTabu = 10;
            else if (qubo_size < 250)
                nTabu = 12;
            else if (qubo_size < 500)
                nTabu = 13;
            else if (qubo_size < 1000)
                nTabu = 21;
            else if (qubo_size < 2500)
                nTabu = 29;
            else if (qubo_size < 8000)
                nTabu = 34;
            else /*qubo_size >= 8000*/
                nTabu = 35;
        }
    }

    sign = findMax_ ? 1.0 : -1.0;

    best_energy = local_search(solution, qubo_size, qubo, flip_cost, bit_flips);
    val_index_sort(index, flip_cost, qubo_size);  // Create index array of sorted values
    thisIter = iter_max - (*bit_flips);
    increaseIter = thisIter / 2;
    Vlastchange = best_energy;

    for (uint i = 0; i < qubo_size; i++) best[i] = solution[i];  // copy the best solution so far
    for (uint i = 0; i < qubo_size; i++) TabuK[i] = 0;           // zero out the Tabu vector

    int kk, kkstr = 0, kkend = qubo_size, kkinc;
    uint bit_cycle_1 = qubo_size, bit_cycle_2 = qubo_size, bit_cycle = 0;
    while (*bit_flips < iter_max) {
        // best solution in neighbour, initialized most negative number
        double neighbour_best = BIGNEGFP;
        brk = false;
        if (kkstr == 0) {  // sweep top to bottom
            kkstr = qubo_size - 1;
            kkinc = -1;
            kkend = -1;
        } else {  // sweep bottom to top
            kkstr = 0;
            kkinc = 1;
            kkend = qubo_size;
        }

        for (kk = kkstr; kk != kkend; kk = kk + kkinc) {
            uint bit = index[kk];
            if (TabuK[bit] != (int8_t)0) continue;
            {
                (*bit_flips)++;
                double new_energy = Vlastchange + flip_cost[bit];  //  value if Q[k] bit is flipped
                if (new_energy > best_energy && bit != bit_cycle_1) {
                    brk = true;
                    last_bit = bit;
                    float Delta_E = (float)(new_energy - best_energy);
                    new_energy = evaluate_1bit(Vlastchange, bit, solution, qubo_size, (const double **)qubo,
                                               flip_cost);  // flip the bit and fix tables
                    Vlastchange = local_search_1bit(new_energy, solution, qubo_size, qubo, flip_cost,
                                                    bit_flips);  // local search to polish the change
                    val_index_sort_ns(index, flip_cost,
                                      qubo_size);  // update index array of sorted values, don't shuffle index
                    best_energy = Vlastchange;

                    for (uint i = 0; i < qubo_size; i++) best[i] = solution[i];  // copy the best solution so far

                    howFar = ((double)(iter_max - (*bit_flips)) / (double)thisIter);
                    if (Verbose_ > 3) {
                        printf("Tabu new best %lf ,K=%d,last=%d, last_2=%d, cycle=%d,iteration = %" LONGFORMAT
                               ""
                               ", %lf, %d\n",
                               Vlastchange * sign, last_bit, bit_cycle_1, bit_cycle_2, bit_cycle, (int64_t)(*bit_flips),
                               howFar, brk);
                    }
                    if (target_set) {
                        if (Vlastchange >= (sign * target)) {
                            break;
                        }
                    }
                    //  trying to capture a non progressive cycle, after update, really not an advance
                    //  but have flipped a bit in a different place,, sometime a cycle of 3 bit positions
                    if (Delta_E <= 0.00000001) bit_cycle++;
                    if (bit_cycle_2 == bit_cycle_1) bit_cycle++;
                    if (bit_cycle_2 == last_bit) bit_cycle++;
                    if (bit_cycle > 4) break;

                    bit_cycle_2 = bit_cycle_1;
                    bit_cycle_1 = last_bit;
                    if (howFar < 0.80 && numIncrease > 0) {
                        if (Verbose_ > 3) {
                            printf("Increase Itermax %" LONGFORMAT ", %" LONGFORMAT "\n", iter_max,
                                   (iter_max + increaseIter));
                        }
                        iter_max += increaseIter;
                        thisIter += increaseIter;
                        numIncrease--;
                    }
                    break;
                }
                // Q vector unchanged
                if (new_energy > neighbour_best) {  // check for improved neighbour solution
                    last_bit = bit;                 // record position
                    neighbour_best = new_energy;    // record neighbour solution value
                }
            }
        }

        if (target_set) {
            if (Vlastchange >= (sign * target)) {
                break;
            }
        }
        if (bit_cycle > 6) break;

        if (!brk) {  // this is the fall-thru case and we haven't tripped interior If V> VS test so flip Q[K]
            Vlastchange = evaluate_1bit(Vlastchange, last_bit, solution, qubo_size, (const double **)qubo, flip_cost);
        }

        uint i;
        for (i = 0; i < qubo_size; i++) TabuK[i] = MAX(0, TabuK[i] - 1);

        // add some asymmetry
        if (solution[qubo_size - 1] == 0) {
            TabuK[last_bit] = nTabu + 1;
        } else {
            TabuK[last_bit] = nTabu - 1;
        }
    }

    // copy over the best solution
    for (uint i = 0; i < qubo_size; i++) solution[i] = best[i];

    // ok, we are leaving Tabu, we can do a for-sure clean-up run of evaluate, to be sure we
    // return the true evaluation of the function (given that we only do this a handful of times)
    double final_energy = evaluate(solution, qubo_size, (const double **)qubo, flip_cost);

    // Create index array of sorted values
    val_index_sort(index, flip_cost, qubo_size);
    return final_energy;
}

// reduce() computes a subQUBO (val_s) from large QUBO (val)
// for the purposes of optimizing on a subregion (a subset of the variables).
// It does this by fixing all variables outside the subregion to their current values,
// and adding the influence of the fixed variables to the linear (diagonal) terms in the subQUBO.
//
// @param Icompress is the list of variables in the subregion that will be extracted
// @param qubo is the large QUBO matrix to be solved
// @param sub_qubo_size is the number of variable in the subregion
// @param qubo_size is the number of variables in the large QUBO matrix
// @param[out] sub_qubo is the returned subQUBO
// @param[out] sub_solution is a current solution on the subQUBO
void reduce(int *Icompress, double **qubo, uint sub_qubo_size, uint qubo_size, double **sub_qubo, int8_t *solution,
            int8_t *sub_solution) {
    // clean out the subMatrix
    for (uint i = 0; i < sub_qubo_size; i++) {                          // for each column
        for (uint j = 0; j < sub_qubo_size; j++) sub_qubo[i][j] = 0.0;  // for each row
    }

    // fill the subMatrix
    for (uint i = 0; i < sub_qubo_size; i++) {  // for each column
        sub_solution[i] = solution[Icompress[i]];
        for (uint j = i; j < sub_qubo_size; j++) {  // copy row
            sub_qubo[i][j] = qubo[Icompress[i]][Icompress[j]];
        }
    }

    // The remainder of the function is clamping the sub_qubo to the
    // solution state surrounding it.
    // Go over every variable that we are extracting
    for (uint sub_variable = 0; sub_variable < sub_qubo_size; sub_variable++) {
        // Get the global index of the current variable
        int variable = Icompress[sub_variable];
        double clamp = 0;

        // this will keep track of the index of the next sub_qubo component,
        // we don't include those in the clamping
        int ji = sub_qubo_size - 1;

        // Go over all other (non-extracted) variables
        // from the highest until we reach the current variable
        for (int j = qubo_size - 1; j > variable; j--) {
            if (j == Icompress[ji]) {
                // Found a sub_qubo element, skip it, watch for the next one
                ji--;
            } else {
                clamp += qubo[variable][j] * solution[j];
            }
        }

        // Go over all other (non-extracted) variables
        // from zero until we reach the current variable
        ji = 0;
        for (int j = 0; j < variable + 1; j++) {
            if (j == Icompress[ji]) {
                // Found a sub_qubo element, skip it, watch for the next one
                ji++;
            } else {
                clamp += qubo[j][variable] * solution[j];
            }
        }

        // Now that we know what the effects of the non-extracted variables
        // are on the sub_qubo we include it by adding it as a linear
        // bias in the sub_qubo (a diagonal matrix entry)
        sub_qubo[sub_variable][sub_variable] += clamp;
    }
    return;
}

// solv_submatrix() performs QUBO optimization on a subregion.
// In this function the subregion is optimized using tabu_search() rather than using the D-Wave hardware.
//
// @param[in,out] solution inputs a current solution and returns the best solution found
// @param[out] best stores the best solution found during the algorithm
// @param qubo_size is the number of variables in the QUBO matrix
// @param qubo is the QUBO matrix to be solved
// @param flip_cost is the impact vector (the change in objective function value that results from flipping each bit)
// @param bit_flips is the number of candidate bit flips performed in the entire algorithm so far
// @param TabuK stores the list of tabu moves
// @param index is the order in which to perform candidate bit flips (determined by Qval).
double solv_submatrix(int8_t *solution, int8_t *best, uint qubo_size, double **qubo, double *flip_cost,
                      int64_t *bit_flips, int *TabuK, int *index) {
    int nTabu;
    int64_t iter_max = (*bit_flips) + (int64_t)MAX((int64_t)3000, (int64_t)20000 * (int64_t)qubo_size);
    if (qubo_size < 20)
        nTabu = 10;
    else if (qubo_size < 100)
        nTabu = 10;
    else if (qubo_size < 250)
        nTabu = 12;
    else if (qubo_size < 500)
        nTabu = 13;
    else if (qubo_size < 1000)
        nTabu = 21;
    else if (qubo_size < 2500)
        nTabu = 29;
    else if (qubo_size < 8000)
        nTabu = 34;
    else /*qubo_size >= 8000*/
        nTabu = 35;

    return tabu_search(solution, best, qubo_size, qubo, flip_cost, bit_flips, iter_max, TabuK, Target_, false, index,
                       nTabu);
}
// reduce_solv_projection reduces from a submatrix solves the QUBO projects the solution and
//      returns the number of changes
// @param Icompress index vector , ordered lowest to highest, of the row/columns to extract subQubo
// @param qubo is the QUBO matrix to extract from
// @param qubo_size is the number of variables in the QUBO matrix
// @param subMatrix is the size of the subMatrix to create and solve
// @param[in,out] solution inputs a current solution and returns the projected solution
// @param[out] stores the new, projected solution found during the algorithm
int reduce_solve_projection(int *Icompress, double **qubo, int qubo_size, int subMatrix, int8_t *solution,
                            parameters_t *param) {
    int change = 0;
    int8_t *sub_solution = (int8_t *)malloc(sizeof(int8_t) * subMatrix);
    double **sub_qubo;

    sub_qubo = (double **)malloc2D(qubo_size, qubo_size, sizeof(double));

    reduce(Icompress, qubo, subMatrix, qubo_size, sub_qubo, solution, sub_solution);
    // solve
    if (Verbose_ > 3) {
        printf("\nBits set before solver ");
        for (int j = 0; j < subMatrix; j++) printf("%d", solution[Icompress[j]]);
    }

    for (int i = 0; i < subMatrix; i++) {
        sub_solution[i] = solution[Icompress[i]];
    }

    param->sub_sampler(sub_qubo, subMatrix, sub_solution, param->sub_sampler_data);

    // modification to write out subqubos
    // char subqubofile[sizeof "subqubo10000.qubo"];
    // sprintf(subqubofile,"subqubo%05ld.qubo",numPartCalls);
    // write_qubo(sub_qubo,subMatrix,subqubofile);

    // projection
    if (Verbose_ > 3) {
        printf("\nBits set after solver  ");
        for (int j = 0; j < subMatrix; j++) printf("%d", sub_solution[j]);
        printf("\n");
    }
    for (int j = 0; j < subMatrix; j++) {
        int bit = Icompress[j];
        if (solution[bit] != sub_solution[j]) change++;
        solution[bit] = sub_solution[j];
    }

    free(sub_solution);
    free(sub_qubo);
    return change;
}

void dw_sub_sample(double **sub_qubo, int subMatrix, int8_t *sub_solution, void *sub_sampler_data) {
    dw_solver(sub_qubo, subMatrix, sub_solution);
    int64_t sub_bit_flips = 0;  //  run a local search with higher precision than the Dwave
    double *flip_cost = (double *)malloc(sizeof(double) * subMatrix);
    local_search(sub_solution, subMatrix, sub_qubo, flip_cost, &sub_bit_flips);
    free(flip_cost);
}

void tabu_sub_sample(double **sub_qubo, int subMatrix, int8_t *sub_solution, void *sub_sampler_data) {
    int *TabuK;
    int *index;
    double *flip_cost = (double *)malloc(sizeof(double) * subMatrix);
    int8_t *current_best = (int8_t *)malloc(sizeof(int8_t) * subMatrix);

    if (GETMEM(TabuK, int, subMatrix) == NULL) BADMALLOC
    if (GETMEM(index, int, subMatrix) == NULL) BADMALLOC

    int64_t bit_flips = 0;
    for (int i = 0; i < subMatrix; i++) {
        TabuK[i] = 0;
        index[i] = i;
        current_best[i] = sub_solution[i];
    }
    solv_submatrix(sub_solution, current_best, subMatrix, sub_qubo, flip_cost, &bit_flips, TabuK, index);

    free(current_best);
    free(flip_cost);
    free(index);
    free(TabuK);
}

// Define the default set of parameters for the solve routine
parameters_t default_parameters() {
    parameters_t param;
    param.repeats = 50;
    param.sub_sampler = &tabu_sub_sample;
    param.sub_size = 47;
    param.sub_sampler_data = NULL;
    return param;
}

// Entry into the overall solver from the main program
//
// It is the main function for solving a quadratic boolean optimization problem.
//
// The algorithm alternates between:
//   1) performing a global tabu search (tabu_search()) from the current solution
//   2) selecting subregions and optimizing on each of those subregions with
//      all other variables clamped (solv_submatrix()).
//
// Subregions are chosen based on increasing impact ("flip_cost"). Impact
// is defined as the change in objective function value that occurs by flipping a bit.
// The first subregion is chosen as the N least impactful variables, and so on.
// When none of the variables in any of the subregions change, a new solution
// is chosen based on randomizing those variables.
//
// After Pchk = 8 iterations with no improvement, the algorithm is
//   completely restarted with a new random solution.
// After nRepeats iterations with no improvement, the algorithm terminates.
//
// @param qubo The QUBO matrix to be solved
// @param qubo_size is the number of variables in the QUBO matrix
// @param[out] solution_list output solution table
// @param[out] energy_list output energy table
// @param[out] solution_counts output occurence table
// @param[out] Qindex order of entries in the solution table
// @param QLEN Number of entries in the solution table
// @param[in,out] param Other parameters to the solve method that have default values.
void solve(double **qubo, const int qubo_size, int8_t **solution_list, double *energy_list, int *solution_counts,
           int *Qindex, int QLEN, parameters_t *param) {
    double *flip_cost, energy;
    int *TabuK, *index, start_;
    int8_t *solution, *tabu_solution;
    long numPartCalls = 0;
    int64_t bit_flips = 0, IterMax;

    start_ = clock();
    bit_flips = 0;

    // Get some memory for the larger val matrix to solve
    if (GETMEM(solution, int8_t, qubo_size) == NULL) BADMALLOC
    if (GETMEM(tabu_solution, int8_t, qubo_size) == NULL) BADMALLOC
    if (GETMEM(flip_cost, double, qubo_size) == NULL) BADMALLOC
    if (GETMEM(index, int, qubo_size) == NULL) BADMALLOC
    if (GETMEM(TabuK, int, qubo_size) == NULL) BADMALLOC

    int num_nq_solutions = 0;

    for (int i = 0; i < QLEN + 1; i++) {
        energy_list[i] = BIGNEGFP;
        solution_counts[i] = 0;
        for (int j = 0; j < qubo_size; j++) {
            solution_list[i][j] = 0;
        }
    }

    // get some memory for reduced sub matrices
    // int8_t  *sub_solution, *Qt_s, *Qbest;
    int8_t *Qbest;
    double best_energy;
    int *Pcompress;

    if (GETMEM(Pcompress, int, qubo_size) == NULL) BADMALLOC
    // initialize and set some tuning parameters
    //
    const int Progress_check = 12;                // number of non-progresive passes thru main loop before reset
    const float SubMatrix_span = 0.214f;          // percent of the total size will be covered by the subMatrix pass
    const int64_t InitialTabuPass_factor = 6500;  // initial pass factor for tabu iterations
    const int64_t TabuPass_factor = 1700;         // iterative pass factor for tabu iterations

    const int subMatrix = param->sub_size;
    int MaxNodes_sub = MAX(subMatrix + 1, SubMatrix_span * qubo_size);
    int l_max = MIN(qubo_size - subMatrix, MaxNodes_sub);
    int len_index = 0;

    randomize_solution(tabu_solution, qubo_size);
    for (int i = 0; i < qubo_size; i++) {
        index[i] = i;  // initial index to 0,1,2,...qubo_size
        solution[i] = 0;
    }

    int l = 0, DwaveQubo = 0;
    double sign = findMax_ ? 1.0 : -1.0;
    struct sol_man_rslt result;

    // run initial Searches to prime the solutions for outer loop based upon algorithm choice
    //
    if (strncmp(&algo_[0], "o", strlen("o")) == 0) {
        IterMax = bit_flips + (int64_t)MAX((int64_t)400, InitialTabuPass_factor * (int64_t)qubo_size);
        if (Verbose_ > 2) {
            DLT;
            printf(" Starting Full initial Tabu\n");
        }
        energy = tabu_search(solution, tabu_solution, qubo_size, qubo, flip_cost, &bit_flips, IterMax, TabuK, Target_,
                             TargetSet_, index, 0);

        // save best result
        best_energy = energy;
        result = manage_solutions(solution, solution_list, energy, energy_list, solution_counts, Qindex, QLEN,
                                  qubo_size, &num_nq_solutions);
        Qbest = &solution_list[Qindex[0]][0];

    } else if (strncmp(&algo_[0], "d", strlen("d")) == 0) {
        // when using this method we need at least solutions for a "differential" backbone this
        // step is to prime the solution sets with at least one more
        //
        len_index = 0;
        int pass = 0;
        while (len_index < MIN(1 * subMatrix, qubo_size / 2)) {
            // DL;printf(" len_index %d %d \n",len_index,pass);
            randomize_solution(solution, qubo_size);
            energy = local_search(solution, qubo_size, qubo, flip_cost, &bit_flips);
            result = manage_solutions(solution, solution_list, energy, energy_list, solution_counts, Qindex, QLEN,
                                      qubo_size, &num_nq_solutions);
            len_index = mul_index_solution_diff(solution_list, num_nq_solutions, qubo_size, Pcompress, 0, Qindex);
            if (pass++ > 40) break;
            // printf(" len_index = %d  NU %d  energy %lf\n",len_index,NU,energy);
        }
        solution_population(solution, solution_list, num_nq_solutions, qubo_size, Qindex, 10);
        IterMax = bit_flips + (int64_t)MAX((int64_t)40, InitialTabuPass_factor * (int64_t)qubo_size / 2);
        energy = tabu_search(solution, tabu_solution, qubo_size, qubo, flip_cost, &bit_flips, IterMax, TabuK, Target_,
                             TargetSet_, index, 0);
        result = manage_solutions(solution, solution_list, energy, energy_list, solution_counts, Qindex, QLEN,
                                  qubo_size, &num_nq_solutions);
        Qbest = &solution_list[Qindex[0]][0];
        best_energy = energy_list[Qindex[0]];

    } else {
        fprintf(stderr, "Did not recognize algorithm %s\n", algo_);
        exit(2);
    }

    val_index_sort(index, flip_cost, qubo_size);  // create index array of sorted values
    if (Verbose_ > 0) {
        print_output(qubo_size, solution, numPartCalls, best_energy * sign, CPSECONDS, param);
    }
    if (Verbose_ > 1) {
        DLT;
        printf(" V Starting outer loop =%lf iterations %" LONGFORMAT "\n", best_energy * sign, bit_flips);
    }

    // starting main search loop Partition ( run parts on tabu or Dwave ) --> Tabu rinse and repeat
    short RepeatPass = 0, NoProgress = 0;
    short ContinueWhile = false;
    if (TargetSet_) {
        if (best_energy >= (sign * Target_)) {
            ContinueWhile = false;
        } else {
            ContinueWhile = true;
        }
    } else {
        if (RepeatPass < param->repeats) {
            ContinueWhile = true;
        } else {
            ContinueWhile = false;
        }
    }

    DwaveQubo = 0;

    // outer loop begin
    while (ContinueWhile) {
        if (qubo_size > 20 &&
            subMatrix < qubo_size) {  // these are of the size that will use updates from submatrix processing
            if (strncmp(&algo_[0], "o", strlen("o")) == 0) {
                // use the first "remove" index values to remove rows and columns from new matrix
                // initial TabuK to nothing tabu sub_solution[i] = Q[i];
                // create compression bit vector
                val_index_sort(index, flip_cost, qubo_size);  // Create index array of sorted values
                l_max = MIN(qubo_size - subMatrix, MaxNodes_sub);
                if (Verbose_ > 1)
                    printf("Reduced submatrix solution l = 0; %d, subMatrix size = %d\n", l_max, subMatrix);
            } else if (strncmp(&algo_[0], "d", strlen("d")) == 0) {
                // pick "backbone" as an index of non-matching bits in solutions
                //
                len_index = mul_index_solution_diff(solution_list, num_nq_solutions, qubo_size, Pcompress, 0, Qindex);
                // need to cover all of len_index so we will pad out the Qindex to a multiple of subMatrix
                l_max = len_index;
            }

            // begin submatrix passes
            if (NoProgress % Progress_check ==
                (Progress_check - 1)) {  // every Progress_check (th) loop without progess
                // reset completely
                // solution_population( solution, solution_list, num_nq_solutions, qubo_size, Qindex);
                randomize_solution(solution, qubo_size);
                if (Verbose_ > 1) {
                    DLT;
                    printf(" \n\n Reset Q and start over Repeat = %d/%d, as no progress is exhausted %d %d\n\n\n",
                           param->repeats, RepeatPass, NoProgress, NoProgress % Progress_check);
                }
            } else {
                int change = 0;
                {  // scope of parallel region
                    int t_change = 0;
                    int *Icompress;
                    if (GETMEM(Icompress, int, qubo_size) == NULL) BADMALLOC
                    for (l = 0; l < l_max; l += subMatrix) {
                        if (strncmp(&algo_[0], "o", strlen("o")) == 0) {
                            if (Verbose_ > 3) printf("Submatrix starting at backbone %d\n", l);

                            for (int i = l, j = 0; i < l + subMatrix; i++) {
                                Icompress[j++] = index[i];  // create compression index
                            }
                            index_sort(Icompress, subMatrix, true);  // sort it for effective reduction

                            // coarsen and reduce the problem
                        } else if (strncmp(&algo_[0], "d", strlen("d")) == 0) {
                            if (Verbose_ > 3) printf("Submatrix starting at backbone %d\n", l);
                            int i_strt = l;
                            if (l + subMatrix > len_index)
                                i_strt = len_index - subMatrix - 1;  // cover all of len_index by backup on last pass
                            for (int i = i_strt, j = 0; i < i_strt + subMatrix; i++) {
                                Icompress[j++] = Pcompress[i];  // create compression index
                            }
                        }
                        t_change = reduce_solve_projection(Icompress, qubo, qubo_size, subMatrix, solution, param);
                        // do the following in a critical region

                        change = change + t_change;
                        numPartCalls++;
                        DwaveQubo++;
                        // end critical region
                    }
                    free(Icompress);
                }

                // submatrix search did not produce enough new values, so randomize those bits
                if (change <= 2) {
                    if (strncmp(&algo_[0], "o", strlen("o")) == 0) {
                        flip_solution_by_index(solution, l, index);
                        // randomize_solution_by_index(solution, l, index);
                    } else if (strncmp(&algo_[0], "d", strlen("d")) == 0) {
                        len_index = mul_index_solution_diff(solution_list, num_nq_solutions, qubo_size, Pcompress, 0,
                                                            Qindex);
                        flip_solution_by_index(solution, len_index, Pcompress);
                        // randomize_solution_by_index(solution, len_index, Pcompress);
                    }
                    if (Verbose_ > 3) {
                        printf(" Submatrix search did not produce enough new values, so randomize %d bits\n", l);
                    }
                } else {
                    if (Verbose_ > 3) {
                        printf("Number of solution Bits changed %d \n ", change);
                    }
                }

                // completed submatrix passes
                if (Verbose_ > 1) printf("\n");
            }
        }
        if (Verbose_ > 1) {
            DLT;
            printf(" ***Full Tabu  -- after partition pass \n");
        }
        // FULL TABU run here

        IterMax = bit_flips + TabuPass_factor * (int64_t)qubo_size;
        val_index_sort(index, flip_cost, qubo_size);  // Create index array of sorted values
        energy = tabu_search(solution, tabu_solution, qubo_size, qubo, flip_cost, &bit_flips, IterMax, TabuK, Target_,
                             TargetSet_, index, 0);
        val_index_sort(index, flip_cost, qubo_size);  // Create index array of sorted values

        if (Verbose_ > 1) {
            DLT;
            printf("Latest answer  %4.5f iterations =%" LONGFORMAT "\n", energy * sign, (int64_t)bit_flips);
        }

        result = manage_solutions(solution, solution_list, energy, energy_list, solution_counts, Qindex, QLEN,
                                  qubo_size, &num_nq_solutions);
        Qbest = &solution_list[Qindex[0]][0];
        best_energy = energy_list[Qindex[0]];

        // print_solutions( solution_list,energy_list,solution_counts,num_nq_solutions,qubo_size,Qindex);
        if (result.code == NEW_HIGH_ENERGY_UNIQUE_SOL) {  // better solution
            RepeatPass = 0;

            if (Verbose_ > 1) {
                DLT;
                printf(" IMPROVEMENT; RepeatPass set to %d\n", RepeatPass);
            }
            if (Verbose_ > 0) {
                print_output(qubo_size, Qbest, numPartCalls, best_energy * sign, CPSECONDS, param);
            }
        } else if (result.code == DUPLICATE_ENERGY ||
                   result.code == DUPLICATE_HIGHEST_ENERGY) {  // equal solution, but it is different
            if (result.pos > 4 || result.count > 8) {
                randomize_solution(solution, qubo_size);
            }
            RepeatPass++;
            if (result.code == DUPLICATE_ENERGY) {
                NoProgress++;
            }
            if (result.code == DUPLICATE_HIGHEST_ENERGY && result.count == 1) {
                if (Verbose_ > 0) {
                    print_output(qubo_size, Qbest, numPartCalls, best_energy * sign, CPSECONDS, param);
                }
            }
        } else if (result.code == NOTHING) {  // not as good as our worst so far
            RepeatPass++;
            NoProgress++;
            if (Verbose_ > 1) {
                printf("NO improvement RepeatPass =%d\n", RepeatPass);
            }
        }

        if (Verbose_ > 1) {
            DLT;
            printf("V Best outer loop =%lf iterations %" LONGFORMAT "\n", best_energy * sign, bit_flips);
        }

        // check on, if to continue the outer loop
        if (TargetSet_) {
            if (best_energy >= (sign * Target_)) {
                ContinueWhile = false;
            } else {
                ContinueWhile = true;
            }
        } else {
            if (RepeatPass < param->repeats) {
                ContinueWhile = true;
            } else {
                ContinueWhile = false;
            }
        }

        // timeout test
        if (CPSECONDS >= Time_) {
            ContinueWhile = false;
        }
    }  // end of outer loop

    // all done print results if needed and free allocated arrays
    if (WriteMatrix_) print_solution_and_qubo(Qbest, qubo_size, qubo);

    if (Verbose_ == 0) {
        Qbest = &solution_list[Qindex[0]][0];
        best_energy = energy_list[Qindex[0]];
        // printf(" evaluated solution %8.2lf\n",
        //     sign * Simple_evaluate(Qbest, qubo_size, (const double **)qubo));
        print_output(qubo_size, Qbest, numPartCalls, best_energy * sign, CPSECONDS, param);
    }

    free(solution);
    free(tabu_solution);
    free(flip_cost);
    free(index);
    free(TabuK);
    free(Pcompress);

    return;
}

#ifdef __cplusplus
}
#endif
