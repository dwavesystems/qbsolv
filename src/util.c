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
#include "include.h"
#include "extern.h"

//
// create and pointer fill a 2d array of "size" for
// X[rows][cols] addressing. Using only a single malloc
//
void **malloc2D(uint rows, uint cols, uint size)
{
	char      *ptr;
	char      **big_array;
	uintptr_t space, i;

	space     = rows * (sizeof(char *) + (cols * size)); // the total amount of memory required to hold both the matrix and the lookup table
	big_array = (char**)malloc(space);
	if (big_array == NULL) {
		DL;
		printf("\n\t%s error - memory request for X[%d][%d], %ld Mbytes  "
		       "denied\n\n", pgmName_, rows, cols, (long)space / 1024 / 1024 );
		exit(9);
	}

	space = cols * size;                  // space is now equal to the size of a row
	ptr   = (char*)&big_array[rows];      // ie, &big_array[nRows]
	for (i = 0; i < rows; ++i) {          // assign pointer to row, then increment pointer by size of row
		big_array[i] = ptr;
		ptr         += space;
	}
	return (void*)big_array;
}

// this randomly sets the bit vector to 1 or 0
//
void set_bit(short *Q, int nbits)
{
	int  i;

	for (i = 0; i < nbits; i++) {
        Q[i] = rand() % 2;
	}
}

// this randomly sets the bit vector to 1 or 0, with index
//
void set_bit_index(short *Q, int nbits, int *index)
{
	int  i;

	for (i = 0; i < nbits; i++) {
        Q[index[i]] = rand() % 2;
	}
}

// shuffle the index vector before sort
void shuffle_index(int *index, int n)
{
	int j, i;
	int tmp;

	for (i = n - 1; i > 0; i--) {
		j   = rand() % (i + 1);
		tmp = index[i]; index[i] = index[j]; index[j] = tmp; // swap values
	}
}

//  print out the bit vector as row and column, surrounding the Qubo in triangular form  used in the -w option
//
void print_V_Q_Qval(short *Q, int maxNodes, double **val)
{
	int    i, j;
	double fmin;

	if ( findMax_ ) {
		fmin = 1.0;
	} else {
		fmin = -1.0;
	}


	fprintf(outFile_, "ij, "); for (i = 0; i < maxNodes; i++) fprintf(outFile_, ",%d", i); fprintf(outFile_, "\n");
	fprintf(outFile_, "Q,"); for (i = 0; i < maxNodes; i++) fprintf(outFile_, ",%d", Q[i]); fprintf(outFile_, "\n");
	for ( i = 0; i < maxNodes; i++) {
		fprintf(outFile_, "%d,%d,", i, Q[i]);
		for (j = 0; j < i; j++) fprintf(outFile_, ",");
		for (j = i; j < maxNodes; j++) {
			if (val[i][j] != 0.0 ) {
				fprintf(outFile_, "%6.4lf,", (val[i][j] * (double)fmin));
			}else {
				fprintf(outFile_, ",");
			}
		}
		fprintf(outFile_, "\n");
	}
	/*  print out the bit vector as row and column, surrounding the
	 *  Qubo where both the row and col bit is set in triangular form */
	fprintf(outFile_, "  Values that have a Q of 1 ");
	fprintf(outFile_, "ij, "); for (i = 0; i < maxNodes; i++) fprintf(outFile_, ",%d", i); fprintf(outFile_, "\n");
	fprintf(outFile_, "Q,"); for (i = 0; i < maxNodes; i++) fprintf(outFile_, ",%d", Q[i]); fprintf(outFile_, "\n");
	for ( i = 0; i < maxNodes; i++) {
		fprintf(outFile_, "%d,%d,", i, Q[i]);
		for (j = 0; j < i; j++) fprintf(outFile_, ",");
		for (j = i; j < maxNodes; j++) {
			if (((double)Q[i] * Q[j]) * val[i][j] != 0 ) {
				fprintf(outFile_, "%6.4lf,", val[i][j] * (double)fmin * (double)Q[i] * (double)Q[j]);
			}else {
				fprintf(outFile_, ",");
			}
		}
		fprintf(outFile_, "\n");
	}
}

