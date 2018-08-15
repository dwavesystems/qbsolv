qbsolv
======

[![Build Status](https://travis-ci.org/dwavesystems/qbsolv.svg?branch=master)](https://travis-ci.org/dwavesystems/qbsolv)
[![Build status](https://ci.appveyor.com/api/projects/status/y2f7rqxvepn4ak4b/branch/master?svg=true)](https://ci.appveyor.com/project/dwave-adtt/qbsolv/branch/master)

A decomposing solver, finds a minimum value of a large quadratic unconstrained binary optimization (QUBO) problem by
splitting it into pieces solved either via a D-Wave system or a classical tabu solver.

_(Note that qbsolv by default uses its internal classical solver. Access to a D-Wave system must be arranged separately.)_

Installation or Building
------------------------

#### Python

A wheel might be available for your system on pypi. Source distributions are provided as well.

```bash
pip install dwave-qbsolv
```

Alternately, you can build the library with setuptools

```bash
pip install -r python/requirements.txt
pip install cython==0.27
python setup.py install
```

#### C
To build the C library use cmake to generate a build command for your system. On Linux the commands would be something
like this:

```
mkdir build; cd build
cmake ..
make
```

To build the command line interface turn the cmake option `QBSOLV_BUILD_CMD` on. The command line option for cmake to do
this would be `-DQBSOLV_BUILD_CMD=ON`. To build the tests turn the cmake option `QBSOLV_BUILD_TESTS` on. The command
line option for cmake to do this would be `-DQBSOLV_BUILD_TESTS=ON`.

Command Line Usage
------------------

```
    qbsolv -i infile [-o outfile] [-m] [-T] [-n] [-S SubMatrix] [-w]
        [-h] [-a algorithm] [-v verbosityLevel] [-V] [-q] [-t seconds]

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
    -a algorithm
        This optional argument chooses nuances of the outer loop
        algorithm.  The default is o.
        'o' for original qbsolv method. Submatrix based upon change in energy.
        'p' for path relinking.  Submatrix based upon differences of solutions
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
        "-S 0" or "-S" argument not present indicates to use the
        size specified in the embedding file found in the workspace
        set up by DW.  If a DW environment has not been established,
        the value will default to (47) and will use the tabu solver
        for subproblem solutions.  If a value is specified, qbsolv uses
        that value to create subproblem and solve with the tabu solver.
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
        information at each step of the algorithm. This increases
        the output up to a value of 4.
    -V
        If present, this optional argument will emit the version
        number of the qbsolv program and exit without execution.
    -q
        If present, this optional argument triggers printing the
        format of the QUBO file.
    -r seed
        Used to reset the seed for the random number generation

------------------------
qbsolv "qubo" input file format

   A .qubo file contains data which describes an unconstrained
quadratic binary optimization problem.  It is an ASCII file comprised
of four types of lines:

1) Comments - defined by a "c" in column 1.  They may appear
    anywhere in the file, and are otherwise ignored.

2) One program line, which starts with p in the first column.
    The program line must be the first non-comment line in the file.
    The program line has six required fields (separated by space(s)),
    as in this example:

  p   qubo  topology   maxNodes   nNodes   nCouplers

    where:
  p         the problem line sentinel
  qubo      identifies the file type
  topology  a string which identifies the topology of the problem
            and the specific problem type.  For an unconstrained problem,
            target will be "0" or "unconstrained."   Possible, for future
            implementations, valid strings might include "chimera128"
            or "chimera512" (among others).
  maxNodes   number of nodes in the topology.
  nNodes     number of nodes in the problem (nNodes <= maxNodes).
            Each node has a unique number and must take a value in the
            the range {0 - (maxNodes-1)}.  A duplicate node number is an
            error.  The node numbers need not be in order, and they need
            not be contiguous.
  nCouplers  number of couplers in the problem.  Each coupler is a
            unique connection between two different nodes.  The maximum
            number of couplers is (nNodes)^2.  A duplicate coupler is
            an error.

3) nNodes clauses.  Each clause is made up of three numbers.  The
            numbers are separated by one or more blanks.  The first two
            numbers must be integers and are the number for this node
            (repeated).  The node number must be in {0 , (maxNodes-1)}.
            The third value is the weight associated with the node, may be
            an integer or float, and can take on any positive or negative
            value, or zero.

4) nCouplers clauses.  Each clause is made up of three numbers.  The
            numbers are separated by one or more blanks.  The first two
            numbers must be different integers and are the node numbers
            for this coupler.  The two values (i and j) must have (i < j).
            Each number must be one of the nNodes valid node numbers (and
            thus in {0, (maxNodes-1)}).  The third value is the strength
            associated with the coupler, may be an integer or float, and can
            take on any positive or negative value, but not zero.  Every node
            must connect with at least one other node (thus must have at least
            one coupler connected to it).

Here is a simple QUBO file example for an unconstrained QUBO with 4
nodes and 6 couplers.  This example is provided to illustrate the
elements of a QUBO benchmark file, not to represent a real problem.

        | <--- column 1
        c
        c  This is a sample .qubo file
        c  with 4 nodes and 6 couplers
        c
        p  qubo  0  4  4  6
        c ------------------
        0  0   3.4
        1  1   4.5
        2  2   2.1
        3  3   -2.4
        c ------------------
        0  1   2.2
        0  2   3.4
        1  2   4.5
        0  3   -2
        1  3   4.5678
        2  3   -3.22
```

Library usage
-------------

TODO

Contribution
------------

See `contrib.txt`
