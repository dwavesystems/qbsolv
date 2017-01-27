README.txt file for qbsolv 

This directory contains the following files and directories:
- README.txt: 		this file
- License:	        A copy of the Apache License Version 2.0
- src:		        Source directory of the qbsolv program with Makefile
			that creates a binary read and solve "QUBO" files
- doc:			OpenOffice, PDF, and HTML versions of the man page,
			and instructions for how to change them
- example:		Directory of example(s) application(s) using qbsolv
			as a solver
- tests:		Directory of scripts and qubo(s) to test qbsolv
- contrib.txt:		Instructions for potential contributors

qbsolv (qb for qubo, solv for solve )

	qbsolv -i infile [-o outfile] [-m] [-T] [-n] [-S SubMatrix] [-w] 
		[-h] [-v verbosityLevel] [-V] [-q]

DESCRIPTION 
	qbsolv executes a quadratic unconstrained binary optimization 
	(QUBO) problem represented in a file, providing bit-vector 
	result(s) that minimizes (or optionally, maximizes) the value of 
	the objective function represented by the QUBO.  The problem is 
	represented in the QUBO(5) file format and notably is not limited 
	to the size or connectivity pattern of the D-Wave system on which 
	it will be executed. 
		The options are as follows: 
	-i infile 
		The name of the file in which the input QUBO resides.  This 
		is a required option. 
	-o outfile 
		This optional argument denotes the name of the file to 
		which the output will be written.  The default is the 
		standard output. 
	-m 
		This optional argument denotes to find the maximum instead 
		of the minimum. 
	-T target 
		This optional argument denotes to stop execution when the 
		target value of the objective function is found. 
	-t timeout 
		This optional argument stops execution when the elapsed 
		cpu time equals or exceeds timeout value. Timeout is only checked 
		after completion of the main loop. Other halt values 
		such as 'target' and 'repeats' will halt before 'timeout'.
		The default value is 2592000.0.
	-n repeats 
		This optional argument denotes, once a new optimal value is 
		found, to repeat the main loop of the algorithm this number
		of times with no change in optimal value before stopping.  
		The default value is 50. 
	-S subproblemSize 
		This optional argument indicates the size of the sub-
		problems into which the QUBO will be decomposed.  A 
		subproblem size of 0 (the default) indicates to use the 
		size known by qbsolv to be most effective for the D-Wave 
		system targeted for execution.  A subproblem size greater 
		than zero indicates to use the given size and execute with 
		qbsolvs internal classical tabu solver rather than using 
		the D-Wave system. 
	-w 
		If present, this optional argument will print the QUBO 
		matrix and result in .csv format. 
	-h 
		If present, this optional argument will print the help or 
		usage message for qbsolv and exit without execution. 
	-v verbosityLevel 
		This optional argument denotes the verbosity of output. A 
		verbosityLevel of 0 (the default) will output the number of 
		bits in the solution, the solution, and the energy of the 
		solution.  A verbosityLevel of 1 will output the same 
		information for multiple solutions, if found. A 
		verbosityLevel of 2 will also output more detailed 
		information at each step of the algorithm.   
	-V 
		If present, this optional argument will emit the version 
		number of the qbsolv program and exit without execution. 
	-q 
		If present, this optional argument triggers printing the 
		format of the QUBO file.
	-r seed 
		Used to reset the seed for the random number generation 
