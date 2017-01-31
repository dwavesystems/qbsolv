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

// this function evaluates the objective function for a given Q, It is called when the search
// is starting over, such as after a projection in outer loop of solver
// flip_cost = the change if a Q bit is flipped
// Row and Col are used for fast flip_cost updates in other functions when the bit is flipped
double evaluate(short *Q, int maxNodes, double **qubo, double *flip_cost, double *Row, double *Col)
{
	int    i, j;
	double result = 0.0;

	for (i = 0; i < maxNodes; i++) {
		Row[i] = 0.0; Col[i] = 0.0;

		for ( j = i + 1; j < maxNodes; j++) {
			Row[i] += qubo[i][j] * (double)Q[j];
		}
		for ( j = 0; j < i; j++) {
			Col[i] += qubo[j][i] * (double)Q[j];
		}

		if ( Q[i] == 1 ) {
			result +=    Row[i] +          qubo[i][i];
			flip_cost[i] = -( Row[i] + Col[i] + qubo[i][i] );
		} else {
			flip_cost[i] =  ( Row[i] + Col[i] + qubo[i][i] );
		}
	}
	return result;
}

// this function evaluates change in the objective function and flip_cost for a given Q, and
// flip_cost = the change if a Q single bit is flipped, Row and Col are updated so this function
// can be optimal
double evaluate_1bit(double V_old, int bit, short *Q, int maxNodes, double **qubo, double *flip_cost, double *Row, double *Col)
{
	int i, j;

	double result;

	result = V_old + flip_cost[bit];

	Q[bit] = 1 - Q[bit];

	if (Q[bit] == 0 ) {
		// then it was a 1 before and we have to reduce all the rows from 0 to bit by qubo[bit][j]*Q[j]
		// from bit+1 to maxNodes reduce cols by qubo[j][bit]*Q[j]
		for (i = 0; i < bit; i++) {
			Row[i] -= qubo[i][bit];
		}
		for (i = bit + 1; i < maxNodes; i++) {
			Col[i] -= qubo[bit][i];
		}

	} else{
		// then it was a 0 before and we have to increase all the rows from 0 to bit by qubo[bit][j]*Q[j]
		// from bit+1 to maxNodes increase cols by qubo[j][bit]*Q[j]
		for (i = 0; i < bit; i++) {
			Row[i] += qubo[i][bit];
		}
		for (i = bit + 1; i < maxNodes; i++) {
			Col[i] += qubo[bit][i];
		}
	}
	// Col[bit] Row[bit] need recalculation
	i = bit; { // recalculate
		Col[i] = 0.0;
		for ( j = 0; j < i; j++) {
			Col[i] += qubo[j][i] * (double)Q[j];
		}
		Row[i] = 0.0;
		for ( j = i + 1; j < maxNodes; j++) {
			Row[i] += qubo[i][j] * (double)Q[j];
		}
	}
	for (i = 0; i < maxNodes; i++) {
		if ( Q[i] == 1 ) {
			flip_cost[i] = -( Row[i] + Col[i] + qubo[i][i] );
		} else {
			flip_cost[i] =  ( Row[i] + Col[i] + qubo[i][i] );
		}
	}

	return result;
}

// this function performs a local Max search but doesn't require an initial Evaluation
// improving Q and returning the last Evaluated value
double local_search_1bit(double V, short *Q, int maxNodes, double **qubo, double *flip_cost, double *Row, double *Col, long long *t)
{
	short improve;
	int   k, kk;
	int   kkstr = 0, kkend = maxNodes, kkinc;
	int   index[maxNodes];

	for (k = 0; k < maxNodes; k++) {
		index[k] = k;
	}

	// initial evaluate (flip_cost,Row,Col all primed)  needed before evaluate_1bit can be used
	improve = true;
	while (improve) {
		improve = false;
		if ( kkstr == 0 ) { // sweep top to bottom
			shuffle_index(index, maxNodes);
			kkstr = maxNodes - 1; kkinc = -1; kkend = 0;
		}else{ // sweep bottom to top
			kkstr = 0; kkinc = 1; kkend = maxNodes; // got thru it backwards then reshuffle
		}
		for (kk = kkstr; kk != kkend; kk = kk + kkinc) {
			k = index[kk];
			(*t)++;
			if ( V + flip_cost[k] > V ) {
				V       = evaluate_1bit(V, k, Q, maxNodes, qubo, flip_cost, Row, Col);
				improve = true;
			}
		}
	}
	return V;
}

