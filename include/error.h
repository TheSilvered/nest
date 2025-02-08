/**
 * @file error.h
 *
 * @brief Error management interface
 *
 * @author TheSilvered
 */
#ifndef ERROR_H
#define ERROR_H

#include "str.h"
#include "llist.h"

// Error messages

#define _Nst_EM_BAD_BREAK "';' outside of a loop"
#define _Nst_EM_BAD_RETURN "'=>' outside of a function"
#define _Nst_EM_NAN_TO_INT "cannot cast a NaN to an Int"
#define _Nst_EM_INF_TO_INT "cannot cast an infinity to an Int"
#define _Nst_EM_EXPECTED_IF "expected '?'"
#define _Nst_EM_CIRC_IMPORT "circular import"
#define _Nst_EM_READ_FAILED "failed to read from the file"
#define _Nst_EM_FILE_CLOSED "the file '%.4096s' was closed"
#define _Nst_EM_INT_TOO_BIG "Int literal's value is too large"
#define _Nst_EM_FILE_NOT_SO "the file is not a valid shared object"
#define _Nst_EM_COMPLEX_POW "fractional power of a negative number"
#define _Nst_EM_EXPECTED_OP "expected stack or local stack operator"
#define _Nst_EM_NAN_TO_BYTE "cannot cast a NaN to a Byte"
#define _Nst_EM_INF_TO_BYTE "cannot cast an infinity to a Byte"
#define _Nst_EM_BAD_CAST(to) "cannot cast an object of type '%s' to '" to "'"
#define _Nst_EM_INVALID_CHAR "invalid character"
#define _Nst_EM_BAD_CONTINUE "'..' outside of a loop"
#define _Nst_EM_FILE_NOT_DLL "the file is not a valid DLL"
#define _Nst_EM_WRITE_FAILED "failed to write to the file"
#define _Nst_EM_REAL_TOO_BIG "Real literal's value is too large"
#define _Nst_EM_BAD_INT_BASE "the base must be between 2 and 36"
#define _Nst_EM_OPEN_COMMENT "multi-line comment was never closed"
#define _Nst_EM_INVALID_CPID "the encoding selected is not supported"
#define _Nst_EM_FAILED_CHDIR "failed to change the current working directory"
#define _Nst_EM_MISSING_PAREN "unmatched '('"
#define _Nst_EM_FAILED_GETCWD "failed to get the current working directory"
#define _Nst_EM_BAD_TYPE_NAME "type names must only contain 7-bit ASCII"
#define _Nst_EM_EXPECTED_BRACE "expected '}'"
#define _Nst_EM_EXPECTED_COLON "expected ':'"
#define _Nst_EM_EXPECTED_CATCH "expected '?!'"
#define _Nst_EM_MISSING_VBRACE "unmatched '<{'"
#define _Nst_EM_INVALID_SYMBOL "invalid symbol"
#define _Nst_EM_MODULO_BY_ZERO "modulo by zero"
#define _Nst_EM_UNEXPECTED_TOK "unexpected token"
#define _Nst_EM_EXPECTED_VALUE "expected a value"
#define _Nst_EM_FILE_NOT_FOUND "file '%s' not found"
#define _Nst_EM_EXPECTED_IDENT "expected an identifier"
#define _Nst_EM_INVALID_ESCAPE "invalid escape sequence"
#define _Nst_EM_EXPECTED_TYPES "expected type '%s', got '%s' instead"
#define _Nst_EM_MISSING_BRACKET "unmatched '['"
#define _Nst_EM_EXPECTED_VBRACE "expected '}>'"
#define _Nst_EM_BAD_INT_LITERAL "invalid Int literal"
#define _Nst_EM_UNHASHABLE_TYPE "type '%s' is not hashable"
#define _Nst_EM_RANGE_STEP_ZERO "the step cannot be zero"
#define _Nst_EM_LIB_INIT_FAILED "the module failed to initialize"
#define _Nst_EM_INVALID_CASTING "invalid type cast from '%s' to '%s'"
#define _Nst_EM_EXPECTED_BRACKET "expected '['"
#define _Nst_EM_DIVISION_BY_ZERO "division by zero"
#define _Nst_EM_BAD_BYTE_LITERAL "invalid Byte literal"
#define _Nst_EM_BAD_REAL_LITERAL "invalid Real literal"
#define _Nst_EM_OPEN_STR_LITERAL "string literal was never closed"
#define _Nst_EM_INVALID_ENCODING "could not decode byte %ib for %s encoding"
#define _Nst_EM_STR_INDEX_FAILED "failed to index string"
#define _Nst_EM_CALL_FAILED(func) "call to '" func "' failed"
#define _Nst_EM_RAN_OUT_OF_MEMORY "ran out of memory"
#define _Nst_EM_MISSING_FUNC(func) "missing '" func "' for a custom iterator"
#define _Nst_EM_MAP_TO_SEQ_HASH(i) "non-hashable object found at " i " %zi"
#define _Nst_EM_RECURSIVE_CALLS(i) "over " i " recursive calls, parsing failed"
#define _Nst_EM_EXPECTED_R_BRACKET "expected ']'"
#define _Nst_EM_INVALID_EXPRESSION "invalid expression"
#define _Nst_EM_NULL_ARG(func, argn) func ": " argn " is NULL"
#define _Nst_EM_INVALID_TYPE_LETTER(f) f ": invalid type string"
#define _Nst_EM_EXPECTED_COMMA_OR_BRACE "expected ',' or '}'"
#define _Nst_EM_EXPECTED_COMMA_OR_VBRACE "expected ',' or '}>'"
#define _Nst_EM_EXPECTED_RETURN_OR_BRACKET "expected '=>' or '['"
#define _Nst_EM_INVALID_OPERAND_TYPE(operand)                                 \
    "invalid type '%s' for '" operand "'"
