/*
 Copyright 2016 D-Wave Systems Inc

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
double evaluate(int8_t *solution, uint qubo_size, double **qubo, double *flip_cost)
{
	double result = 0.0;

	for (uint ii = 0; ii < qubo_size; ii++) {
		double row_sum = 0.0;
		double col_sum = 0.0;

		// qubo an upper triangular matrix, so start right of the diagonal
		// for the rows, and stop at the diagonal for the columns
		for (uint jj = ii + 1; jj < qubo_size; jj++)
			row_sum += qubo[ii][jj] * (double)solution[jj];

		for (uint jj = 0; jj < ii; jj++)
			col_sum += qubo[jj][ii] * (double)solution[jj];

		// If the variable is currently 1, then by flipping it we lose
		// what it is currently contributing (so we negate the contribution),
		// when it is currently false, gain that ammount by flipping
		if (solution[ii] == 1) {
			result += row_sum + qubo[ii][ii];
			flip_cost[ii] = -(row_sum + col_sum + qubo[ii][ii]);
		} else {
			flip_cost[ii] =  (row_sum + col_sum + qubo[ii][ii]);
		}
	}

	return result;
}


// Flips a given bit in the solution, and calculates the new energy.
//
// All the auxillary informanion (flip_cost) is updated.
//
// @param old_energy The current objective function value
// @param bit is the bit to be flipped
// @param[in,out] solution inputs a current solution, flips the given bit
// @param qubo_size is the number of variables in the QUBO matrix
// @param qubo the QUBO matrix being solved
// @param[out] flip_cost The change in energy from flipping a bit
// @returns New energy of the modified solution
double evaluate_1bit(double old_energy, uint bit, int8_t *solution, uint qubo_size,
	double **qubo, double *flip_cost)
{
	double result = old_energy + flip_cost[bit];

	// Flip the bit and reverse its flip_cost
	solution[bit] = 1 - solution[bit];
	flip_cost[bit] = -flip_cost[bit];

	// Update the flip cost for all of the adjacent variables
	if (solution[bit] == 0 ) {
		// for rows ii up to bit, the flip_cost[ii] changes by qubo[bit][ii]
		// for columns ii from bit+1 and higher, flip_cost[ii] changes by qubo[ii][bit]
		// the sign of the change (positive or negative) depends on both solution[bit] and solution[ii].
		for (uint ii = 0; ii < bit; ii++)
			flip_cost[ii] += qubo[ii][bit]*(solution[ii]-!solution[ii]);

		for (uint ii = bit + 1; ii < qubo_size; ii++)
			flip_cost[ii] += qubo[bit][ii]*(solution[ii]-!solution[ii]);

	} else {
		// if solution[bit] was a 1 before, flip_cost[ii] changes in the other direction.
		for (uint ii = 0; ii < bit; ii++)
			flip_cost[ii] -= qubo[ii][bit]*(solution[ii]-!solution[ii]);

		for (uint ii = bit + 1; ii < qubo_size; ii++)
			flip_cost[ii] -= qubo[bit][ii]*(solution[ii]-!solution[ii]);

	}

	return result;
}


// Tries to improve the current solution Q by flipping single bits.
// It flips a bit whenever a bit flip improves the objective function value,
// terminating when a local optimum is found.
// It returns the objective function value for the new solution.
//
// This routine does not perform a full evalution of the the state or auxillary
// information, it assumes it is already up to date.
//
// @param energy The current objective function value
// @param[in,out] solution inputs a current solution, modified by local search
// @param[in] size is the number of variables in the QUBO matrix
// @param[in] qubo the QUBO matrix being solved
// @param[out] flip_cost The change in energy from flipping a bit
// @param[in,out] t is the number of candidate bit flips performed in the entire algorithm so far
// @returns New energy of the modified solution
double local_search_1bit(double energy, int8_t *solution, uint qubo_size,
	double **qubo, double *flip_cost, int64_t *t)
{
	int kkstr = 0, kkend = qubo_size, kkinc;
	int index[qubo_size];

	for (uint kk = 0; kk < qubo_size; kk++) {
		index[kk] = kk;
	}

	// The local search terminates at the local optima, so the moment we can't
	// improve with a single bit flip
	bool improve = true;
	while (improve) {
		improve = false;

		if (kkstr == 0) { // sweep top to bottom
			shuffle_index(index, qubo_size);
			kkstr = qubo_size - 1; kkinc = -1; kkend = 0;
		} else { // sweep bottom to top
			kkstr = 0; kkinc = 1; kkend = qubo_size; // got thru it backwards then reshuffle
		}

		for (int kk = kkstr; kk != kkend; kk = kk + kkinc) {
			uint bit = index[kk];
			(*t)++;
			if (energy + flip_cost[bit] > energy) {
				energy  = evaluate_1bit(energy, bit, solution, qubo_size, qubo, flip_cost);
				improve = true;
			}
		}
	}
	return energy;
}

// Performance a local Max search improving the solution and returning the last evaluated value
//
// Mostly the same as local_search_1bit, except it first evaluates the
// current solution and updates the auxillary information (flip_cost)
//
// @param[in,out] solution inputs a current solution, modified by local search
// @param size is the number of variables in the QUBO matrix
// @param[out] flip_cost The change in energy from flipping a bit
// @param t is the number of candidate bit flips performed in the entire algorithm so far
// @returns New energy of the modified solution
double local_search(int8_t *solution, int qubo_size, double **qubo,
	double *flip_cost, int64_t *t)
{
	double energy;

	// initial evaluate needed before evaluate_1bit can be used
	energy = evaluate(solution, qubo_size, qubo, flip_cost);
	energy = local_search_1bit(energy, solution, qubo_size, qubo, flip_cost, t); // local search to polish the change
	return energy;
}

// This function is called by solve to execute a tabu search, This is THE Tabu search
//
// A tabu optmization algorithm tries find an approximately maximal solution
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
// @param flip_cost is the impact vector (the chainge in objective function value that results from flipping each bit)
// @param t is the number of candidate bit flips performed in the entire algorithm so far
// @param iter_max is the maximum size of t allowed before terminating
// @param TabuK is stores the list of tabu moves
// @param target Halt if this energy is reached and TargetSet is true
// @param target_set Do we have a target energy at which to terminate
// @param index is the order in which to perform candidate bit flips (determined by flip_cost).
double tabu_search(int8_t *solution, int8_t *best, uint qubo_size, double **qubo,
	double *flip_cost, int64_t *t, int64_t iter_max,
	int *TabuK, double target, bool target_set, int *index)
{

	uint      last_bit = 0;	// Track what the previously flipped bit was
	bool      brk; // flag to mark a break and not a fall thru of the loop
	double    best_energy; // best solution so far
	double    Vlastchange; // working solution variable
	int       nTabu;
	double    sign;
	int64_t thisIter;
	int64_t increaseIter;
	int       numIncrease = 900;
	double    howFar;

	// setup nTabu
	// these nTabu numbers might need to be adjusted to work correctly
	if (Tlist_ != -1) {
		nTabu = MIN(Tlist_, (int)qubo_size + 1 ); // tabu use set tenure
	} else {
		if      (qubo_size < 100)  nTabu = 10;
		else if (qubo_size < 250)  nTabu = 12;
		else if (qubo_size < 500)  nTabu = 13;
		else if (qubo_size < 1000) nTabu = 21;
		else if (qubo_size < 2500) nTabu = 29;
		else if (qubo_size < 8000) nTabu = 34;
		else /*qubo_size >= 8000*/ nTabu = 35;
	}

	if ( findMax_ ) {
		sign = 1.0;
	} else {
		sign = -1.0;
	}

	best_energy  = local_search(solution, qubo_size, qubo, flip_cost, t);
	val_index_sort(index, flip_cost, qubo_size); // Create index array of sorted values
	thisIter     = iter_max - (*t);
	increaseIter = thisIter / 2;
	Vlastchange  = best_energy;

	for (uint i = 0; i < qubo_size; i++) best[i] = solution[i]; // copy the best solution so far
	for (uint i = 0; i < qubo_size; i++) TabuK[i] = 0; // zero out the Tabu vector

	int kk, kkstr = 0, kkend = qubo_size, kkinc;
	while (*t < iter_max) {
		// best solution in neighbour, initialized most negative number
		double neighbour_best = BIGNEGFP;
		brk = false;
		if ( kkstr == 0 ) { // sweep top to bottom
			kkstr = qubo_size - 1; kkinc = -1; kkend = 0;
		} else { // sweep bottom to top
			kkstr = 0; kkinc = 1; kkend = qubo_size;
		}

		for (kk = kkstr; kk != kkend; kk = kk + kkinc) {
			uint bit = index[kk];
			if (TabuK[bit] != (int8_t)0 ) continue;
			{
				(*t)++;
				double new_energy = Vlastchange + flip_cost[bit]; //  value if Q[k] bit is flipped
				if (new_energy > best_energy) {
					brk         = true;
					last_bit    = bit;
					new_energy  = evaluate_1bit(Vlastchange, bit, solution, qubo_size, qubo, flip_cost); // flip the bit and fix tables
					Vlastchange = local_search_1bit(new_energy, solution, qubo_size, qubo, flip_cost, t); // local search to polish the change
					val_index_sort_ns(index, flip_cost, qubo_size); // update index array of sorted values, don't shuffle index
					best_energy = Vlastchange;

					for (uint i = 0; i < qubo_size; i++) best[i] = solution[i]; // copy the best solution so far

					if (target_set) {
						if (Vlastchange >= (sign * target)) {
							break;
						}
					}
					howFar = ((double)(iter_max - (*t)) / (double)thisIter);
					if (Verbose_ > 3) {
						printf("Tabu new best %lf ,K=%d,iteration = %lld, %lf, %d\n",
							best_energy * sign, last_bit,(long long) (*t), howFar, brk );
					}
					if ( howFar < 0.80  && numIncrease > 0 ) {
						if (Verbose_ > 3) {
							printf("Increase Itermax %lld, %lld\n", (long long) iter_max, 
                                    (long long) (iter_max + increaseIter));
						}
						iter_max  += increaseIter;
						thisIter += increaseIter;
						numIncrease--;
					}
					break;
				}
				// Q vector unchanged
				if (new_energy > neighbour_best) { // check for improved neighbor solution
					last_bit = bit;   // record position
					neighbour_best = new_energy;   // record neighbor solution value
				}
			}
		}

		if (target_set) {
			if (Vlastchange >= (sign * target)) {
				break;
			}
		}
		if ( !brk ) { // this is the fall thru case and we havent tripped interior If V> VS test so flip Q[K]
			Vlastchange = evaluate_1bit(Vlastchange, last_bit, solution, qubo_size, qubo, flip_cost);
		}

		uint i;
		for (i = 0; i < qubo_size; i++) TabuK [i] =  MAX(0, TabuK[i] - 1);

		// add some asymetry
		if (solution[qubo_size-1] == 0) {
			TabuK[last_bit] = nTabu + 1;
		} else {
			TabuK[last_bit] = nTabu - 1;
		}
	}

 	// copy over the best solution
	for (uint i = 0; i < qubo_size; i++) solution[i] = best[i];

	// ok, we are leaving Tabu, we can do a for sure clean up run of evaluate, to be sure we
	// return the true evaluation of the function (given that we only do this a handfull of times)
	double final_energy = evaluate(solution, qubo_size, qubo, flip_cost);

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
void reduce(int *Icompress, double **qubo, uint sub_qubo_size, uint qubo_size,
	double **sub_qubo, int8_t *solution, int8_t *sub_solution)
{
	// clean out the subMatrix
	for (uint i = 0; i < sub_qubo_size; i++) { // for each column
		for (uint j = 0; j < sub_qubo_size; j++)
			sub_qubo[i][j] = 0.0; // for each row
	}

	// fill the subMatrix
	for (uint i = 0; i < sub_qubo_size; i++) { // for each column
		sub_solution[i] = solution[Icompress[i]];
		for (uint j = i; j < sub_qubo_size; j++) { // copy row
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

		// Go over all other (non-extracted) variable
		// from the highest until we reach the current variable
		for (int j = qubo_size - 1; j > variable; j--) {
			if ( j == Icompress[ji] ) {
				// Found a sub_qubo element, skip it, watch for the next one
				ji--;
			} else {
				clamp += qubo[variable][j] * solution[j];
			}
		}

		// Go over all other (non-extracted) variable
		// from zero until we reach the current variable
		ji = 0;
		for (int j = 0; j < variable + 1; j++) {
			if ( j == Icompress[ji] ) {
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
// @param flip_cost is the impact vector (the chainge in objective function value that results from flipping each bit)
// @param t is the number of candidate bit flips performed in the entire algorithm so far
// @param TabuK is stores the list of tabu moves
// @param index is the order in which to perform candidate bit flips (determined by Qval).
double solv_submatrix(int8_t *solution, int8_t *best, uint qubo_size, double **qubo,
	double *flip_cost, int64_t *t, int *TabuK, int *index)
{
	int64_t iter_max = (*t) + (int64_t)MAX((int64_t)3000, (int64_t)20000 * (int64_t)qubo_size);

	return tabu_search(solution, best, qubo_size, qubo, flip_cost,
		t, iter_max, TabuK, Target_, false, index);
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
// When none of the variables any in the subregions change, a new solution
// is chosen based on randomizing those variables.
//
// After Pchk = 8 iterations with no improvement, the algorithm is
//   completely restarted with a new random solution.
// After nRepeats iterations with no improvement, the algorithm terminates.
//
// @param qubo The QUBO matrix to be solved
// @param qubo_size is the number of variables in the QUBO matrix
// @param nRepeats is the number of iterations without improvement before giving up
void solve(double **qubo, const int qubo_size, int nRepeats)
{
	double    *flip_cost, energy;
	int       *TabuK, *index, *index_s, start_;
	int8_t    *solution, *tabu_solution;
	long      numPartCalls = 0;
	int64_t t = 0,  IterMax;

	start_ = clock();
	t      = 0;

	// Get some memory for the larger val matrix to solve
	if (GETMEM(solution, int8_t, qubo_size) == NULL) BADMALLOC
	if (GETMEM(tabu_solution, int8_t, qubo_size) == NULL) BADMALLOC
	if (GETMEM(flip_cost, double, qubo_size) == NULL) BADMALLOC
	if (GETMEM(index, int, qubo_size) == NULL) BADMALLOC
	if (GETMEM(TabuK, int, qubo_size) == NULL) BADMALLOC

	// get some memory for storing and shorting Q bit vectors
	const int QLEN=20;
	int8_t  **solution_list;
	double *energy_list;
	int    *solution_counts, *Qindex, NU = 0; // NU = current count of items

	solution_list = (int8_t**)malloc2D(QLEN + 1, qubo_size, sizeof(int8_t));
	if (GETMEM(energy_list, double, QLEN + 1) == NULL) BADMALLOC
	if (GETMEM(solution_counts, int, QLEN + 1) == NULL) BADMALLOC
	if (GETMEM(Qindex, int, QLEN + 1) == NULL) BADMALLOC

	for (int i = 0; i < QLEN + 1; i++) {
		energy_list[i]     = BIGNEGFP;
		solution_counts[i] = 0;
		for (int j = 0; j < qubo_size; j++ ) {
			solution_list[i][j] = 0;
		}
	}

	// get some memory for reduced sub matrices
	int8_t  *sub_solution, *Qt_s, *Qbest;
	double best_energy, *sub_flip_cost, **sub_qubo;
	int    *TabuK_s, *Icompress;

	sub_qubo = (double**)malloc2D(qubo_size, qubo_size, sizeof(double));
	if (GETMEM(Icompress, int, qubo_size) == NULL) BADMALLOC
	if (GETMEM(Qbest, int8_t, qubo_size) == NULL) BADMALLOC
	if (GETMEM(TabuK_s, int, SubMatrix_) == NULL) BADMALLOC
	if (GETMEM(sub_solution, int8_t, SubMatrix_) == NULL) BADMALLOC
	if (GETMEM(Qt_s, int8_t, SubMatrix_) == NULL) BADMALLOC
	if (GETMEM(sub_flip_cost, double, SubMatrix_) == NULL) BADMALLOC
	if (GETMEM(index_s, int, SubMatrix_) == NULL) BADMALLOC

	// initialize and set some tuning parameters
	//
	const int Progress_check = 12;                 // number of non progresive passes thru main loop before reset
	const float SubMatrix_span = 0.214; // percent of the total size will be covered by the subMatrix pass
	const int64_t InitialTabuPass_factor=6500; // initial pass factor for tabu iterations
	const int64_t TabuPass_factor=1600.;        // iterative pass factor for tabu iterations

	const int MaxNodes_sub = MAX(SubMatrix_ + 1, SubMatrix_span * qubo_size);
	const int subMatrix    = SubMatrix_;
	const int l_max        = MIN(qubo_size - SubMatrix_, MaxNodes_sub);

	randomize_solution(tabu_solution, qubo_size);
	for (int i = 0; i < qubo_size; i++) {
		index[i]    = i;   // initial index to 0,1,2,...qubo_size
		TabuK[i]    = 0;   // initial TabuK to nothing tabu
		solution[i] = 0;
	}
	for (int i = 0; i < SubMatrix_; i++) {
		index_s[i] = i; // initial index to 0,1,2,...SubMartix_
		sub_solution[i] = 0;
		Qt_s[i]    = tabu_solution[i];
	}

	int    l = 0, DwaveQubo = 0;
	double sign = findMax_ ? 1.0 : -1.0;

	// run initial Tabu Search to establish backbone
	IterMax = t + (int64_t)MAX((int64_t)400, InitialTabuPass_factor * (int64_t)qubo_size);
	if (Verbose_ > 2) {
		DLT; printf(" Starting Full initial Tabu\n");
	}
	energy = tabu_search(solution, tabu_solution, qubo_size, qubo, flip_cost,
		&t, IterMax, TabuK, Target_, TargetSet_, index);

	// save best result
	best_energy = energy;
	NU    = manage_solutions(solution, solution_list, energy, energy_list, solution_counts, Qindex, QLEN, qubo_size);
	for (int i = 0; i < qubo_size; i++) Qbest[i] = solution[i];
	val_index_sort(index, flip_cost, qubo_size); // create index array of sorted values
	if ( Verbose_ > 0 ) {
		print_output(qubo_size, solution, numPartCalls, best_energy * sign,
			(double)(clock() - start_) / CLOCKS_PER_SEC);
	}
	if (Verbose_ > 1) {
		DLT; printf(" V Starting outer loop =%lf iterations %lld\n", best_energy * sign, (long long) t);
	}

	// starting main search loop Partition ( run parts on tabu or Dwave ) --> Tabu rinse and repeat
	short RepeatPass = 0, NoProgress = 0;
	short ContinueWhile = false;
	if ( TargetSet_ ) {
		if ( best_energy >= (sign * Target_) ) {
			ContinueWhile = false;
		} else {
			ContinueWhile = true;
		}
	} else {
		if ( RepeatPass < nRepeats) {
			ContinueWhile = true;
		} else {
			ContinueWhile = false;
		}
	}
	if ( qubo_size < 20 || subMatrix > qubo_size ) {
		ContinueWhile = false; // trivial problem or Submatrix won't contribute
	}

	DwaveQubo = 0;

	// outer loop begin
	while ( ContinueWhile ) {
		// use the first "remove" index values to remove rows and columns from new matrix
		// initial TabuK to nothing tabu sub_solution[i] = Q[i];
		// create compression bit vector

		val_index_sort(index, flip_cost, qubo_size); // Create index array of sorted values
		if (Verbose_ > 1) printf("Reduced submatrix solution l = 0; %d, subMatrix size = %d\n",
		                         l_max, subMatrix);

		// begin submatrix passes
		if ( NoProgress % Progress_check == (Progress_check - 1) ) { // every Progress_check (th) loop without progess
			// reset completely
			randomize_solution(solution, qubo_size);
			for (int i = 0; i < qubo_size; i++) {
				TabuK[i] = 0;
			}
			if (Verbose_ > 1) {
				DLT; printf(" \n\n Reset Q and start over Repeat = %d/%d, as no progress is exhausted %d %d\n\n\n",
				            nRepeats, RepeatPass, NoProgress, NoProgress % Progress_check);
			}
		} else {
			int change=false;
			for (l = 0; l < l_max; l += subMatrix) {
				if (Verbose_ > 3) printf("Submatrix starting at backbone %d\n", l);

				for (int i = l, j = 0; i < l + subMatrix; i++) {
					Icompress[j++] = index[i]; // create compression index
				}
				for (int j = 0; j < subMatrix; j++) {
					TabuK[Icompress[j]] = 0;
				}
				index_sort(Icompress, subMatrix, true); // sort it for effective reduction

				// coarsen and reduce the problem
				reduce(Icompress, qubo, subMatrix, qubo_size, sub_qubo, solution, sub_solution);
				if (Verbose_ > 3) {
					printf("Bits as set before solve ");
					for (int j = 0; j < subMatrix; j++) printf("%d", solution[Icompress[j]]);
					if (Verbose_ > 3) printf("\n");
				}
				if ( UseDwave_ ) {
					dw_solver(sub_qubo, subMatrix, sub_solution);
				} else {
					solv_submatrix(sub_solution, Qt_s, subMatrix, sub_qubo, sub_flip_cost, &t, TabuK_s, index_s);
				}
				for (int j = 0; j < subMatrix; j++) {
					int bit = Icompress[j];
					if (solution[bit] != sub_solution[j] ) change=true;
					solution[bit] = sub_solution[j];
				}
				if (Verbose_ > 3) {
					printf("Bits set after solve     ");
					for (int j = 0; j < subMatrix; j++) printf("%d", solution[Icompress[j]]);
					printf("\n");
				}
				numPartCalls++;
				DwaveQubo++;
			}

			// submatrix search did not produce any new values, so randomize those bits
			if (!change) {
				randomize_solution_by_index(solution, l, index );
				if (Verbose_ > 3) {
					printf(" Submatrix search did not produce any new values, so randomize %d bits\n",l);
				}
			}

			// completed submatrix passes
			if ( Verbose_ > 1 ) printf("\n");
		}
		if ( Verbose_ > 1 ) {
			DLT; printf(" ***Full Tabu  -- after partition pass \n");
		}
		// FULL TABU run here

		IterMax = t + TabuPass_factor * (int64_t)qubo_size;
		val_index_sort(index, flip_cost, qubo_size); // Create index array of sorted values
		energy = tabu_search(solution, tabu_solution, qubo_size, qubo, flip_cost,
			&t, IterMax, TabuK, Target_, TargetSet_ ,index);
		val_index_sort(index, flip_cost, qubo_size); // Create index array of sorted values

		if ( Verbose_ > 1 ) {
			DLT; printf("Latest answer  %4.5f iterations =%lld\n", energy * sign,(long long) t);
		}

		NU = manage_solutions(solution, solution_list, energy, energy_list, solution_counts, Qindex, QLEN, qubo_size);
		int repeats;
		repeats = NU % 10;
		NU      = NU - repeats;
		if ( NU == 10 ) { // better solution
			best_energy = energy;
			for (int i = 0; i < qubo_size; i++) Qbest[i] = solution[i];
			RepeatPass = 0;

			if ( Verbose_ > 1 ) {
				DLT; printf(" IMPROVEMENT; RepeatPass set to %d\n", RepeatPass);
			}
			if ( Verbose_ > 0 ) {
				print_output(qubo_size, Qbest, numPartCalls, best_energy * sign, (double)(clock() - start_) / CLOCKS_PER_SEC);
			}
		} else if ( NU == 30 || NU == 20  ) { // equal solution, but how it is different?
			RepeatPass++;
			if (is_array_equal(solution, Qbest, qubo_size)) {
				NoProgress++;
			} else {
				for (int i = 0; i < qubo_size; i++) Qbest[i] = solution[i];
				if ( repeats == 1 && Verbose_ > 0) { // we haven't printed this out before
					print_output(qubo_size, Qbest, numPartCalls, best_energy * sign, (double)(clock() - start_) / CLOCKS_PER_SEC);
				}
			}
		} else { // not as good as our best so far
			RepeatPass++;
			NoProgress++;
			if ( Verbose_ > 1) {
				printf("NO improvement RepeatPass =%d\n", RepeatPass);
			}
		}

		if ( Verbose_ > 1) {
			DLT; printf("V Best outer loop =%lf iterations %lld\n", best_energy * sign,(long long) t);
		}

		// check on, if to continue the outer loop
		if ( TargetSet_ ) {
			if ( best_energy >= (sign * Target_) ) {
				ContinueWhile = false;
			} else {
				ContinueWhile = true;
			}
		} else {
			if ( RepeatPass < nRepeats) {
				ContinueWhile = true;
			} else {
				ContinueWhile = false;
			}
		}

		// timeout test
		if ( (double)(clock() - start_) / CLOCKS_PER_SEC >= Time_ ) {
			ContinueWhile = false;
		}
	} // end of outer loop

	// all done print results if needed and free allocated arrays
	if (WriteMatrix_)
		print_solution_and_qubo(solution, qubo_size, qubo);

	if ( Verbose_ == 0 )
		print_output(qubo_size, Qbest, numPartCalls, best_energy * sign, (double)(clock() - start_) / CLOCKS_PER_SEC);

	free(solution); free(tabu_solution); free(flip_cost);
	free(index); free(TabuK); free(energy_list); free(solution_counts); free(Qindex); free(Icompress);
	free(Qbest); free(TabuK_s); free(sub_solution); free(Qt_s); free(qubo); free(sub_qubo);
	free(sub_flip_cost); free (index_s); free(solution_list);
	return;
}
