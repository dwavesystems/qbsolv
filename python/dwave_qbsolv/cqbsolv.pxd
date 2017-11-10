# distutils: language=c++
from libc.stdio cimport FILE

cdef extern from "stdheaders_shim.h":
    # When a ctypedef is externed cython uses the type information for
    # building the interface, but the actual typedef used in the generated
    # code comes from the .h file. This is a work around for stdint being
    # missing from older Microsoft c compilers.
    ctypedef char int8_t
    ctypedef long long int64_t
    ctypedef int int32_t

cdef extern from "qbsolv.h":
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

    void solve(double **qubo, const int qubo_size, int8_t **solution_list,
               double *energy_list, int *solution_counts, int *Qindex, int QLEN,
               parameters_t *param)

    void dw_sub_sample(double**, int, int8_t*, void*)

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

cdef extern from "dwsolv.h":
    void dw_close()
    int dw_init()