// this function performance a local Max search improving Q and returning the last Evaluated value
double local_search(short *Q, int maxNodes, double **qubo, double *flip_cost, double *Row, double *Col, long long *t)
{
	double V;

	// initial evaluate needed before evaluate_1bit can be used
	V = evaluate(Q, maxNodes, qubo, flip_cost, Row, Col);
	V = local_search_1bit(V, Q, maxNodes, qubo, flip_cost, Row, Col, t); // local search to polish the change
	return V;
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
// @param[in,out] Q inputs a current solution and returns the best solution found
// @param[out] Qt stores the best solution found during the algorithm
// @param maxNodes is the number of variables in the QUBO matrix
// @param qubo is the QUBO matrix to be solved
// @param flip_cost is the impact vector (the chainge in objective function value that results from flipping each bit)
// @param Row Contributions to flip_cost coming from the rows on qubo
// @param Col Contributions to flip_cost coming from the columns on qubo
// @param t is the number of candidate bit flips performed in the entire algorithm so far
// @param IterMax is the maximum size of t allowed before terminating
// @param TabuK is stores the list of tabu moves
// @param Target Halt if this energy is reached and TargetSet is true
// @param TargetSet Do we have a target energy at which to terminate
// @param index is the order in which to perform candidate bit flips (determined by flip_cost).
double tabu_search(short *Q, short *Qt, int maxNodes, double **qubo, double *flip_cost,
 double *Row, double *Col, long long *t, long long IterMax, int *TabuK,
            double Target, int TargetSet, int *index)
{
	int       i, k, K;     // iteration working vars
	int       brk;         // flag to mark a break and not a fall thru of the loop
	double    Vs;          // best solution so far
	double    Vss;         // best solution in neighbour
	double    V;           // working solution variable
	double    Vlastchange; // working solution variable
	int       nTabu;
	double    fmin;
	long long thisIter;
	long long increaseIter;
	int       numIncrease = 900;
	double    howFar;

	// setup nTabu
	// these nTabu numbers might need to be adjusted to work correctly
	if ( Tlist_ != -1 ) {
		nTabu = MIN(Tlist_, maxNodes + 1 ); // tabu use set tenure
	} else {
		if ( maxNodes < 100 ) {
			nTabu = 10;
		}
		else if ( maxNodes < 250) {
			nTabu = 12;
		}
		else if ( maxNodes < 500) {
			nTabu = 13;
		}
		else if ( maxNodes < 1000) {
			nTabu = 21;
		}
		else if ( maxNodes < 2500) {
			nTabu = 29;
		}
		else if ( maxNodes < 8000) {
			nTabu = 34;
		}
		else { // maxNodes >= 8000
			nTabu = 35;
		}
	}

	if ( findMax_ ) {
		fmin = 1.0;
	} else {
		fmin = -1.0;
	}

	Vs = local_search(Q, maxNodes, qubo, flip_cost, Row, Col, t);
	val_index_sort(index, flip_cost, maxNodes); // Create index array of sorted values
	thisIter     = IterMax - (*t);
	increaseIter = thisIter / 2;
	Vlastchange  = Vs; V = Vs;
	K            = 0;
	for (i = 0; i < maxNodes; i++) Qt[i] = Q[i]; // copy the best solution so far
	for (i = 0; i < maxNodes; i++) TabuK[i] = 0; // zero out the Tabu vector

	int kk, kkstr = 0, kkend = maxNodes, kkinc;
	while (*t < IterMax) {
		Vss = BIGNEGFP; // initialized most negative number
		brk = false;
		if ( kkstr == 0 ) { // sweep top to bottom
			kkstr = maxNodes - 1; kkinc = -1; kkend = 0;
		} else { // sweep bottom to top
			kkstr = 0; kkinc = 1; kkend = maxNodes;
		}

		for (kk = kkstr; kk != kkend; kk = kk + kkinc) {
			k = index[kk];
			if (TabuK[k] != (short)0 ) continue;
			{
				(*t)++;
				V = Vlastchange + flip_cost[k]; //  value if Q[k] bit is flipped
				if (  V > Vs  ) {
					brk         = true;
					K           = k;
					V           = evaluate_1bit(Vlastchange, k, Q, maxNodes, qubo, flip_cost, Row, Col); // flip the bit and fix tables
					Vlastchange = local_search_1bit(V, Q, maxNodes, qubo, flip_cost, Row, Col, t); // local search to polish the change
					val_index_sort_ns(index, flip_cost, maxNodes); // update index array of sorted values, don't shuffle index
					Vs = Vlastchange;
					for (i = 0; i < maxNodes; i++) Qt[i] = Q[i]; // copy the best solution so far

					if ( TargetSet ) {
						if ( Vlastchange >= (fmin * Target) ) {
							break;
						}
					}
					howFar = ( (double)( IterMax - (*t) ) / (double)thisIter  );
					if (Verbose_ > 3) {
						printf("Tabu new best %lf ,K=%d,iteration = %lld, %lf, %d\n", Vs * fmin, K, (*t), howFar, brk );
					}
					if ( howFar < 0.80  && numIncrease > 0 ) {
						if (Verbose_ > 3) {
							printf("Increase Itermax %lld, %lld\n", IterMax, IterMax + increaseIter);
						}
						IterMax  += increaseIter;
						thisIter += increaseIter;
						numIncrease--;
					}
					break;
				}
				// Q vector unchanged
				if (V > Vss) { // check for improved neighbor solution
					K   = k;   // record position
					Vss = V;   // record neighbor solution value
				}
			}
		}

		if ( TargetSet ) {
			if ( Vlastchange >= (fmin * Target) ) {
				break;
			}
		}
		if ( !brk ) { // this is the fall thru case and we havent tripped interior If V> VS test so flip Q[K]
			Vlastchange = evaluate_1bit(Vlastchange, K, Q, maxNodes, qubo, flip_cost, Row, Col);
		}
		for (i = 0; i < maxNodes; i++) TabuK [i] =  MAX(0, TabuK[i] - 1);
		if (Q[i] == 0 ) {
			TabuK[K] = nTabu + 1;
		} else {
			TabuK[K] = nTabu - 1;
		} // add some asymetry
	}

	for (i = 0; i < maxNodes; i++) Q[i] = Qt[i]; // copy over the best solution

	// ok, we are leaving Tabu, we can do a for sure clean up run of evaluate, to be sure we
	// return the true evaluation of the function (given that we only do this a handfull of times)
	V = evaluate(Q, maxNodes, qubo, flip_cost, Row, Col);

	// Create index array of sorted values
	val_index_sort(index, flip_cost, maxNodes);
	return V;
}

// given a bit vector Qcompress, remove the row and column of qubo, nodes and couplers
// also returning the new maxNodes and nCouplers
void reduce(int *Icompress, double **qubo, int subMatrix, int maxNodes, double **val_s, short *Q, short *Q_s)
{
	int i, j; // scratch intergers looping

	// using the Qcompress bit vector reduce the Val matrix

	// clean out the subMatrix
	for (i = 0; i < subMatrix; i++) { // for each column
		for (j = 0; j < subMatrix; j++) val_s[i][j] = 0.0; // for each row
	}
	// fill the subMatrix
	for (i = 0; i < subMatrix; i++) { // for each column
		Q_s[i] = Q[Icompress[i]];
		for (j = i; j < subMatrix; j++) { // copy row
			val_s[i][j] = qubo[Icompress[i]][Icompress[j]];
		}
	}

	// clamping
	double clamp;
	int    rc_s, rc; //rc = row/col (diag) rc_s submatrix version
	int    ji;

	// Go over every variable that we are extracting
	for (rc_s = 0; rc_s < subMatrix; rc_s++) {
		// Get the global index of the current variable
		rc    = Icompress[rc_s];
		clamp = 0;

		// Go over all other (non-extracted) variable
		// from the highest until we reach the current variable
		ji    = subMatrix - 1;
		for ( j = maxNodes - 1; j > rc; j--) {
			if ( j == Icompress[ji] ) {
				ji--;
			} else {
				clamp += qubo[rc][j] * Q[j];
			}
		}

		// Go over all other (non-extracted) variable
		// from zero until we reach the current variable
		ji = 0;
		for ( j = 0; j < rc + 1; j++) {
			if ( j == Icompress[ji] ) {
				ji++;
			} else {
				clamp += qubo[j][rc] * Q[j];
			}
		}
		val_s[rc_s][rc_s] += clamp;
	}
	return;
}

// do the smaller matrix solver here, currently dummied not holding the Dwave part yet
double solv_submatrix(short *Q, short *Qt, int maxNodes, double **qubo, double *flip_cost,
                      double *Row, double *Col, long long *t, int *TabuK, int *index)
{
	long long IterMax = (*t) + (long long)MAX((long long)3000, (long long)20000 * (long long)maxNodes);

	return tabu_search(Q, Qt, maxNodes, qubo, flip_cost, Row, Col, t, IterMax, TabuK, Target_, false, index);
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
// @param maxNodes is the number of variables in the QUBO matrix
// @param nRepeats is the number of iterations without improvement before giving up
void solve(double **qubo, const int maxNodes, int nRepeats)
{
	double    *flip_cost, *Row, *Col, V;
	int       i, j,  *TabuK, *index, *index_s, start_;
	short     *Q, *Qt;
	long      numPartCalls = 0;
	long long t = 0,  IterMax;

	start_ = clock();
	t      = 0;

	// Get some memory for the larger val matrix to solve
	if (GETMEM(Q, short, maxNodes) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Qt, short, maxNodes) == NULL) {
		BADMALLOC
	}
	if (GETMEM(flip_cost, double, maxNodes) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Row, double, maxNodes) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Col, double, maxNodes) == NULL) {
		BADMALLOC
	}
	if (GETMEM(index, int, maxNodes) == NULL) {
		BADMALLOC
	}
	if (GETMEM(TabuK, int, maxNodes) == NULL) {
		BADMALLOC
	}

	// get some memory for storing and shorting Q bit vectors
	const int QLEN=20;
	short  **Qlist;
	double *QVs;
	int    *Qcounts, *Qindex, NU = 0; // NU = current count of items

	Qlist = (short**)malloc2D(QLEN + 1, maxNodes, sizeof(short));
	if (GETMEM(QVs, double, QLEN + 1) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Qcounts, int, QLEN + 1) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Qindex, int, QLEN + 1) == NULL) {
		BADMALLOC
	}
	for (i = 0; i < QLEN + 1; i++) {
		QVs[i]     = BIGNEGFP;
		Qcounts[i] = 0;
		for ( j = 0; j < maxNodes; j++ ) {
			Qlist[i][j] = 0;
		}
	}

	// get some memory for reduced sub matrices
	short  *Q_s, *Qt_s, *Qbest;
	double Vbest, *Qval_s, *Row_s, *Col_s, **val_s;
	int    *TabuK_s, *Icompress;

	val_s = (double**)malloc2D(maxNodes, maxNodes, sizeof(double));
	if (GETMEM(Icompress, int, maxNodes) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Qbest, short, maxNodes) == NULL) {
		BADMALLOC
	}
	if (GETMEM(TabuK_s, int, SubMatrix_) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Q_s, short, SubMatrix_) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Qt_s, short, SubMatrix_) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Qval_s, double, SubMatrix_) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Row_s, double, SubMatrix_) == NULL) {
		BADMALLOC
	}
	if (GETMEM(Col_s, double, SubMatrix_) == NULL) {
		BADMALLOC
	}
	if (GETMEM(index_s, int, SubMatrix_) == NULL) {
		BADMALLOC
	}

	// initialize and set some tuning parameters
	//
	const int Progress_check = 12;                 // number of non progresive passes thru main loop before reset
	const float SubMatrix_span = 0.214; // percent of the total size will be covered by the subMatrix pass
	const long long InitialTabuPass_factor=6500; // initial pass factor for tabu iterations
	const long long TabuPass_factor=1600.;        // iterative pass factor for tabu iterations

	const int MaxNodes_sub = MAX(SubMatrix_ + 1, SubMatrix_span * maxNodes);
	const int subMatrix    = SubMatrix_;
	const int l_max        = MIN(maxNodes - SubMatrix_, MaxNodes_sub);

	set_bit(Qt, maxNodes);
	for (i = 0; i < maxNodes; i++) {
		index[i] = i;   // initial index to 0,1,2,...maxNodes
		TabuK[i] = 0;   // initial TabuK to nothing tabu
		Q[i]     = 0;
	}
	for (i = 0; i < SubMatrix_; i++) {
		index_s[i] = i; // initial index to 0,1,2,...SubMartix_
		Q_s[i]     = 0;
		Qt_s[i]    = Qt[i];
	}

	int    l = 0, DwaveQubo = 0;
	double fmin;

	if ( findMax_ ) {
		fmin = 1.0;
	} else {
		fmin = -1.0;
	}

	// run initial Tabu Search to establish backbone
	IterMax = t + (long long)MAX((long long)400, InitialTabuPass_factor * (long long)maxNodes);
	if (Verbose_ > 2) {
		DLT; printf(" Starting Full initial Tabu\n");
	}
	V = tabu_search(Q, Qt, maxNodes, qubo, flip_cost, Row, Col, &t, IterMax, TabuK, Target_, TargetSet_, index);

	// save best result
	Vbest = V;
	NU    = manage_Q(Q, Qlist, V, QVs, Qcounts, Qindex, QLEN, maxNodes);
	for (i = 0; i < maxNodes; i++) Qbest[i] = Q[i];
	val_index_sort(index, flip_cost, maxNodes); // create index array of sorted values
	if ( Verbose_ > 0 ) {
		print_output(maxNodes, Q, numPartCalls, Vbest * fmin, (double)(clock() - start_) / CLOCKS_PER_SEC);
	}
	if (Verbose_ > 1) {
		DLT; printf(" V Starting outer loop =%lf iterations %lld\n", Vbest * fmin, t);
	}

	// starting main search loop Partition ( run parts on tabu or Dwave ) --> Tabu rinse and repeat
	short RepeatPass = 0, NoProgress = 0;
	short ContinueWhile = false;
	if ( TargetSet_ ) {
		if ( Vbest >= (fmin * Target_) ) {
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
	if ( maxNodes < 20 || subMatrix > maxNodes ) {
		ContinueWhile = false; // trivial problem or Submatrix won't contribute
	}

	DwaveQubo = 0;

	// outer loop begin
	while ( ContinueWhile ) {
		// use the first "remove" index values to remove rows and columns from new matrix
		// initial TabuK to nothing tabu Q_s[i] = Q[i];
		// create compression bit vector

		val_index_sort(index, flip_cost, maxNodes); // Create index array of sorted values
		if (Verbose_ > 1) printf("Reduced submatrix solution l = 0; %d, subMatrix size = %d\n",
		                         l_max, subMatrix);

		// begin submatrix passes
		if ( NoProgress % Progress_check == (Progress_check - 1) ) { // every Progress_check (th) loop without progess
			// reset completely
			set_bit(Q, maxNodes);
			for (i = 0; i < maxNodes; i++) {
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
				j = 0;
				for (i = l; i < l + subMatrix; i++) {
					Icompress[j++] = index[i]; // create compression index
				}
				for (j = 0; j < subMatrix; j++) {
					TabuK[Icompress[j]] = 0;
				}
				index_sort(Icompress, subMatrix, true); // sort it for effective reduction

				// coarsen and reduce the problem
				reduce(Icompress, qubo, subMatrix, maxNodes, val_s, Q, Q_s);
				if (Verbose_ > 3) {
					printf("Bits as set before solve ");
					for (j = 0; j < subMatrix; j++) printf("%d", Q[Icompress[j]]);
					if (Verbose_ > 3) printf("\n");
				}
				if ( UseDwave_ ) {
					dw_solver( val_s, subMatrix, Q_s );
				} else {
					solv_submatrix(Q_s, Qt_s, subMatrix, val_s, Qval_s, Row_s, Col_s, &t, TabuK_s, index_s);
				}
				for (j = 0; j < subMatrix; j++) {
					i    = Icompress[j];
					if ( Q[i] != Q_s[j] ) change=true;
					Q[i] = Q_s[j];
				}
				if (Verbose_ > 3) {
					printf("Bits set after solve     ");
					for (j = 0; j < subMatrix; j++) printf("%d", Q[Icompress[j]]);
					printf("\n");
				}
				numPartCalls++;
				DwaveQubo++;
			}

			// submatrix search did not produce any new values, so randomize those bits
			if (!change) {
				set_bit_index( Q, l, index );
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

		IterMax = t + TabuPass_factor * (long long)maxNodes;
		val_index_sort(index, flip_cost, maxNodes); // Create index array of sorted values
		V = tabu_search(Q, Qt, maxNodes, qubo, flip_cost, Row, Col, &t, IterMax, TabuK, Target_, TargetSet_ ,index);
		val_index_sort(index, flip_cost, maxNodes); // Create index array of sorted values

		if ( Verbose_ > 1 ) {
			DLT; printf("Latest answer  %4.5f iterations =%lld\n", V * fmin, t);
		}

		NU = manage_Q(Q, Qlist, V, QVs, Qcounts, Qindex, QLEN, maxNodes);
		int repeats;
		repeats = NU % 10;
		NU      = NU - repeats;
		if ( NU == 10 ) { // better solution
			Vbest = V;
			for (i = 0; i < maxNodes; i++) Qbest[i] = Q[i];
			RepeatPass = 0;

			if ( Verbose_ > 1 ) {
				DLT; printf(" IMPROVEMENT; RepeatPass set to %d\n", RepeatPass);
			}
			if ( Verbose_ > 0 ) {
				print_output(maxNodes, Qbest, numPartCalls, Vbest * fmin, (double)(clock() - start_) / CLOCKS_PER_SEC);
			}
		} else if ( NU == 30 || NU == 20  ) { // equal solution, but how it is different?
			RepeatPass++;
			if (is_Q_equal(Q, Qbest, maxNodes)) {
				NoProgress++;
			} else {
				for (i = 0; i < maxNodes; i++) Qbest[i] = Q[i];
				if ( repeats == 1 && Verbose_ > 0) { // we haven't printed this out before
					print_output(maxNodes, Qbest, numPartCalls, Vbest * fmin, (double)(clock() - start_) / CLOCKS_PER_SEC);
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
			DLT; printf("V Best outer loop =%lf iterations %lld\n", Vbest * fmin, t);
		}

		// check on, if to continue the outer loop
		if ( TargetSet_ ) {
			if ( Vbest >= (fmin * Target_) ) {
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
		print_V_Q_Qval(Q, maxNodes, qubo);

	if ( Verbose_ == 0 )
		print_output(maxNodes, Qbest, numPartCalls, Vbest * fmin, (double)(clock() - start_) / CLOCKS_PER_SEC);

    free(Q); free(Qt); free(flip_cost); free(Row); free(Col); free(index); free(TabuK); free(QVs); free(Qcounts);
    free(Qindex); free(Icompress); free(Qbest); free(TabuK_s); free(Q_s); free(Qt_s); free(qubo); free(val_s);
    free(Qval_s); free(Row_s); free(Col_s); free (index_s); free(Qlist);
	return;
}
