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
#include <getopt.h>

// define what will be used by "extern" in other functions
//
FILE            *inFile_, *outFile_;
int             maxNodes_, nCouplers_, nNodes_, nRepeats_, findMax_;
int             Verbose_, SubMatrix_, UseDwave_, TargetSet_, RepeatPass_, WriteMatrix_, Tlist_;
char            *outFileNm_, pgmName_[16];
double          **val;
double          Target_, Time_;
struct nodeStr_ *nodes_;
struct nodeStr_ *couplers_;
//  main routine,
//  -read the command line
//  -read the input qubo file
//  -setup the matrix
//  -call the solver
//  //  put in large statics like the -help output
//      and the qubo file format print out
//

int  main( int argc,  char *argv[])
{
/*
 *  Initialize global variables, set up data structures,
 *  process the commandline and the environment, ...
 *  Initialize global variables
 */
	extern char *optarg;
	extern int  optind, optopt, opterr;

	char *inFileName = NULL;

	strcpy(pgmName_, "qbsolv");
	inFile_      = NULL;
	findMax_     = FALSE;
	UseDwave_    = FALSE;
	Verbose_     = 0;
	nRepeats_    = 50; // default setting
	SubMatrix_   = 46; // submatrix default
	WriteMatrix_ = FALSE;
	outFile_     = stdout;
	TargetSet_   = FALSE;
	Time_        = 2592000; // the maximum runtime of the algorithm in seconds before timeout (2592000 = a months worth of seconds)
	Tlist_       = -1; // tabu list length  -1 signials go with defaults
	long seed       = 17932241798878;
	int  errorCount = 0;

	static struct option longopts[] = {
		{ "help",           no_argument,       NULL, 'h'},
		{ "infile",         required_argument, NULL, 'i'},
		{ "outfile",        required_argument, NULL, 'o'},
		{ "verbosityLevel", required_argument, NULL, 'v'},
		{ "version",        no_argument,       NULL, 'V'},
		{ "subproblemSize", required_argument, NULL, 'S'},
		{ "target",         required_argument, NULL, 'T'},
		{ "repeats",        required_argument, NULL, 'n'},
		{ "max",            no_argument,       NULL, 'm'},
		{ "output",         required_argument, NULL, 'o'},
		{ "timeout",        required_argument, NULL, 't'},
		{ "quboFormat",     no_argument,       NULL, 'q'},
		{ "tlist",          required_argument, NULL, 'l'},
		{ "seed",           required_argument, NULL, 'r'},
		{ NULL,             no_argument,       NULL, 0}
	};

	int  opt, option_index = 0;
	char *chx; // used as exit ptr in strto(x) functions

	while ((opt = getopt_long(argc, argv, "Hhi:o:v:VS:T:l:n:wmo:t:qr:", longopts, &option_index)) != -1) {
		switch (opt) {
		case 'H':
		case 'h':
			print_help();
			exit(0);
		case 'i':
			inFileName = optarg;
			if ((inFile_ = fopen(inFileName, "r")) == NULL) {
				fprintf(stderr, "\n\t Error - can't find/open file " "\"%s\"\n\n", optarg);
				exit(9);
			}
			break;
		case 'l':
			Tlist_ = strtol(optarg, &chx, 10); // this sets the length of the tabu list
			break;
		case 'm':
			findMax_ = TRUE; // go for the maximum value otherwise the minimum is found by default
			break;
		case 'n':
			nRepeats_ = strtol(optarg, &chx, 10); // this sets the number of outer loop repeat without improvement
			break;
		case 'v':
			Verbose_ = strtol(optarg, &chx, 10); // this sets the value of the Verbose
			break;
		case 'V':
			fprintf(outFile_, " Version " VERSION " \n Compiled: " __DATE__ ","__TIME__ "\n");
			exit(9);
			break;
		case 'S':
			SubMatrix_ = strtol(optarg, &chx, 10); // this sets the size of the Partitioning Matrix
			if ( SubMatrix_ < 10 ) {
				if ( SubMatrix_ != 0) {
					fprintf(stderr, "\n Error --  SubMatrix must be 0 or greater than 10.  -S %d\n ", SubMatrix_);
					++errorCount;
				}
			}

			if ( SubMatrix_ == 0 ) {
				UseDwave_ = TRUE;
			}
			break;
		case 'T':
			Target_    = strtod(optarg, (char**)NULL); // this sets desired optimal energy
			TargetSet_ = TRUE;
			break;
		case 't':
			Time_ = strtod(optarg, (char**)NULL); // this sets the maximum runtime of the algorithm in seconds
			break;
		case 'o':
			if ((outFile_ = fopen(optarg, "w")) == NULL) {
				fprintf(stderr, "\n\t Error - can't find/write file " "\"%s\"\n\n", optarg);
				exit(9);
			}
			outFileNm_ = optarg;
			break;
		case 'q':
			print_qubo_format();
			exit(0);
			break;
		case 'r':
			seed = strtol(optarg, &chx, 10); // sets the seed value
			break;
		case 'w':
			WriteMatrix_ = TRUE;
			break;
		default: /* '?' or unknown */
			print_help();
			exit(0);
			break;
		}
	}
	srand( seed );

	if (inFile_ == NULL) {
		fprintf(stderr, "\n\t%s error -- no input file (-i option) specified"
		        "\n\n", pgmName_);
		exit(9);
	}

	errorCount = read_qubo(inFileName); // read in the QUBO from file

	if ((errorCount > 0) ) {
		fprintf(stderr, "\n\t%d Input error(s) on file \"%s\"\n\n"
		        "\t%s has been stopped.\n\tThere is a desription "
		        "of the .qubo file format: use %s -q to print it\n\n",
		        errorCount, inFileName, pgmName_, pgmName_);
		exit(1);
	}

	val = (double**)malloc2D(maxNodes_, maxNodes_, sizeof(double) ); // create a 2d double array
	fill_val(val, maxNodes_, nodes_, nNodes_, couplers_, nCouplers_); // move to a 2d array

	if ( UseDwave_ ) {
		dw_init();
	}

	solve(val, maxNodes_);

	if ( UseDwave_ ) {
		dw_close();
	}
	if (Verbose_ > 3) {
		fprintf(outFile_, "\n\t\"qbsolv  -i %s\" (%d nodes, %d couplers) - end-of-job\n\n",
		        inFileName, nNodes_, nCouplers_);
	}
	exit(0);

}

