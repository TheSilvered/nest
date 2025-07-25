/**
 * @file var_table.h
 *
 * @brief Variable table interface using Nst_MapObj
 *
 * @author TheSilvered
 */

#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include "map.h"
#include "sequence.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Structure representing the Nest variable table
 *
 * @param vars: the map of local variables
 * @param global_table: the map of global variables
 */
NstEXP typedef struct _Nst_VarTable {
    Nst_ObjRef *vars;
    Nst_ObjRef *global_table;
} Nst_VarTable;

/**
 * Initialize a variable table.
 *
 * @param global_table: the current global variable table, can be `NULL`
 * @param cwd: the current working directory, ignored when `global_table` is
 * not `NULL` or `no_default` is `true`
 * @param args: the command line arguments, ignored when `global_table` is not
 * `NULL` or `no_default` is `true`
 * @param no_default: whether to create predefined variables
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_vt_init(Nst_VarTable *vt, Nst_Obj *global_table,
                             Nst_Obj *args, bool no_default);
/**
 * Destroy the contents of an `Nst_VarTable`. If `_vars_` still points to the
 * `vars` field of the table it is dropped.
 */
NstEXP void NstC Nst_vt_destroy(Nst_VarTable *vt);
/**
 * Get a value from a variable table.
 *
 * @param vt: the variable table to get the value from
 * @param name: the name of the value to get
 *
 * @return The value associated with the key or `Nst_null()` if the key is not
 * present in the table.
 */
NstEXP Nst_ObjRef *NstC Nst_vt_get(Nst_VarTable vt, Nst_Obj *name);
/**
 * Set a value in a variable table.
 *
 * @param vt: the variable table to set the value into
 * @param name: the name of the value to set
 * @param value: the value to associate to name
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_vt_set(Nst_VarTable vt, Nst_Obj *name, Nst_Obj *val);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VAR_TABLE_H
