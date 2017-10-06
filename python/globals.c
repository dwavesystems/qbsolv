#include <stdint.h>
#include "../cmd/readqubo.h"

FILE   *outFile_;
FILE   *solution_input_;
int    maxNodes_, nCouplers_, nNodes_, findMax_, start_, numsolOut_;
int    Verbose_, TargetSet_, WriteMatrix_, Tlist_;
char   *outFileNm_, pgmName_[16], algo_[4];
double Target_, Time_;

struct nodeStr_ *nodes_;
struct nodeStr_ *couplers_;
