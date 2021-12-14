.. image:: https://img.shields.io/pypi/v/dwave-qbsolv.svg
    :target: https://pypi.python.org/pypi/dwave-qbsolv

.. image:: https://codecov.io/gh/dwavesystems/qbsolv/branch/master/graph/badge.svg
    :target: https://codecov.io/gh/dwavesystems/qbsolv

.. image:: https://travis-ci.org/dwavesystems/qbsolv.svg?branch=master
    :target: https://travis-ci.org/dwavesystems/qbsolv

.. image:: https://ci.appveyor.com/api/projects/status/y2f7rqxvepn4ak4b/branch/master?svg=true
    :target: https://ci.appveyor.com/project/dwave-adtt/qbsolv/branch/master

.. image:: https://readthedocs.com/projects/d-wave-systems-qbsolv/badge/?version=latest
    :target: https://docs.ocean.dwavesys.com/projects/qbsolv/en/latest/?badge=latest

.. image:: https://circleci.com/gh/dwavesystems/qbsolv.svg?style=svg
    :target: https://circleci.com/gh/dwavesystems/qbsolv

======
Qbsolv
======

**NOTICE**: This repository is deprecated as of the end of 2021. Support will be
discontinued after March 2022. Please update your code to use Ocean's
`dwave-hybrid <https://docs.ocean.dwavesys.com/en/stable/docs_hybrid/sdk_index.html>`_
or Leap's quantum-classical
`hybrid solvers <https://docs.dwavesys.com/docs/latest/doc_leap_hybrid.html>`_
instead.

 .. index-start-marker

A decomposing solver that finds a minimum value of a large quadratic unconstrained binary
optimization (QUBO) problem by splitting it into pieces. The pieces are solved using a
classical solver running the tabu algorithm. qbsolv also enables configuring a D-Wave
system as the solver.

.. Note:: Access to a D-Wave system must be arranged separately.

.. index-end-marker

Installation or Building
========================

.. installation-start-marker

Python
------

A wheel might be available for your system on PyPI. Source distributions are provided as well.

.. code-block:: python

    pip install dwave-qbsolv


Alternatively, you can build the library with setuptools.

.. code-block:: bash

    pip install -r python/requirements.txt
    python setup.py install

C
-

To build the C library use cmake to generate a build command for your system. On Linux the commands would be something
like this:

.. code-block:: bash

    mkdir build; cd build
    cmake ..
    make

To build the command line interface turn the cmake option `QBSOLV_BUILD_CMD` on. The command line option for cmake to do
this would be `-DQBSOLV_BUILD_CMD=ON`. To build the tests turn the cmake option `QBSOLV_BUILD_TESTS` on. The command
line option for cmake to do this would be `-DQBSOLV_BUILD_TESTS=ON`.

.. installation-end-marker

Command Line Usage
==================

.. usage-start-marker

.. code::

    qbsolv -i infile [-o outfile] [-m] [-T] [-n] [-S SubMatrix] [-w]
        [-h] [-a algorithm] [-v verbosityLevel] [-V] [-q] [-t seconds]

Description
-----------

qbsolv executes a quadratic unconstrained binary optimization
(QUBO) problem represented in a file. It returns bit-vector
results that minimizes---or optionally, maximizes---the value of
the objective function represented by the QUBO.  The problem is
represented in QUBO(5) file format.

The QUBO input problem is not limited to the graph size or connectivity of a
sampler, for example the D-Wave system.

Options are as follows:

.. code::

    -i infile
        Name of the file for the input QUBO. This option is mandatory.
    -o outfile
        Optional output filename.
        Default is the standard output.
    -a algorithm
        Optional selection for the outer loop algorithm.  Default is o.
        'o' for original qbsolv method. Submatrix based upon change in energy.
        'p' for path relinking.  Submatrix based upon differences of solutions
    -m
        Optional selection of finding the maximum instead of the minimum.
    -T target
        Optional argument target value of the objective function. Stops execution when found.
    -t timeout
        Optional timeout value. Stops execution when the elapsed CPU time equals or
        exceeds it. Timeout is only checked after completion of the main
        loop. Other halt values such as 'target' and 'repeats' halt before 'timeout'.
        Default value is 2592000.0.
    -n repeats
        Optional number of times the main loop of the algorithm is repeated with
        no change in optimal value found before stopping.
        Default value is 50.
    -S subproblemSize
        Optional size of the sub-problems into which the QUBO is decomposed.
        If no "-S 0" or "-S" argument is present, uses the size specified in the
        embedding file found in the workspace set up by DW. If no DW environment is
        established, value defaults to 47 and uses the tabu solver on subproblems.
        If a value is specified, subproblems based on that size are solved with the
        tabu solver.
    -w
        If present, the QUBO matrix and result are printed in .csv format.
    -h
        If present, prints the help or usage message for qbsolv and exits without execution.
    -v verbosityLevel
        Optional setting of the verbosity of output. The default verbosityLevel of
        0 outputs the number of bits in the solution, the solution,
        and the energy of the solution.  A verbosityLevel of 1 outputs the same
        information for multiple solutions, if found. A verbosityLevel of 2
        also outputs more detailed information at each step of the algorithm. The
        information increases for verbosity levels of up to 4.
    -V
        If present, prints the version number of the qbsolv program and exits without execution.
    -q
        If present, prints the format of the QUBO file.
    -r seed
        Used to reset the seed for the random number generation.

.. usage-end-marker

qbsolv QUBO Input File Format
=============================

.. format-start-marker

A .qubo file contains data that describes an unconstrained
quadratic binary optimization problem.  It is an ASCII file comprising
four types of lines:

1. Comments defined by a "c" in column 1. Comments may appear
   anywhere in the file, and are ignored.

2. Program line defined by a "p" in the first column.
   A single program line must be the first non-comment line in the file.
   The program line has six required fields separated by space(s),
   as in this example:

    .. code::

       p   qubo  topology   maxNodes   nNodes   nCouplers

    where:

    .. code::

       p          Problem line sentinel.
       qubo       File type identifier.
       topology   String that identifies the topology of the problem and the specific
                  problem type. For an unconstrained problem, target is "0" or
                  "unconstrained." In future implementations, valid strings
                  might include "chimera128" or "chimera512" (among others).
       maxNodes   Number of nodes in the topology.
       nNodes     Number of nodes in the problem (nNodes <= maxNodes).
                  Each node has a unique number and must take a value in the range
                  {0 - (maxNodes-1)}. A duplicate node number is an error. Node
                  numbers need not be in order, and need not be contiguous.
       nCouplers  Number of couplers in the problem. Each coupler is a unique connection
                  between two different nodes. The maximum number of couplers is (nNodes)^2.
                  A duplicate coupler is an error.

3. nNodes clauses. Each clause is made up of three numbers, separated
   by one or more blanks. The first two numbers must be integers and are the number
   for this node (repeated). The node number must be in range {0 , (maxNodes-1)}.
   The third value is the weight associated with the node. Weight may be an integer
   or float, and can take on any positive or negative value, or be set to zero.

4. nCouplers clauses. Each clause is made up of three numbers, separated by one or
   more blanks. The first two numbers, (i and j), are the node numbers for this coupler
   and must be different integers, where (i < j).Each number must be one of the nNodes
   valid node numbers (and thus in range {0, (maxNodes-1)}).
   The third value is the strength associated with the coupler. Strength may be an
   integer or float, and can take on any positive or negative value, but not zero.
   Every node must connect with at least one other node (thus must have at least
   one coupler connected to it).

Here is a simple QUBO file example for an unconstrained QUBO with 4
nodes and 6 couplers. This example is provided to illustrate the
elements of a QUBO benchmark file, not to represent a real problem.

.. code::

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

.. format-end-marker

Library usage
-------------

TODO