//  This routine performs the standard output for qbsolv
//
void print_output(int maxNodes, short *Q, long numPartCalls, double energy, double seconds)
{
	int i;

	fprintf(outFile_, "%d Number of bits in solution\n", maxNodes);
	for (i = 0; i < maxNodes; i++) {
		fprintf(outFile_, "%d", Q[i]);
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
void fill_val(double **val, int maxNodes, struct nodeStr_ *nodes, int nNodes, struct nodeStr_ *couplers, int nCouplers)
{
	int i, j;  //scratch intergers looping

	for (i = 0; i < maxNodes; i++) {
		for (j = 0; j < maxNodes; j++) {
			val[i][j] = 0.0;
		}
	}
	if (findMax_) {
		for (i = 0; i < nNodes; i++) {
			val[nodes[i].n1][nodes[i].n1] = nodes[i].value;
		}
		for (i = 0; i < nCouplers; i++) {
			val[couplers[i].n1][couplers[i].n2] = couplers[i].value;
		}
	} else {
		for (i = 0; i < nNodes; i++) {
			val[nodes[i].n1][nodes[i].n1] = -nodes[i].value;
		}
		for (i = 0; i < nCouplers; i++) {
			val[couplers[i].n1][couplers[i].n2] = -couplers[i].value;
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
void quick_sort_iterative_index(double val[], int arr[], int n)
{
	int h, l;

	h = n - 1; // last index
	l = 0; // first index
	int *stack; // temp space = n + 1
	// Create an auxiliary stack
	//int stack[ h - l + 1 ];
	if ((GETMEM(stack, int, h - l + 1)) == NULL) {
		BADMALLOC
	}

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
	free(stack);
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

	for (i = 0; i < n; i++) index[i] = i;
	shuffle_index(index, n);
	quick_sort_iterative_index(val, index, n);
	// check code:
	// for (i=0;i<n-1;i++) { if (val[index[i]]<val[index[i+1]]) { DL; exit(9); } }
	return;
}

void val_index_sort_ns(int *index, double *val, int n)
{
	int i;

	// Assure that the index array covers val[] completely
	for (i = 0; i < n; i++) index[i] = i;
	quick_sort_iterative_index(val, index, n);
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
	else if (x > y) return -1;   // return 1 if you want ascending, -1 if you want descending order.
	return 0;
}

//
// sort an index array
//
void index_sort(int *index, int n, short forward)
{
	if (forward) {
		qsort(index, n, sizeof *index, &compare_intsAsc);
		// check code:for (j=0;j<n-1;j++) { if (index[j]>index[j+1]) { DL; exit(9); }
	} else {
		qsort(index, n, sizeof *index, &compare_intsDes);
		// check code:for (j=0;j<n-1;j++) { if (index[j]<index[j+1]) { DL; exit(9); }
	}
}

// compares two vectors, bit by bit
int is_Q_equal( short *Qnow, short *Qcompare, int nbits)
{
	int i;

	for (i = 0; i < nbits; i++) {
		if ( Qnow[i] != Qcompare[i] ) {
			return false;
		}
	}
	return true;
}

int manage_Q( short *Qnow, short **Qlist, double Vnow, double *QVs, int *Qcounts, int *Qindex, int nMax, int nbits)
{
	// Qnow is the Q vector being looked at
	// Qlist is the 2d array of Q vectors being stored
	// Vnow is the energy of the Q vector being looked at
	// QVs is the 1d array of energies corresponding to Qlists
	// Qcounts is the 1d array of hits on the corresponding Qlists
	// Qindex is the order of Qlist based upon energies
	// nMax is size of the arrays (Qlist,Qvs...)
	// if Qnow is unique, and is better than or equal to the worst Q add it to Qlist
	// if Qnow is not unique ( equal energy )  increment number of times found

	// return codes
	enum
	{
		NOTHING = 0,                   // nothing new, do nothing
		NEW_HIGH_ENERGY_UNIQUE_Q = 10, // Q is unique, highest new energy
		DUPLICATE_HIGHEST_ENERGY = 20, // two cases, Q is unique, duplicate energy
		                               //            Q is duplicate, highest energy, add the number of repeats to this value
		DUPLICATE_ENERGY = 30,         // two cases, Q is unique, highest energy
		                               //            Q is duplicate, not highest, add the number of repeats to this value
		NEW_ENERGY_UNIQUE_Q = 40       // Q is unique, new highest energy
	};

	int i, j, I, iL;

	val_index_sort_ns(Qindex, QVs, nMax); // index array of sorted energies

	// new high value
	if (Vnow > QVs[Qindex[0]] ) {
		I = nMax - 1; // we will add it to the space to an empty queue, Sort will fix it later

		// save the bits to Qlist first entry
		for ( i = 0; i < nbits; i++ ) {
			Qlist[Qindex[I]][i] = Qnow[i];
		}

		QVs[Qindex[I]]     = Vnow;
		Qcounts[Qindex[I]] = 1;

		// index array of sorted energies
		val_index_sort_ns(Qindex, QVs, nMax);

		// we have added this Qnow to the collective, might have overwritten an old one
		return NEW_HIGH_ENERGY_UNIQUE_Q;
	}

	// list energies are all higher than this, do nothing
	if (Vnow < QVs[Qindex[nMax - 1]] ) {
		return NOTHING;
	}

	// new energy is in the range of our list
	// search thru the list to see if there is an equal energy
	for (i = 0; i < nMax; i++) {
		if ( Vnow == QVs[Qindex[i]] ) {

			j = i; // now have a common energy, but it could be that we have a different Q

			// look thru all Q's of common energy (they are ordered)
			while (j < nMax && QVs[Qindex[j]] == Vnow) {
				if ( is_Q_equal( &Qlist[Qindex[j]][0], Qnow, nbits)) {
					// simply mark this Q and energy as a duplicate find
					Qcounts[Qindex[j]]++;

					if ( Vnow == QVs[Qindex[0]] ) {
						// duplicate energy matching another Q and equal to best energy
						return DUPLICATE_HIGHEST_ENERGY + Qcounts[Qindex[j]];
					} else {
						// duplicate energy matching older lower energy Q
						return DUPLICATE_HIGHEST_ENERGY;
					}
				}
				j++;
			}

			// fallen thru equal energies so we need to add it to the list
			j          = Qindex[nMax - 1]; // add it to the worst energy position ( prefilled with worst possible value )
			QVs[j]     = Vnow; // save energy
			Qcounts[j] = 1; // set number of hits as this is a first

			// save the bits to Qlist
			for ( i = 0; i < nbits; i++ ) {
				Qlist[j][i] = Qnow[i];
			}

			// Create index array of sorted energies
			val_index_sort_ns(Qindex, QVs, nMax);
			if ( Vnow == QVs[Qindex[0]] ) {
				// duplicate highest energy unique Q and equal to best energy
				return DUPLICATE_ENERGY;
			} else {
				// duplicate energy matching older lower energy Q
				return DUPLICATE_ENERGY + Qcounts[j];
			}
		}

		// we have spilled off the list of energies and need to add this one
		else if ( Vnow < QVs[Qindex[i]] ) {
			j          = Qindex[nMax - 1]; // add it to the worst energy position as it is unique but within the list
			QVs[j]     = Vnow; // save energy
			Qcounts[j] = 1; // set number of hits as this is a first

			// save the bits to Qlist
			for ( i = 0; i < nbits; i++ ) {
				Qlist[j][i] = Qnow[i];
			}

			// create index array of sorted energies
			val_index_sort_ns(Qindex, QVs, nMax);
			return NEW_ENERGY_UNIQUE_Q;
		}
	}

	for ( iL = 0; iL < nMax; iL++)
		printf(" %d %d %lf %d \n", Qindex[iL], iL, QVs[Qindex[iL]], Qcounts[Qindex[iL]]);

	exit(9);
}

// write qubo file to *filename
void write_qubo(double **val, int nMax, const char *filename)
{
	FILE *file;

	if ((file = fopen(filename, "w")) == 0) {
		DL; printf(" failed to open %s\n", filename); exit(9);
	}
	int nNodes = 0, nCouplers = 0, i, j;

	// count the non-zero couplers and nodes
	for ( i = 0; i < nMax; i++) {
		if ( val[i][i] != 0.0 ) nNodes++;
		for (j = i + 1; j < nMax; j++ ) {
			if ( val[i][j] != 0.0 ) nCouplers++;
		}
	}
	fprintf(file, "p qubo 0 %d %d %d\n", nMax, nNodes, nCouplers);
	for ( i = 0; i < nMax; i++) {
		if ( val[i][i] != 0 ) fprintf(file, "%d %d %lf\n", i, i, val[i][i]);
	}
	for ( i = 0; i < nMax; i++) {
		for (j = i + 1; j < nMax; j++ ) {
			if ( val[i][j] != 0.0 ) fprintf(file, "%d %d %lf\n", i, j, val[i][j]);
		}
	}
	fclose(file);
}
