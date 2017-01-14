/*
 Copyright 2016 D-Wave Systems Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/
// -------  GLOBAL Variables  ------------------------------------------
extern FILE   *outFile_;
extern int    maxNodes_, nCouplers_, nNodes_, nRepeats_, findMax_, start_;
extern int    my_pid_, UseDwave_;
extern int    Verbose_, SubMatrix_, TargetSet_, RepeatPass_, WriteMatrix_, Tlist_;
extern char   *outFileNm_, pgmName_[16];
extern char   filename_b[256], filename_result[256];
extern char   filename_epqmi_max[256];
extern char   filename_max_full[256];
extern double Target_, Time_;

extern struct nodeStr_ *nodes_;
extern struct nodeStr_ *couplers_;

extern char       *workspace_, *connection_, *wspath_, *solver_, ws_tmp_path_[256], tmp_path_[256];
extern const char *DW_INTERNAL__WORKSPACE_;
extern const char *DW_INTERNAL__CONNECTION_;
extern const char *DW_INTERNAL__WSPATH_;
extern const char *DW_INTERNAL__SOLVER_;
