/**
 * @file function.h
 *
 * @brief Nst_FuncObj interface
 *
 * @author TheSilvered
 */

#ifndef FUNCTION_H
#define FUNCTION_H

#include "compiler.h"
#include "map.h"

/* Casts ptr to Nst_FuncObj * */
#define FUNC(ptr) ((Nst_FuncObj *)(ptr))
/**
 * @brief Alias for _Nst_func_set_vt that casts func to Nst_FuncObj * and map
 * to Nst_MapObj *
 */
#define Nst_func_set_vt(func, map)                                            \
    _Nst_func_set_vt((Nst_FuncObj *)(func), (Nst_MapObj *)(map))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus


/**
 * The union representing the body of a function object.
 *
 * @param bytecode: the body is an instruction list
 * @param c_func: the body is a C function
 */
NstEXP typedef union _Nst_FuncBody {
    Nst_InstList *bytecode;
    Nst_Obj *(*c_func)(usize arg_num, Nst_Obj **args);
} Nst_FuncBody;

/**
 * The structure representing a Nest function object.
 *
 * @param body: the body of the function
 * @param args: the array of names of the arguments
 * @param arg_num: the maximum number of arguments
 * @param mod_globals: the global variable table when the function was defined
 */
NstEXP typedef struct _Nst_FuncObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_FuncBody body;
    Nst_Obj **args;
    usize arg_num;
    Nst_MapObj *mod_globals;
} Nst_FuncObj;

/**
 * Creates a new function object with an instruction-list body.
 *
 * @brief bytecode is assumed to be a valid non-NULL pointer. The args array
 * must be set later manually.
 *
 * @param arg_num: the maximum number of arguments the function accepts
 * @param bytecode: the body of the function
 *
 * @return The new function object or NULL on failure. On failure the error is
 * set.
 */
NstEXP Nst_Obj *Nst_func_new(usize arg_num, Nst_InstList *bytecode);
/**
 * Creates a new function object with a C function body.
 *
 * @brief cbody is assumed to be a valid non_NULL pointer. The args array must
 * NOT be set since it is not used.
 *
 * @param arg_num: the maximum number of arguments the function accepts
 * @param cbody: the body of the function
 *
 * @return The new function object or NULL on failure. On failure the error is
 * set.
 */
NstEXP Nst_Obj *Nst_func_new_c(usize arg_num,
                               Nst_Obj *(*cbody)(usize     arg_num,
                                                 Nst_Obj  **args));
/**
 * Sets the mod_globals table of a function and all the functions defined
 * inside it.
 *
 * @brief If the field is already set or the function has a C body, it is not
 * modified
 *
 * @param func: the function to change the mod_globals field of
 * @param map: the map to set as the new value
 */
NstEXP void _Nst_func_set_vt(Nst_FuncObj *func, Nst_MapObj *map);

/* Traverse function for Nst_FuncObj */
NstEXP void _Nst_func_traverse(Nst_FuncObj *func);
/* Track function for Nst_FuncObj */
NstEXP void _Nst_func_track(Nst_FuncObj *func);
/* Destructor for Nst_Func_Obj */
NstEXP void _Nst_func_destroy(Nst_FuncObj *func);

/* The flags for Nst_FuncObj */
NstEXP typedef enum _Nst_FuncFlags {
    Nst_FLAG_FUNC_IS_C = 0b1
} Nst_FuncFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FUNCTION_H