#define _Nst_EM_EXPECTED_IDENT_OR_EXTR                                        \
    "expected an identifier or an extraction"
#define _Nst_EM_EXPECTED_TYPE(type)                                           \
    "expected type '" type "', got '%s' instead"
#define _Nst_EM_COMPOUND_ASSIGMENT                                            \
    "cannot unpack values in a compound assignment"
#define _Nst_EM_NEGATIVE_SIZE_FOR_SEQ                                         \
    "the length of the sequence cannot be negative"
#define _Nst_EM_UNEXPECTED_NEWLINE                                            \
    "single-quote strings cannot stretch multiple lines"
#define _Nst_EM_NO_LIB_FUNC(func)                                             \
    "the library does not specify a '" func "' function"
#define _Nst_EM_INVALID_DECODING                                              \
    "could not decode code point U+%06X for %s encoding"
#define _Nst_EM_INDEX_OUT_OF_BOUNDS(type)                                     \
    "index %lli out of bounds for '" type "' of size %zi"
#define _Nst_EM_ARG_NUM_DOESNT_MATCH(f)                                       \
    f ": the number of arguments and types does not match"
#define _Nst_EM_CALL_STACK_SIZE_EXCEEDED                                      \
    "the maximum call stack size (%zi calls) was exceeded"
#define _Nst_EM_LEFT_ARGS_NUM(op, count, s)                                   \
    "'" op "' expects only " count " argument" s " on the left"
#define _Nst_EM_WRONG_TYPE_FOR_ARG2                                           \
    "expected type '%s' for argument %zi, got type '%s' instead"
#define _Nst_EM_WRONG_ARG_NUM                                                 \
    "the function expected at most %zi argument%s but %lli %s passed"
#define _Nst_EM_WRONG_TYPE_FOR_ARG(type)                                      \
    "expected type '" type "' for argument %zi, got type '%s' instead"
#define _Nst_EM_WRONG_UNPACK_LENGTH                                           \
    "expected %lli items to unpack but the sequence contains %zi"
#define _Nst_EM_MAP_TO_SEQ_LEN_ERR(i)                                         \
    "expected a sequence of length 2, found one of length %zi instead at " i  \
    " %zi"
#define _Nst_EM_MAP_TO_SEQ_TYPE_ERR(i)                                        \
    "expected each element to be a Vector or an Array, found a '%s' instead " \
    "at " i " %zi"

/* [docs:ignore_sym _Nst_EM_WRONG_ARG_NUM] */

/* Correctly formats the `_Nst_EM_WRONG_ARG_NUM` error message. */
#define _Nst_EM_WRONG_ARG_NUM_FMT(func_arg_num, arg_num)                \
    Nst_sprintf(                                                              \
        _Nst_EM_WRONG_ARG_NUM,                                                \
        func_arg_num, func_arg_num == 1 ? "" : "s",                           \
        arg_num, arg_num == 1 ? "was" : "were")

/**
 * @brief Alias for `_Nst_set_error` that casts `name` to `Nst_StrObj *` and
 * builds a formatted string with `fmt`.
 */