void  print_help( void)
{
	printf("\n\t%s -i infile [-o outfile] [-m] [-T] [-n] [-S SubMatrix] [-w] \n"
	       "\t\t[-h] [-v verbosityLevel] [-V] [-q]\n"
	       "\nDESCRIPTION \n"
	       "\tqbsolv executes a quadratic unconstrained binary optimization \n"
	       "\t(QUBO) problem represented in a file, providing bit-vector \n"
	       "\tresult(s) that minimizes (or optionally, maximizes) the value of \n"
	       "\tthe objective function represented by the QUBO.  The problem is \n"
	       "\trepresented in the QUBO(5) file format and notably is not limited \n"
	       "\tto the size or connectivity pattern of the D-Wave system on which \n"
	       "\tit will be executed. \n"
	       "\t\tThe options are as follows: \n"
	       "\t-i infile \n"
	       "\t\tThe name of the file in which the input QUBO resides.  This \n"
	       "\t\tis a required option. \n"
	       "\t-o outfile \n"
	       "\t\tThis optional argument denotes the name of the file to \n"
	       "\t\twhich the output will be written.  The default is the \n"
	       "\t\tstandard output. \n"
	       "\t-m \n"
	       "\t\tThis optional argument denotes to find the maximum instead \n"
	       "\t\tof the minimum. \n"
	       "\t-T target \n"
	       "\t\tThis optional argument denotes to stop execution when the \n"
	       "\t\ttarget value of the objective function is found. \n"
	       "\t-t timeout \n"
	       "\t\tThis optional argument stops execution when the elapsed \n"
	       "\t\tcpu time equals or exceeds timeout value. Timeout is only checked \n"
	       "\t\tafter completion of the main loop. Other halt values \n"
	       "\t\tsuch as \'target\' and \'repeats\' will halt before \'timeout\'.\n"
	       "\t\tThe default value is %8.1f.\n"
	       "\t-n repeats \n"
	       "\t\tThis optional argument denotes, once a new optimal value is \n"
	       "\t\tfound, to repeat the main loop of the algorithm this number\n"
	       "\t\tof times with no change in optimal value before stopping.  \n"
	       "\t\tThe default value is %d. \n"
	       "\t-S subproblemSize \n"
	       "\t\tThis optional argument indicates the size of the sub-\n"
	       "\t\tproblems into which the QUBO will be decomposed.  A \n"
	       "\t\tsubproblem size of 0 (the default) indicates to use the \n"
	       "\t\tsize known by qbsolv to be most effective for the D-Wave \n"
	       "\t\tsystem targeted for execution.  A subproblem size greater \n"
	       "\t\tthan zero indicates to use the given size and execute with \n"
	       "\t\tqbsolvs internal classical tabu solver rather than using \n"
	       "\t\tthe D-Wave system. \n"
	       "\t-w \n"
	       "\t\tIf present, this optional argument will print the QUBO \n"
	       "\t\tmatrix and result in .csv format. \n"
	       "\t-h \n"
	       "\t\tIf present, this optional argument will print the help or \n"
	       "\t\tusage message for qbsolv and exit without execution. \n"
	       "\t-v verbosityLevel \n"
	       "\t\tThis optional argument denotes the verbosity of output. A \n"
	       "\t\tverbosityLevel of 0 (the default) will output the number of \n"
	       "\t\tbits in the solution, the solution, and the energy of the \n"
	       "\t\tsolution.  A verbosityLevel of 1 will output the same \n"
	       "\t\tinformation for multiple solutions, if found. A \n"
	       "\t\tverbosityLevel of 2 will also output more detailed \n"
	       "\t\tinformation at each step of the algorithm.   \n"
	       "\t-V \n"
	       "\t\tIf present, this optional argument will emit the version \n"
	       "\t\tnumber of the qbsolv program and exit without execution. \n"
	       "\t-q \n"
	       "\t\tIf present, this optional argument triggers printing the \n"
	       "\t\tformat of the QUBO file.\n"
	       "\t-r seed \n"
	       "\t\tUsed to reset the seed for the random number generation \n",
	       pgmName_, Time_, nRepeats_);

	return;
}

