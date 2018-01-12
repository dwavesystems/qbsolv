#ifndef EPQMI_H_INCLUDED
#define EPQMI_H_INCLUDED

#define DW_EPQMI_MAGIC 0x7a6b0c4d

typedef struct DW_epqmi { unsigned int magic; } DW_epqmi;

/*
 * If the epqmi_file argument is a NULL pointer, use the current DW
 * environment.  Assume the file is named "default.epqmi" and is
 * located in the current workspace.  Otherwise, assume that the
 * epqmi_file names a file in the current workspace.  This returns
 * NULL if there is a problem and a non-NULL pointer if all is
 * successful.
 */

DW_epqmi *DW_epqmi_read(char *epqmi_file);

int DW_epqmi_list_params(DW_epqmi *epqmi, char ***param_names, int *params);

int DW_epqmi_list_vars(DW_epqmi *epqmi, char ***var_names, int *vars);

int DW_epqmi_bind(DW_epqmi *epqmi, float *param_values);

int DW_epqmi_exec(DW_epqmi *epqmi, int num_reads);

int DW_epqmi_sols(DW_epqmi *epqmi, int *solutions);

int DW_epqmi_sol_vars(DW_epqmi *epqmi, int solnum, char *var, int *valid);

int DW_epqmi_sol_occurs(DW_epqmi *epqmi, int solnum, int *occurrences);

int DW_epqmi_sol_obj(DW_epqmi *epqmi, int solnum, float *objective);

int DW_epqmi_free(DW_epqmi *epqmi);

#endif  // EPQMI_H_INCLUDED
