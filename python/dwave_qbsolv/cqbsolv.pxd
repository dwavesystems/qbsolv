from libc.stdint cimport int8_t, int32_t
from libc.stdio cimport FILE

cdef extern from "input_structures.h":
    # a single node in the problem QUBO
    cdef struct nodeStr_:
        int32_t n1, n2
        double value

    # pointer type for subsolver.
    # Args:
    #   -a 2d double array that is the sub-problem
    #   -the size of the sub-problem
    #   -a state vector: on input is the current best state
    #       and should be set to the output state
    ctypedef void (*SubSolver)(double**, int, int8_t*, void*)

    cdef struct parameters_t:
        int32_t repeats
        SubSolver sub_sampler
        int32_t sub_size
        void* sub_sampler_data

    parameters_t default_parameters()


cdef extern from "solver.h":
    void solve(double **qubo, const int qubo_size, int8_t **solution_list,
               double *energy_list, int *solution_counts, int *Qindex, int QLEN,
               parameters_t param)


cdef extern from "util.h":
    void  **malloc2D(unsigned int rows, unsigned int cols, unsigned int size)


cdef extern from "extern.h":
    cdef FILE *outFile_
    cdef int maxNodes_
    cdef int nCouplers_
    cdef int nNodes_
    cdef int findMax_
    cdef int start_
    cdef int numsolOut_
    cdef int my_pid_
    cdef int Verbose_
    cdef int TargetSet_
    cdef int WriteMatrix_
    cdef int Tlist_
    cdef char *outFileNm_
    cdef char pgmName_[16]
    cdef char algo_[4];
    cdef double Target_
    cdef double  Time_;

    cdef nodeStr_ *nodes_;
    cdef nodeStr_ *couplers_;