#define Nst_set_errorf(name, fmt, ...)                                        \
    Nst_set_error(name, Nst_sprintf(fmt, __VA_ARGS__))
/**
 * @brief Alias for `_Nst_set_syntax_error` that builds a formatted string with
 * `fmt`.
 */
#define Nst_set_syntax_errorf(fmt, ...)                                       \
    Nst_set_syntax_error(Nst_sprintf(fmt, __VA_ARGS__))
/**
 * @brief Alias for `_Nst_set_memory_error` that builds a formatted string with
 * `fmt`.
 */
#define Nst_set_memory_errorf(fmt, ...)                                       \
    Nst_set_memory_error(Nst_sprintf(fmt, __VA_ARGS__))
/**
 * @brief Alias for `_Nst_set_type_error` that builds a formatted string with
 * `fmt`.
 */
#define Nst_set_type_errorf(fmt, ...)                                         \
    Nst_set_type_error(Nst_sprintf(fmt, __VA_ARGS__))
/**
 * @brief Alias for `_Nst_set_value_error` that builds a formatted string with
 * `fmt`.
 */
#define Nst_set_value_errorf(fmt, ...)                                        \
    Nst_set_value_error(Nst_sprintf(fmt, __VA_ARGS__))
/**
 * @brief Alias for `_Nst_set_math_error` that builds a formatted string with
 * `fmt`.
 */
#define Nst_set_math_errorf(fmt, ...)                                         \
    Nst_set_math_error(Nst_sprintf(fmt, __VA_ARGS__))
/**
 * @brief Alias for `_Nst_set_call_error` that builds a formatted string with
 * `fmt`.
 */
#define Nst_set_call_errorf(fmt, ...)                                         \
    Nst_set_call_error(Nst_sprintf(fmt, __VA_ARGS__))
/**
 * @brief Alias for `_Nst_set_import_error` that builds a formatted string with
 * `fmt`.
 */
#define Nst_set_import_errorf(fmt, ...)                                       \
    Nst_set_import_error(Nst_sprintf(fmt, __VA_ARGS__))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The structure where the source text of a Nest file is kept.
 *
 * @param allocated: whether `text`, `path` and `lines` are heap allocated
 * @param text: the UTF-8 text of the file
 * @param path: the path of the file
 * @param lines: the beginning of each line of the file
 * @param len: the length in bytes of `text`
 * @param lines_len: the number of lines in the file
 */
NstEXP typedef struct _Nst_SourceText {
    bool allocated;
    i8 *text;
    i8 *path;
    i8 **lines;
    usize text_len;
    usize lines_len;
} Nst_SourceText;

/**
 * The structure representing a position inside a source file.
 *
 * @param line: the line of the position, the first is line 0
 * @param col: the column of the position, the first is 0
 * @param text: the text this position refers to
 */
NstEXP typedef struct _Nst_Pos {
    i32 line;
    i32 col;
    Nst_SourceText *text;
} Nst_Pos;

/**
 * The structure containing the full traceback of the error.
 *
 * @param error_occurred: whether the traceback contains an error
 * @param error_name: the name of the error (e.g. `Value Error`, `Type Error`
 * etc.)
 * @param error_msg: the message of the error
 * @param positions: the list of positions that led to the error
 */
NstEXP typedef volatile struct _Nst_Traceback {
    bool error_occurred;
    Nst_Obj *error_name;
    Nst_Obj *error_msg;
    Nst_LList *positions;
} Nst_Traceback;

/* Sets how the error message is printed (with or without ANSI escapes). */
NstEXP void NstC Nst_set_color(bool color);
/* Forces a copy of the position. */
NstEXP Nst_Pos NstC Nst_copy_pos(Nst_Pos pos);
/* Creates an empty position, with no valid text. */
NstEXP Nst_Pos NstC Nst_no_pos(void);
/* Prints a formatted `Nst_Traceback`. */
NstEXP void NstC Nst_print_traceback(Nst_Traceback *tb);

/* Initializes the fields of a `Nst_SourceText` struct. */
NstEXP void NstC Nst_source_text_init(Nst_SourceText *src);
/**
 * @brief Frees a heap allocated text source, `text` can be `NULL`. No error
 * is set.
 */
NstEXP void NstC Nst_source_text_destroy(Nst_SourceText *text);

/**
 * Sets the global operation error with the given name and message.
 *
 * @brief It takes a reference of both name and message.
 */
