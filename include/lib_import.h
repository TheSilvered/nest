/**
 * @file lib_import.h
 *
 * @brief C/C++ library utilities
 *
 * @author TheSilvered
 */

/* [docs:ignore]
  Usage of the 'types' argument:

  Builtin types:
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

  Other:
    'o': any object

  Whitespace:
    Whitespace inside the type string is mostly ignored but there cannot be
    any whitespace characters after the last type.

    "i s v" -> OK
    "i s v " -> Error

  Multiple types per argument:
    To specifiy more than one type that an argument can be, use a pipe (|)
    between the various types.

    Nst_Obj *var;
    Nst_DEF_EXTRACT("i|s", &var); // 'var' accepts only Int or Str objects

  Custom types:
    You can have any number of custom types in a single argument and you should
    label them with #. The custom type(s) are to be passed to the function
    before the pointer to the variable in the order in which they appear.

    Nst_Obj *var;
    Nst_DEF_EXTRACT("#|#|#", custom_type1, custom_type2, custom_type3, &var);

  Optional types:
    To have an optional type you can use ? before the type itself. Optional
    types allow for either the specified types or Null to be acceppted. Because
    of this writing ?i and i|n is the same thing.

    Nst_IOFileObj *file;
    Nst_DEF_EXTRACT("?F", &file); // 'file' can be either a File or NULL object

  Casting to other Nst_Obj:
    By following the selected type(s) with a colon (:) you can specify the type
    that the object should be casted to after it has been checked with exactly
    one letter. This letter can be any of the builtin types except 'n'. Using
    'o' as the type to cast will only increase the reference of the argument.
    In fact, when casting an object with this method a new reference is always
    put inside the given variable that needs dereferencing when no longer in
    use.

    Nst_IntObj *num;
    Nst_DEF_EXTRACT("i|B:i", &num); // 'num' accepts a Int and Byte objects but
                                    // it will always contain an Int

  Casting to C types:
    If you follow the selcted type(s) with an underscore (_), you can extract
    the value of the argument into a C value. This method only accepts 'i',
    'r', 'b' and 'B' and cannot be used when casting to another Nst_Obj.

    bool opt;
    Nst_DEF_EXTRACT("i_b", &opt); // 'opt' only accepts Int objects but
                                  // will always contain a boolean

  Implicit casting to C types:
    If a type is specified as only one of 'i', 'r', 'b' or 'B' it is
    automatically translated to 'i_i', 'r_r', 'b_b' and 'B_B' respectively.
    The values are extracted to the following C types:
    - i -> i64
    - r -> f64
    - b -> bool
    - B -> u8

    i64 int_num;
    f64 real_num;
    Nst_DEF_EXTRACT("i r", &int_num, &real_num);
    // even though only 'i' and 'r' are specified, the extracted values are C
    // types

  Sequence type checking:
    You can additionally check the types present inside the matched sequence
    by following the type with a dot (.). This will not throw an error if the
    type to check is not an Array or Vector object. Note that this kind of
    checking cannot occurr if the argument is casted to a C type.

    Nst_SeqObj *array_of_ints;
    Nst_DEF_EXTRACT("?a.i|B" &array_of_ints);
    // 'array_of_ints' can be either an Array or Null object. If it is the
    // former its elements can only be Int and Byte objects.

  Shorthands:
    There are some shorthands that reduce the complexity of the type string by
    representing commonly used types into a single character.

    'l': i|B_i
    'N': i|r|B_r
    'A': a|v
    'S': a|v|s:a
    'R': I|a|v|s:I
    'y': o_b

    The types that cast their value to a C type or to another Nst_Obj are
    omitted when the shorthand is used when cheking the contents of a sequence
    and can be overwritten by manual casts.

  Example:
    "N_B ?A.#|i" -> An Int, Real, Byte all casted to a u8 followed by an
                    optional Array or Vector that, if it exists, should contain
                    only objects of a custom type or integers.
    "S.s" -> A string, array or vector that casted to an array should contain
             only strings. This can be written in an expanded form as
             "a|v|s:a.s"
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
