#include "../src/util.c"
#include "../src/extern.h"
#include "../src/include.h"

// compile with gcc util_index_solution_test.c
// this is also for visual inspection testing while developing
// the functions to compare and index differences in solutions
// look over the random "differenced" solution to see that
// mul_index properly identifies the non like solutions

FILE *outFile_;
int maxNodes_, nCouplers_, nNodes_, findMax_;
int Verbose_, SubMatrix_, UseDwave_, TargetSet_, WriteMatrix_, Tlist_;
char *outFileNm_, pgmName_[16];
double **val;
double Target_, Time_;
struct nodeStr_ *nodes_;
struct nodeStr_ *couplers_;

int main() {
    strcpy(pgmName_, "testindex");
    findMax_ = false;
    UseDwave_ = false;
    Verbose_ = 0;
    SubMatrix_ = 46;  // submatrix default
    WriteMatrix_ = false;
    outFile_ = stdout;
    TargetSet_ = false;
    int n = 80, nsol = 40;
    int8_t **solution_list;
    int *index, *e_index;
    int i, j, k, m;

    solution_list = (int8_t **)malloc2D(nsol + 1, n, sizeof(int8_t));
    GETMEM(index, int, n);
    GETMEM(e_index, int, nsol);

    for (i = 0; i < nsol; i++) {
        e_index[i] = i;
    }

    shuffle_index(index, nsol);

    int pass = 0;
    while (pass <= 4) {
        for (i = 0; i < nsol; i++) randomize_solution(&solution_list[i][0], n);
        for (i = nsol - 2; i > -1; i--) {
            for (j = 0; j < n; j++) {
                if (rand() % 4 != 1) {
                    solution_list[i][j] = solution_list[i + 1][j];
                }
            }
        }
        for (i = 1; i < nsol; i++) {
            for (j = 1; j < n; j++) {
                if (rand() % 4 != 1) {
                    solution_list[i][j] = solution_list[i - 1][j];
                }
            }
        }
        printf("---------------\n");
        for (k = 2; k < nsol; k++) {
            if (k == 2) {
                printf(" using index_solution_diff \n");
                m = index_solution_diff(&solution_list[e_index[0]][0], &solution_list[e_index[1]][0], n, index);
            } else {
                printf(" using mul_index_solution_diff \n");
                m = mul_index_solution_diff(solution_list, k, n, index, 0, e_index);
            }
            for (i = 0; i < k; i++) {
                for (j = 0; j < n; j++) {
                    printf("%d", solution_list[e_index[i]][j]);
                }
                printf("\n");
            }
            m = 0;
            for (j = 0; j < n; j++) {
                if (index[m] == j) {
                    printf("*");
                    m++;
                } else {
                    printf("-");
                }
            }
            printf("\n");
            for (j = 0; j < n; j++) {
                printf("%d", j % 10);
            }
            printf("\n");
            for (j = 0; j < m; j++) {
                printf(" %d ", index[j]);
            }
            printf("\n\n");
            pass++;
        }
    }
}
