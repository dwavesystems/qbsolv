============
Introduction
============

Divide-and-conquer and dynamic programming algorithms have a rich history in computer
science for problems with large numbers of variables. Many hard problems that can benefit
from quantum computers are too large to map directly to a QPU. To solve a problem with
more variables than the available number of qubits, we break the problem into subproblems, solve the
subproblems, and then reconstruct an answer to the original problem from the subproblem
solutions.

qbsolv is one such decomposing solver. It provides two interfaces:

* :ref:`usage` (CLI)

  The tabu algorithm is executed on the problem which is divided into subproblems of
  several dozen variables each.
* :ref:`python`

  The Python interface provides a :class:`~dwave_qbsolv.QBSolv` class wrapper for
  the qbsolv C code. A `dimod` sampler can be substituted for the default tabu algorithm.

For a description of the algorithm and implementation, see
:download:`Partitioning Optimization Problems for Hybrid Classical/Quantum Execution <../qbsolv_techReport.pdf>`.

For a description of the tabu search algorithm, see `Tabu search <https://en.wikipedia.org/wiki/Tabu_search>`_\ .

Example
=======

This example sends 30-variable sub-problems of a 500-variable QUBO to the `dwave-neal`
sampler to be incorporated into the tabu results run in the main loop of qbsolv. 

>>> from dwave_qbsolv import QBSolv
>>> import neal
>>> import itertools
>>> import random
...
>>> qubo_size = 500
>>> subqubo_size = 30
>>> Q = {t: random.uniform(-1, 1) for t in itertools.product(range(qubo_size), repeat=2)}
>>> sampler = neal.SimulatedAnnealingSampler()
>>> response = QBSolv().sample_qubo(Q, solver=sampler, solver_limit=subqubo_size)
>>> print("energies=" + str(list(response.data_vectors['energy'])))   # doctest: +SKIP
energies=[-2800.794817495185]
