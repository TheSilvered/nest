/* C/C++ Library utilities */

#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "interpreter.h"
#include "global_consts.h"

#define Nst_MAKE_FUNCDECLR(func_ptr, argc)                                    \
    {                                                                         \
        (void *)(func_ptr),                                                   \
        argc,                                                                 \
        STR(Nst_string_new_c_raw(#func_ptr, false))                           \
    }

#define Nst_MAKE_NAMED_FUNCDECLR(func_ptr, argc, func_name)                   \
    {                                                                         \
        (void *)(func_ptr),                                                   \
        argc,                                                                 \
        STR(Nst_string_new_c_raw(func_name, false))                           \
    }

#define Nst_MAKE_OBJDECLR(obj_ptr)                                            \
    {                                                                         \
        (void *)(obj_ptr),                                                    \
        -1,                                                                   \
        STR(Nst_string_new_c_raw(#obj_ptr, false))                            \
    }

#define Nst_MAKE_NAMED_OBJDECLR(obj_ptr, obj_name)                            \
    {                                                                         \
        (void *)(obj_ptr),                                                    \
        -1,                                                                   \
        STR(Nst_string_new_c_raw(obj_name, false))                            \
    }

#define Nst_RETURN_TRUE return Nst_inc_ref(Nst_true())
#define Nst_RETURN_FALSE return Nst_inc_ref(Nst_false())
#define Nst_RETURN_NULL return Nst_inc_ref(Nst_null())
#define Nst_RETURN_ZERO return Nst_inc_ref(Nst_const()->Int_0)
#define Nst_RETURN_ONE return Nst_inc_ref(Nst_const()->Int_1)
#define Nst_RETURN_COND(cond)                                                 \
    return (cond) ? Nst_inc_ref(Nst_true()) : Nst_inc_ref(Nst_false())

// Function signature for Nest function
#define Nst_FUNC_SIGN(name)                                                   \
    Nst_Obj *NstC name(usize arg_num, Nst_Obj **args)

// Default call to Nst_extract_arg_values
#define Nst_DEF_EXTRACT(ltrl, ...)                                            \
    if ( !Nst_extract_arg_values(ltrl, arg_num, args, __VA_ARGS__) )          \
        return NULL

// Sets 'def_val' if 'obj' is null else 'val'
#define Nst_DEF_VAL(obj, val, def_val)                                        \
    ((obj) == Nst_null() ? (def_val) : (val))

#define Nst_RETURN_NEW_STR(val, len) do {                                     \
    Nst_Obj *_s_ = Nst_string_new(val, len, true);                            \
    if ( _s_ == NULL ) Nst_free(val);                                         \
    return _s_;                                                               \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef struct _Nst_ObjDeclr {
    void *ptr;
    isize arg_num;
    Nst_StrObj *name;
} Nst_ObjDeclr;

NstEXP typedef struct _Nst_DeclrList {
    Nst_ObjDeclr *objs;
    usize obj_count;
} Nst_DeclrList;

// Extracts the C values from the arguments
// `types` is a string of letters for the types, check the full usage in
// src/lib_import.c
// `arg_num`: number of arguments the function expects
// `args`: the arguments themselves
// `err`: the `err` argument of the function
// ...: the pointers to store the values in or the custom types
NstEXP bool NstC Nst_extract_arg_values(const i8 *types, usize arg_num,
                                        Nst_Obj **args, ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H