NstEXP void NstC Nst_set_error(Nst_Obj *name, Nst_Obj *msg);
/**
 * Sets the global operation error with the given message and "Syntax Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC Nst_set_syntax_error(Nst_Obj *msg);
/**
 * Sets the global operation error with the given message and "Memory Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC Nst_set_memory_error(Nst_Obj *msg);
/**
 * Sets the global operation error with the given message and "Type Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC Nst_set_type_error(Nst_Obj *msg);
/**
 * Sets the global operation error with the given message and "Value Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC Nst_set_value_error(Nst_Obj *msg);
/**
 * Sets the global operation error with the given message and "Math Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC Nst_set_math_error(Nst_Obj *msg);
/**
 * Sets the global operation error with the given message and "Call Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC Nst_set_call_error(Nst_Obj *msg);
/**
 * Sets the global operation error with the given message and "Import Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC Nst_set_import_error(Nst_Obj *msg);

/**
 * @brief Sets the global operation error creating a string object from the
 * given message and using "Syntax Error" as the name.
 */
NstEXP void NstC Nst_set_syntax_error_c(const i8 *msg);
/**
 * @brief Sets the global operation error creating a string object from the
 * given message and using "Memory Error" as the name.
 */
NstEXP void NstC Nst_set_memory_error_c(const i8 *msg);
/**
 * @brief Sets the global operation error creating a string object from the
 * given message and using "Type Error" as the name.
 */
NstEXP void NstC Nst_set_type_error_c(const i8 *msg);
/**
 * @brief Sets the global operation error creating a string object from the
 * given message and using "Value Error" as the name.
 */
NstEXP void NstC Nst_set_value_error_c(const i8 *msg);
/**
 * @brief Sets the global operation error creating a string object from the
 * given message and using "Math Error" as the name.
 */
NstEXP void NstC Nst_set_math_error_c(const i8 *msg);
/**
 * @brief Sets the global operation error creating a string object from the
 * given message and using "Call Error" as the name.
 */
NstEXP void NstC Nst_set_call_error_c(const i8 *msg);
/**
 * @brief Sets the global operation error creating a string object from the
 * given message and using "Import Error" as the name.
 */
NstEXP void NstC Nst_set_import_error_c(const i8 *msg);

/* Sets the global operation error with a memory error of failed allocation. */
NstEXP void NstC Nst_failed_allocation(void);

/* [docs:ignore_sym Nst_EK_NONE] */
/* [docs:ignore_sym Nst_EK_LOCAL] */
/* [docs:ignore_sym Nst_EK_GLOBAL] */
/**
 * The kinds of errors that can occur during execution.
 *
 * @param Nst_EK_NONE: no error occurred
 * @param Nst_EK_LOCAL: an error has occurred in the current execution state
 * @param Nst_EK_GLOBAL: an error has occurred while no execution state was set
 */
typedef enum _Nst_ErrorKind {
    Nst_EK_NONE = 0,
    Nst_EK_LOCAL,
    Nst_EK_GLOBAL
} Nst_ErrorKind;

/**
 * @return Whether an error has occurred in the current execution. Check
 * `Nst_ErrorKind` to understand better the return value.
 */
NstEXP Nst_ErrorKind NstC Nst_error_occurred(void);
/**
 * @return The traceback of the current execution or that of the interpreter if
 * no execution state is set.
 */
NstEXP Nst_Traceback *NstC Nst_error_get(void);
/**
 * @brief Clears the traceback of both the current execution and of the
 * interpreter. If no error has occurred nothing is done.
 */
NstEXP void NstC Nst_error_clear(void);

/* Initializes the traceback of the current `Nst_state`. */
NstEXP bool NstC Nst_traceback_init(Nst_Traceback *tb);
/* Frees the traceback of the current `Nst_state`. */
NstEXP void NstC Nst_traceback_destroy(Nst_Traceback *tb);
/* Adds a pair of positions to an error. */
NstEXP void NstC Nst_error_add_positions(Nst_Traceback *tb,
                                         Nst_Pos start, Nst_Pos end);

/**
 * Sets a `Nst_Traceback` with the given fields.
 *
 * @brief `name` will increment the reference count but `msg` will not.
 *
 * @param error: the error to fill
 * @param start: the start position
 * @param end: the end position
 * @param msg: the message of the error
 */
NstEXP void NstC Nst_set_internal_error(Nst_Traceback *tb, Nst_Pos start,
                                        Nst_Pos end, Nst_Obj *name,
                                        Nst_Obj *msg);
