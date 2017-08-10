from libc.stdint cimport int8_t, int32_t
from libc.stdio cimport FILE

cdef extern from "include.h":
    cdef struct nodeStr_:
        int32_t n1, n2;
        double value;

    void  solve(double **val, int maxNodes, int nRepeats, int8_t **solution_list, double *energy_list, int *solution_counts, int *Qindex, int QLEN)
    
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
    cdef int UseDwave_
    cdef int Verbose_
    cdef int SubMatrix_
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

