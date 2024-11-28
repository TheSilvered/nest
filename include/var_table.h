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

/* Alias of `_Nst_vt_get` that casts `name` to `Nst_Obj *`. */
#define Nst_vt_get(vt, name) _Nst_vt_get(vt, OBJ(name))
/* Alias of `_Nst_vt_set` that casts `name` and `val` to `Nst_Obj *`. */
#define Nst_vt_set(vt, name, val) _Nst_vt_set(vt, OBJ(name), OBJ(val))

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
    Nst_MapObj *vars;
    Nst_MapObj *global_table;
} Nst_VarTable;

/**
 * Creates a new var table on the heap.
 *
 * @param global_table: the current global variable table, can be `NULL`
 * @param cwd: the current working directory, ignored when `global_table` is
 * not `NULL` or `no_default` is `true`
 * @param args: the command line arguments, ignored when `global_table` is not
 * `NULL` or `no_default` is `true`
 * @param no_default: whether to create predefined variables
 */
NstEXP Nst_VarTable *NstC Nst_vt_new(Nst_MapObj *global_table,
                                     Nst_SeqObj *args, bool no_default);
/* `Nst_VarTable` destructor. */
NstEXP void NstC Nst_vt_destroy(Nst_VarTable *vt);
/**
 * Retrieves a value from a variable table.
 *
 * @param vt: the variable table to get the value from
 * @param name: the name of the value to get
 *
 * @return The value associated with the key or `Nst_null()` if the key is not
 * present in the table.
 */
NstEXP Nst_Obj *NstC _Nst_vt_get(Nst_VarTable *vt, Nst_Obj *name);
/**
 * Sets a value in a variable table.
 *
 * @param vt: the variable table to set the value into
 * @param name: the name of the value to set
 * @param value: the value to associate to name
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC _Nst_vt_set(Nst_VarTable *vt, Nst_Obj *name, Nst_Obj *val);

/**
 * Creates a new variable table with the correct global table of the function.
 *
 * @param f: the function where the global table is stored
 *
 * @return The new var table or `NULL` on failure. The error is set.
 */
NstEXP Nst_VarTable *NstC Nst_vt_from_func(Nst_FuncObj *f);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VAR_TABLE_H