/**
 * Sets a `Nst_Traceback` with the given fields and creating the message from a
 * C string.
 *
 * @brief `name` will increment the reference count.
 *
 * @param error: the error to fill
 * @param start: the start position
 * @param end: the end position
 * @param msg: the message of the error
 */
NstEXP void NstC Nst_set_internal_error_c(Nst_Traceback *tb, Nst_Pos start,
                                          Nst_Pos end, Nst_Obj *name,
                                          const i8 *msg);

/* Similar to `Nst_set_internal_error` and uses "Syntax Error" as the name. */
NstEXP void NstC Nst_set_internal_syntax_error(Nst_Traceback *tb, Nst_Pos start,
                                               Nst_Pos end, Nst_Obj *msg);
/* Similar to `Nst_set_internal_error` and uses "Memory Error" as the name. */
NstEXP void NstC Nst_set_internal_memory_error(Nst_Traceback *tb, Nst_Pos start,
                                               Nst_Pos end, Nst_Obj *msg);
/* Similar to `Nst_set_internal_error` and uses "Type Error" as the name. */
NstEXP void NstC Nst_set_internal_type_error(Nst_Traceback *tb, Nst_Pos start,
                                             Nst_Pos end, Nst_Obj *msg);
/* Similar to `Nst_set_internal_error` and uses "Value Error" as the name. */
NstEXP void NstC Nst_set_internal_value_error(Nst_Traceback *tb, Nst_Pos start,
                                              Nst_Pos end, Nst_Obj *msg);
/* Similar to `Nst_set_internal_error` and uses "Math Error" as the name. */
NstEXP void NstC Nst_set_internal_math_error(Nst_Traceback *tb, Nst_Pos start,
                                             Nst_Pos end, Nst_Obj *msg);
/* Similar to `Nst_set_internal_error` and uses "Call Error" as the name. */
NstEXP void NstC Nst_set_internal_call_error(Nst_Traceback *tb, Nst_Pos start,
                                             Nst_Pos end, Nst_Obj *msg);
/* Similar to `Nst_set_internal_error` and uses "Import Error" as the name. */
NstEXP void NstC Nst_set_internal_import_error(Nst_Traceback *tb, Nst_Pos start,
                                               Nst_Pos end, Nst_Obj *msg);
/**
 * @brief Similar to `Nst_set_internal_error_c` and uses "Syntax Error" as the
 * name.
 */
NstEXP void NstC Nst_set_internal_syntax_error_c(Nst_Traceback *tb,
                                                 Nst_Pos start, Nst_Pos end,
                                                 const i8 *msg);
/**
 * @brief Similar to `Nst_set_internal_error_c` and uses "Memory Error" as the
 * name.
 */
NstEXP void NstC Nst_set_internal_memory_error_c(Nst_Traceback *tb,
                                                 Nst_Pos start, Nst_Pos end,
                                                 const i8 *msg);
/**
 * @brief Similar to `Nst_set_internal_error_c` and uses "Type Error" as the
 * name.
 */
NstEXP void NstC Nst_set_internal_type_error_c(Nst_Traceback *tb,
                                               Nst_Pos start, Nst_Pos end,
                                               const i8 *msg);
/**
 * @brief Similar to `Nst_set_internal_error_c` and uses "Value Error" as the
 * name.
 */
NstEXP void NstC Nst_set_internal_value_error_c(Nst_Traceback *tb,
                                                Nst_Pos start, Nst_Pos end,
                                                const i8 *msg);
/**
 * @brief Similar to `Nst_set_internal_error_c` and uses "Math Error" as the
 * name.
 */
NstEXP void NstC Nst_set_internal_math_error_c(Nst_Traceback *tb,
                                               Nst_Pos start, Nst_Pos end,
                                               const i8 *msg);
/**
 * @brief Similar to `Nst_set_internal_error_c` and uses "Call Error" as the
 * name.
 */
NstEXP void NstC Nst_set_internal_call_error_c(Nst_Traceback *tb,
                                               Nst_Pos start, Nst_Pos end,
                                               const i8 *msg);
/**
 * @brief Similar to `Nst_set_internal_error_c` and uses "Import Error" as the
 * name.
 */
NstEXP void NstC Nst_set_internal_import_error_c(Nst_Traceback *tb,
                                                 Nst_Pos start, Nst_Pos end,
                                                 const i8 *msg);
/* Sets a memory error for a failed allocation. */
NstEXP void NstC Nst_internal_failed_allocation(Nst_Traceback *tb,
                                                Nst_Pos start, Nst_Pos end);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
