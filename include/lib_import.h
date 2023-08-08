/**
 * @file lib_import.h
 *
 * @brief C/C++ library utilities
 *
 * @author TheSilvered
 */

/*
  Usage of the 'types' argument:

  Builtin types
    't': type
    'i': integer
    'r': real
    'b': bool
    'n': null
    's': string
    'v': vector
    'a': array
    'm': map
    'f': func
    'I': iter
    'B': byte
    'F': file

  Shorthands
    'l': i|B_i
    'N': i|r|B_r
    'A': a|v
    'S': a|v|s:a
    'R': I|a|v|s:I
    'y': o_b

  Other:
    'o': any object

  Custom types:
    You can have any number of custom types in a single argument and you should
    label them with #. The custom type(s) are to be passed to the function
    before the pointer to the variable in the order in which they appear.

    Nst_DEF_EXTRACT("#|#|#", custom_type1, custom_type2, custom_type3, &var);

  Optional types:
    To have an optional type you can use ? before the type itself. Using |n is
    the same thing. i|n is the same as ?i

  Multiple types per argument:
    To specifiy more than one type that an argument can be, use a pipe (|)
    between the various type.

  Automatic type casting:
    After the type specified you can add : or _ followed by exactly one letter.
    : is a cast between Nest objects, _ is a cast to a C type. When using the
    latter there cannot be any optional or custom types and it is restricted to
    only these types after the underscore: i, r, b or B.

  Implicit casting:
    If a type is specified as only one of i, r, b or B it automatically becomes
    i_i, r_r, b_b or B_B if it is not used to check the contents of a sequence
    To get the object itself use i:i, r:r, b:b, B:B and then immediatly
    decrease the reference (it is safe in this case since no new objects are
    created)

  Sequence type checking:
    You can additionally check the types present inside the matched sequence
    By following the type with a dot

  Example:
    i|r|B_B?A.#|i -> An Int, Real, Byte all casted to a u8 followed by an
                     optional Array or Vector that, if it exists, should
                     contain only objects of a custom type or integers.
*/

#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "interpreter.h"
#include "global_consts.h"

/**
 * Initializes a function declaration.
 *
 * @brief For the name of the function the name of the function pointer is
 * used.
 *
 * @param func_ptr: the function pointer to use
 * @param argc: the number of arguments the function accepts
 */
#define Nst_MAKE_FUNCDECLR(func_ptr, argc)                                    \
    {                                                                         \
        (void *)(func_ptr),                                                   \
        argc,                                                                 \
        STR(Nst_string_new_c_raw(#func_ptr, false))                           \
    }

/**
 * Initializes a function declaration with a custom name.
 *
 * @param func_ptr: the function pointer to use
 * @param argc: the number of arguments the function accepts
 * @param func_name: the name to use as a C string
 */
#define Nst_MAKE_NAMED_FUNCDECLR(func_ptr, argc, func_name)                   \
    {                                                                         \
        (void *)(func_ptr),                                                   \
        argc,                                                                 \
        STR(Nst_string_new_c_raw(func_name, false))                           \
    }

/**
 * Initialized an object declaration.
 *
 * @brief For the name of the object the name of the pointer is used.
 *
 * @param obj_pointer: the pointer to the Nest object to declare
 */
#define Nst_MAKE_OBJDECLR(obj_ptr)                                            \
    {                                                                         \
        (void *)(obj_ptr),                                                    \
        -1,                                                                   \
        STR(Nst_string_new_c_raw(#obj_ptr, false))                            \
    }

/**
 * Initialized an object declaration with a custom name.
 *
 * @param obj_pointer: the pointer to the Nest object to declare
 * @param obj_name: the name to use as a C string
 */
#define Nst_MAKE_NAMED_OBJDECLR(obj_ptr, obj_name)                            \
    {                                                                         \
        (void *)(obj_ptr),                                                    \
        -1,                                                                   \
        STR(Nst_string_new_c_raw(obj_name, false))                            \
    }

/* Returns Nst_true(). */
#define Nst_RETURN_TRUE return Nst_inc_ref(Nst_true())
/* Returns Nst_false(). */
#define Nst_RETURN_FALSE return Nst_inc_ref(Nst_false())
/* Returns Nst_null(). */
#define Nst_RETURN_NULL return Nst_inc_ref(Nst_null())
/* Returns Nst_const()->Int_0. */
#define Nst_RETURN_ZERO return Nst_inc_ref(Nst_const()->Int_0)
/* Returns Nst_const()->Int_1. */
#define Nst_RETURN_ONE return Nst_inc_ref(Nst_const()->Int_1)
/**
 * @brief Returns Nst_true() if cond is true and Nst_false otherwise. cond is a
 * C condition.
 */
#define Nst_RETURN_COND(cond)                                                 \
    return (cond) ? Nst_inc_ref(Nst_true()) : Nst_inc_ref(Nst_false())

/* Function signature for a Nest-callable C function. */
#define Nst_FUNC_SIGN(name)                                                   \
    Nst_Obj *NstC name(usize arg_num, Nst_Obj **args)

/* Default call to Nst_extract_arg_values that returns nullptr on error. */
#define Nst_DEF_EXTRACT(ltrl, ...) do {                                       \
    if (!Nst_extract_arg_values(ltrl, arg_num, args, __VA_ARGS__))            \
        return NULL;                                                          \
    } while (0)

/* Results in def_val if obj is Nst_null() and in val otherwise. */
#define Nst_DEF_VAL(obj, val, def_val)                                        \
    (OBJ(obj) == Nst_null() ? (def_val) : (val))

/* Checks if the type of an object is type_name. */
#define Nst_T(obj, type_name) ((obj)->type == Nst_type()->type_name)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Structure defining an object declaration.
 *
 * @param ptr: the pointer to the object or function
 * @param arg_num: the number of arguments if the object is a function, -1 for
 * other declarations
 * @param name: the name of the declared object
 */
NstEXP typedef struct _Nst_ObjDeclr {
    void *ptr;
    isize arg_num;
    Nst_StrObj *name;
} Nst_ObjDeclr;

/**
 * Structure defining a list of object declarations.
 *
 * @param objs: the array of declared objects
 * @param obj_count: the number of objects inside the array
 */
NstEXP typedef struct _Nst_DeclrList {
    Nst_ObjDeclr *objs;
    usize obj_count;
} Nst_DeclrList;

/**
 * Checks the types of the arguments and extracts their values.
 *
 * @brief If you want to check but not get the value of an argument, the
 * pointer in the variable arguments can be NULL. Check the syntax for the
 * types argument in lib_import.h
 *
 * @param types: the string that defines the expected types of the arguments
 * @param arg_num: the number of arguments passed
 * @param args: the arguments to check
 * @param ...: the pointers to the variables where the values extracted are
 * stored and to the custom types
 */
NstEXP bool NstC Nst_extract_arg_values(const i8 *types, usize arg_num,
                                        Nst_Obj **args, ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H
