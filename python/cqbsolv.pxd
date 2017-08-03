from libc.stdint cimport int8_t, int32_t
from libc.stdio cimport FILE

cdef extern from "include.h":
    # void print_help()  # test
    cdef struct nodeStr_:
        int32_t n1, n2;
        double value;

    void  solve(double **Q_array, int n_variables, int n_repeats, int8_t *sample);

    void  **malloc2D(unsigned int rows, unsigned int cols, unsigned int size)

cdef extern from "extern.h":
    cdef FILE   *outFile_;
    cdef int    maxNodes_, nCouplers_, nNodes_, findMax_, start_, numsolOut_;
    cdef int    my_pid_, UseDwave_; 
    cdef int    Verbose_, SubMatrix_, TargetSet_, WriteMatrix_, Tlist_;
    cdef char   *outFileNm_, pgmName_[16], algo_[4];
    cdef double Target_, Time_;

    cdef nodeStr_ *nodes_;
    cdef nodeStr_ *couplers_;

