from libc.stdint cimport int8_t, int64_t
from libc.stdio cimport stdout
from libc.stdlib cimport malloc, free, srand
from cqbsolv cimport Verbose_, SubMatrix_, UseDwave_, algo_, outFile_, Time_, Tlist_, numsolOut_, WriteMatrix_, TargetSet_, findMax_
from cqbsolv cimport solve, malloc2D
import random

ENERGY_IMPACT = 'energy impact'
SOLUTION_DIVERSITY = 'solution diversity'

def run_qbsolv(Q, repeats=50,
               seed=17932241798878, verbosity=-1, algorithm=None, timeout=2592000):
    """TODO
    
    Args:

    Returns:
        solution (list):

    NB: relies on variables in Q being index valued and nonnegative, but not checked at this point
    """

    # qbsolv relies on a number of global variables that need to be set before the algorithm can
    # be run, so let's go ahead and set them here.

    # some of the global variables are changed based on input parameters
    global Verbose_
    Verbose_ = verbosity

    global algo_
    if algorithm is None or algorithm == ENERGY_IMPACT:
        algo_ = "o"
    elif algorithm == SOLUTION_DIVERSITY:
        algo = "d"
    else:
        raise ValueError('unknown algorithm given')

    if not isinstance(timeout, int) or timeout <= 0:
        raise ValueError("'timeout' must be a positive integer")
    global Time_
    Time_ = timeout # the maximum runtime of the algorithm in seconds before timeout (2592000 = a months worth of seconds)

    # other global variables are fixed for our purposes here
    global UseDwave_
    UseDwave_ = False

    global outFile_
    outFile_ = stdout

    global SubMatrix_
    SubMatrix_ = 46

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
        seed = random.randint(0, 1000)
    cdef int64_t c_seed = seed
    srand(c_seed)
    
    # ok, all of the globals are set, so let's get to actually solving the given problem. First we need the
    # list of variables used by Q
    variables = set().union(*Q)

    # we also set the inputs to qbsolv, using cython to make them proper C values
    cdef int n_variables = len(variables)
    cdef int n_repeats = repeats
    cdef int8_t *output_sample = <int8_t *>malloc(n_variables * sizeof(int8_t))

    # create a matrix and set everything to 0
    cdef double **Qarr = <double **>malloc2D(n_variables, n_variables, sizeof(double))
    for row in range(n_variables):
        for col in range(n_variables):
            Qarr[row][col] = 0.

    # put the values from Q into Qarr
    for u, v in Q:
        # upper triangular
        if v < u:
            Qarr[v][u] = -1. * Q[(u, v)]
        else:
            Qarr[u][v] = -1. * Q[(u, v)]

    # Ok, solve using qbsolv! This puts the answer into output_sample
    solve(Qarr, n_variables, repeats, output_sample)

    # read the output out of output_sample, also converting the values back into
    # pure python objects
    sample = {v: int(output_sample[v]) for v in variables}

    # free the allocated variables
    free(output_sample)
    free(Qarr)

    return sample
