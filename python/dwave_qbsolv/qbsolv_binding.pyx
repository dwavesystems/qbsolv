import sys
import random
import logging

from libc.stdint cimport int8_t, int64_t, int32_t
from libc.stdio cimport stdout
from libc.stdlib cimport malloc, free, srand

from dwave_qbsolv.cqbsolv cimport default_parameters, dw_init, dw_close, dw_sub_sample
from dwave_qbsolv.cqbsolv cimport Verbose_, algo_, outFile_, Time_, Tlist_, numsolOut_, WriteMatrix_, TargetSet_, findMax_
from dwave_qbsolv.cqbsolv cimport solve, malloc2D

ENERGY_IMPACT = 0
SOLUTION_DIVERSITY = 1

PY2 = sys.version_info[0] == 2
if PY2:
    iteritems = lambda d: d.iteritems()
else:
    iteritems = lambda d: d.items()

log = logging.getLogger(__name__)


def run_qbsolv(Q, num_repeats=50, seed=17932241798878,  verbosity=-1,
               algorithm=None, timeout=2592000,
               solver=None):
    """TODO: update

    Runs qbsolv.

    Args:
        Q (dict): A dictionary defining the QUBO. Should be of the form
            {(u, v): bias} where u, v are variables and bias is numeric.
        num_repeats (int, optional): Determines the number of times to
            repeat the main loop in qbsolv after determining a better
            sample. Default 50.
        seed (int, optional): Random seed. Default None.
        algorithm (int, optional): Which algorithm to use. Default
            None. Algorithms numbers can be imported from module
            under the names ENERGY_IMPACT and SOLUTION_DIVERSITY.
        solver (function, optional): A function that finds low energy
            solutions from a small QUBO. Must be able to handle arbitrary
            QUBOs of size <= subproblem_size. TODO: more definition

    Returns:
        (list, list): (samples, counts) where samples is
        a list of dics, energies is the energy for each sample and counts
        is the number of times each sample was found by qbsolv.

    NB: relies on variables in Q being index valued and nonnegative, but not checked at this point
    """

    # first up, we want the default parameters for the solve function.
    params = default_parameters()

    log.debug('params.repeats = %d', num_repeats)
    cdef int32_t repeats = num_repeats
    params.repeats = num_repeats

    # Look for keywords identifying methods implemnted in the qbsolv c library
    if solver == 'tabu' or solver is None:
        log.debug('Using builtin tabu sub problem solver.')
    elif solver == 'dw':
        log.debug('Using builtin dw interface sub problem solver.')
        params.sub_sampler = &dw_sub_sample
        params.sub_size = dw_init();

    # Try to identify a dimod solver
    elif hasattr(solver, 'sample_ising') and hasattr(solver, 'sample_qubo'):
        log.debug('Using dimod as sub problem solver.')
        params.sub_sampler = &solver_callback
        params.sub_sampler_data = <void*>solver.sample_qubo

    # Otherwise any callable should work
    elif callable(solver):
        log.debug('Using callback as sub problem solver.')
        params.sub_sampler = &solver_callback
        params.sub_sampler_data = <void*>solver

    else:
        raise ValueError("Invalid value for solver argument {}".format(solver))

    # qbsolv relies on a number of global variables that need to be set before the algorithm can
    # be run, so let's go ahead and set them here.

    # some of the global variables are changed based on input parameters
    global Verbose_
    Verbose_ = verbosity

    global algo_
    cdef int n_solutions = 20  # the maximimum number of solutions returned
    if algorithm is None or algorithm == ENERGY_IMPACT:
        algo_[0] = "o"
        algo_[1] = 0
        # n_solutions = 20
    elif algorithm == SOLUTION_DIVERSITY:
        algo_[0] = "d"
        algo_[1] = 0
        n_solutions = 70
    else:
        raise ValueError('unknown algorithm given')

    if not isinstance(timeout, int) or timeout <= 0:
        raise ValueError("'timeout' must be a positive integer")
    global Time_
    Time_ = timeout # the maximum runtime of the algorithm in seconds before timeout (2592000 = a months worth of seconds)

    # other global variables are fixed for our purposes here

    global outFile_
    outFile_ = stdout

    global Tlist_
    Tlist_ = -1

    global numsolOut_
    numsolOut_ = 0

    global WriteMatrix_
    WriteMatrix_ = False

    global TargetSet_
    TargetSet_ = False

    global findMax_
    findMax_ = False

    # we also take the opporunity to set the random seed used by qbsolv. Qbsolv has a default random seed
    # so we mimic that behaviour here.
    if seed is None:
        seed = random.randint(0, 1L<<30)
        log.debug('setting random seed to %d', seed)
    cdef int64_t c_seed = seed
    srand(c_seed)

    # ok, all of the globals are set, so let's get to actually solving the given problem. First we need the
    # list of variables used by Q
    variables = set().union(*Q)

    # we also set the inputs to qbsolv, using cython to make them proper C values
    cdef int n_variables = len(variables)
    cdef int8_t **solution_list = <int8_t **>malloc2D(n_solutions + 1, n_variables, sizeof(int8_t))
    cdef double *energy_list = <double *>malloc((n_solutions + 1) * sizeof(double))
    cdef int *solution_counts = <int *>malloc((n_solutions + 1) * sizeof(int))
    cdef int *Qindex = <int *>malloc((n_solutions + 1) * sizeof(int))

    # create a matrix and set everything to 0
    cdef double **Q_array = <double **>malloc2D(n_variables, n_variables, sizeof(double))
    for row in range(n_variables):
        for col in range(n_variables):
            Q_array[row][col] = 0.


    # NB: for now we need to flip the sign of Q if we are doing minimization
    # This also affects other locations (ctrl+f QFLIP)
    cdef int u, v
    cdef double bias
    # put the values from Q into Q_array
    for (u, v), bias in iteritems(Q):
        # upper triangular
        if v < u:
            Q_array[v][u] = -bias
        else:
            Q_array[u][v] = -bias

    # Ok, solve using qbsolv! This puts the answer into output_sample
    solve(Q_array, n_variables, solution_list, energy_list, solution_counts, Qindex, n_solutions, &params)

    # we have three things we are interested in, the samples, the energies and the number of times each
    # sample appeared
    samples = []
    counts = []

    # it is actually faster to use a while loop here and keep everything as a C object
    cdef int i = 0
    while i < n_solutions:
        soln_idx = Qindex[i]  # Qindex tracks where the order of the solutions

        # if no solutions were found then we can stop
        if solution_counts[soln_idx] == 0:
            break

        # NB: in principle we have the energy list and could return them directly,
        # right now it appears that QBSolv has a bug, so we will fix this on future
        # release

        samples.append({v: int(solution_list[soln_idx][v]) for v in range(n_variables)})
        counts.append(int(solution_counts[soln_idx]))

        i += 1

    # free the allocated variables
    free(solution_list)
    free(energy_list)
    free(solution_counts)
    free(Qindex)
    free(Q_array)

    # Close dw session
    if solver == 'dw':
        dw_close();

    return samples, counts


cdef void solver_callback(double** Q_array, int n_variables, int8_t* best_solution, void *py_solver):
    log.debug('solver_callback invoked')

    # first we need Q_array to be a dict
    Q = {}
    cdef int v = 0
    cdef int u = 0
    while v < n_variables:
        u = 0
        while u < n_variables:
            bias = Q_array[u][v]

            # NB: for now we need to flip the sign of Q if we are doing minimization
            # This also affects other locations (ctrl+f QFLIP)
            if bias:
                if (u, v) in Q:
                    Q[(u, v)] -= bias
                elif (v, u) in Q:
                    Q[(v, u)] -= bias
                else:
                    Q[(u, v)] = -bias
            u += 1
        v += 1

    # next we want to convert our current best_solution into a single solution
    # dict
    solution = {}
    v = 0
    while v < n_variables:
        solution[v] = best_solution[v]
        v += 1

    new_solution = (<object>py_solver)(Q, solution)

    # finally we write new_solution back into best_solution which is also how
    # we return the value
    v = 0
    while v < n_variables:
        best_solution[v] = new_solution[v]
        v += 1