void  print_qubo_format( void)
{
	char *quboFormat =
		"\n   A .qubo file contains data which describes an unconstrained\n"
		"quadratic binary optimization problem.  It is an ASCII file comprise"
		"d\nof four types of lines:\n\n"
		"1) Comments - defined by a \"c\" in column 1.  They may appear\n"
		"\tanywhere in the file, and are otherwise ignored.\n\n"
		"2) One program line, which starts with p in the first column.\n"
		"\tThe program line must be the first non-comment line in the file.\n"
		"\tThe program line has six required fields (separated by space(s)),\n"
		"\tas in this example:\n\n"
		"  p   qubo  target   maxNodes   nNodes   nCouplers\n\n"
		"    where:\n"
		"  p          the problem line sentinel\n"
		"  qubo       identifies the file type\n"
		"  target     a string which identifies the topology of the problem\n"
		"\t     and the specific problem type.  For an unconstrained problem,\n"
		"\t     target may be \"0\" or \"unconstrained.\"   For constrained\n"
		"\t     problems, valid strings could include \"chimera128\" or\n"
		"\t     \"chimera512\" (among others).\n"
		"  maxNodes   number of nodes in the topology.\n"
		"  nNodes     number of nodes in the problem (nNodes <= maxNodes).\n"
		"\t     Each node has a unique number and must take a value in the\n"
		"\t     the range {0 - (maxNodes-1)}.  A duplicate node number is an\n"
		"\t     error.  The node numbers need not be in order, and they need\n"
		"\t     not be contiguous.\n"
		"  nCouplers  number of couplers in the problem.  Each coupler is a\n"
		"\t     unique connection between two different nodes.  The maximum\n"
		"\t     number of couplers is (nNodes)^2.  A duplicate coupler is\n"
		"\t     an error.\n\n"
		"3) nNodes clauses.  Each clause is made up of three numbers.  The\n"
		"\tnumbers are separated by one or more blanks.  The first two\n"
		"\tnumbers must be integers and are the number for this node\n"
		"\t(repeated).  The node number must be in {0 , (maxNodes-1)}.\n"
		"\tThe third value is the weight associated with the node, may be\n"
		"\tan integer or float, and can take on any positive or negative\n"
		"\tvalue, or zero.\n\n"
		"4) nCouplers clauses.  Each clause is made up of three numbers.  The\n"
		"\tnumbers are separated by one or more blanks.  The first two\n"
		"\tnumbers must be different integers and are the node numbers\n"
		"\tfor this coupler.  The two values (i and j) must have (i < j).\n"
		"\tEach number must be one of the nNodes valid node numbers (and\n"
		"\tthus in {0, (maxNodes-1)}).  The third value is the strength\n"
		"\tassociated with the coupler, may be an integer or float, and can\n"
		"\ttake on any positive or negative value, but not zero.  Every node\n"
		"\tmust connect with at least one other node (thus must have at least\n"
		"\tone coupler connected to it).\n\n"
		"Here is a simple QUBO file example for an unconstrained QUBO with 4\n"
		"nodes and 6 couplers.  This example is provided to illustrate the\n"
		"elements of a QUBO benchmark file, not to represent a real problem.\n"
		"\n\t\tc\n"
		"\t\tc  This is a sample .qubo file\n"
		"\t\tc  with 4 nodes and 6 couplers\n"
		"\t\tc\n"
		"\t\tp  qubo  0  4  4  6 \n"
		"\t\tc ------------------\n"
		"\t\t0  0   3.4\n"
		"\t\t1  1   4.5\n"
		"\t\t2  2   2.1\n"
		"\t\t3  3   -2.4\n"
		"\t\tc ------------------\n"
		"\t\t0  1   2.2\n"
		"\t\t0  2   3.4\n"
		"\t\t1  2   4.5\n"
		"\t\t0  3   -2\n"
		"\t\t0  2   3.4\n"
		"\t\t1  2   4.5\n"
		"\t\t0  3   -2\n"
		"\t\t1  3   4.5678\n"
		"\t\t2  3   -3.22\n\n";

	printf("%s", quboFormat);
	return;
}
