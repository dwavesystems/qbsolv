
cdef extern from "include.h":
    void print_help()  # test
    void  solve(double **val, int maxNodes, int nRepeats, int8_t *bestSolution);
