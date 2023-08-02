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
#define _Nst_EM_INT_TOO_BIG "Int literal's value is too large"
#define _Nst_EM_EXPECTED_IF "expected '?'"
#define _Nst_EM_EXPECTED_OP "expected stack or local stack operator"
#define _Nst_EM_FILE_NOT_SO "the file is not a valid shared object"
#define _Nst_EM_COMPLEX_POW "fractional power of a negative number"
#define _Nst_EM_CIRC_IMPORT "circular import"
#define _Nst_EM_REAL_TOO_BIG "Real literal's value is too large"
#define _Nst_EM_INVALID_CHAR "invalid character"
#define _Nst_EM_OPEN_COMMENT "multiline comment was never closed"
#define _Nst_EM_BAD_CONTINUE "'..' outside of a loop"
#define _Nst_EM_FILE_NOT_DLL "the file is not a valid DLL"
#define _Nst_EM_BAD_INT_BASE "the base must be between 2 and 36"
#define _Nst_EM_INVALID_CPID "the encoding selected is not supported"
#define _Nst_EM_FAILED_CHDIR "failed to change the current working directory"
#define _Nst_EM_MISSING_PAREN "unmatched '('"
#define _Nst_EM_FAILED_GETCWD "failed to get the current working directory"
#define _Nst_EM_INVALID_SYMBOL "invalid symbol"
#define _Nst_EM_INVALID_ESCAPE "invalid escape sequence"
#define _Nst_EM_UNEXPECTED_TOK "unexpected token"
#define _Nst_EM_EXPECTED_IDENT "expected an identifier"
#define _Nst_EM_EXPECTED_VALUE "expected a value"
#define _Nst_EM_MISSING_VBRACE "unmatched '<{'"
#define _Nst_EM_EXPECTED_BRACE "expected '}'"
#define _Nst_EM_EXPECTED_COLON "expected ':'"
#define _Nst_EM_EXPECTED_CATCH "expected '?!'"
#define _Nst_EM_MODULO_BY_ZERO "modulo by zero"
#define _Nst_EM_EXPECTED_TYPES "expected type '%s', got '%s' instead"
#define _Nst_EM_FILE_NOT_FOUND "file '%s' not found"
#define _Nst_EM_BAD_INT_LITERAL "invalid Int literal"
#define _Nst_EM_MISSING_BRACKET "unmatched '['"
#define _Nst_EM_EXPECTED_VBRACE "expected '}>'"
#define _Nst_EM_RANGE_STEP_ZERO "the step cannot be zero"
#define _Nst_EM_LIB_INIT_FAILED "the module failed to initialize"
#define _Nst_EM_INVALID_CASTING "invalid type cast from '%s' to '%s'"
#define _Nst_EM_UNHASHABLE_TYPE "unhashable type '%s'"
#define _Nst_EM_BAD_BYTE_LITERAL "invalid Byte literal"
#define _Nst_EM_BAD_REAL_LITERAL "invalid Real literal"
#define _Nst_EM_OPEN_STR_LITERAL "string literal was never closed"
#define _Nst_EM_EXPECTED_BRACKET "expected '['"
#define _Nst_EM_DIVISION_BY_ZERO "division by zero"
#define _Nst_EM_INVALID_ENCODING "could not decode byte %ib for %s encoding"
#define _Nst_EM_RAN_OUT_OF_MEMORY "ran out of memory"
#define _Nst_EM_EXPECTED_R_BRACKET "expected ']'"
#define _Nst_EM_INVALID_EXPRESSION "invalid expression"
#define _Nst_EM_MAP_TO_SEQ_HASH(i) "unhashable object found at " i " %zi"
#define _Nst_EM_MISSING_FUNC(func) "missing '" func "' for a custom iterator"
#define _Nst_EM_NULL_ARG(func, argn) func ": " argn " is NULL"
#define _Nst_EM_INVALID_TYPE_LETTER(f) f ": invalid type string"
#define _Nst_EM_EXPECTED_COMMA_OR_BRACE "expected ',' or '}'"
#define _Nst_EM_EXPECTED_RETURN_OR_BRACKET "expected '=>' or '['"
#define _Nst_EM_INVALID_OPERAND_TYPE(operand)                                 \
    "invalid type '%s' for '" operand "'"
#define _Nst_EM_EXPECTED_IDENT_OR_EXTR                                        \
    "expected an identifier or an extraction"
#define _Nst_EM_EXPECTED_TYPE(type)                                           \
    "expected type '" type "', got '%s' instead"
#define _Nst_EM_COMPOUND_ASSIGMENT                                            \
    "cannot unpack values in a compound assigment"
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
    "the maximum call stack size (1000 calls) was exceeded"
#define _Nst_EM_LEFT_ARGS_NUM(op, count, s)                                   \
    "'" op "' expects only " count " argument" s " on the left"
#define _Nst_EM_WRONG_TYPE_FOR_ARG2                                           \
    "expected type '%s' for argument %zi, got type '%s' instead"
#define _Nst_EM_EXPECTED_BOOL_ITER_IS_DONE                                    \
    "expected 'Bool' type from '_is_done_', got type '%s' instead"
#define _Nst_EM_WRONG_ARG_NUM                                                 \
    "the function expected at most %zi argument%s but %lli %s passed"
#define _Nst_EM_WRONG_TYPE_FOR_ARG(type)                                      \
    "expected type '" type "' for argument %zi, got type '%s' instead"
#define _Nst_EM_WRONG_UNPACK_LENGTH                                           \
    "expected %lli items to unpack but the sequence only contains %zi"
#define _Nst_EM_MAP_TO_SEQ_LEN_ERR(i)                                         \
    "expected a sequence of length 2, found one of length %zi instead at " i  \
    " %zi"
#define _Nst_EM_MAP_TO_SEQ_TYPE_ERR(i)                                        \
    "expected each element to be a Vector or an Array, found a '%s' instead " \
    "at " i " %zi"

/* Alias for _Nst_set_error that casts name and msg to Nst_StrObj * */
#define Nst_set_error(name, msg)  _Nst_set_error(STR(name), STR(msg))
/* Alias for _Nst_set_syntax_error that casts msg to Nst_StrObj * */
#define Nst_set_syntax_error(msg) _Nst_set_syntax_error(STR(msg))
/* Alias for _Nst_set_memory_error that casts msg to Nst_StrObj * */
#define Nst_set_memory_error(msg) _Nst_set_memory_error(STR(msg))
/* Alias for _Nst_set_type_error that casts msg to Nst_StrObj * */
#define Nst_set_type_error(msg)   _Nst_set_type_error(STR(msg))
/* Alias for _Nst_set_value_error that casts msg to Nst_StrObj * */
#define Nst_set_value_error(msg)  _Nst_set_value_error(STR(msg))
/* Alias for _Nst_set_math_error that casts msg to Nst_StrObj * */
#define Nst_set_math_error(msg)   _Nst_set_math_error(STR(msg))
/* Alias for _Nst_set_call_error that casts msg to Nst_StrObj * */
#define Nst_set_call_error(msg)   _Nst_set_call_error(STR(msg))
/* Alias for _Nst_set_import_error that casts msg to Nst_StrObj * */
#define Nst_set_import_error(msg) _Nst_set_import_error(STR(msg))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The structure where the source text of a Nest file is kept.
 *
 * @param text: the UTF-8 text of the file
 * @param path: the path of the file
 * @param lines: the beginning of each line of the file
 * @param len: the length in bytes of the text
 * @param line_count: the number of lines in the file
 */
NstEXP typedef struct _Nst_SourceText {
    i8 *text;
    i8 *path;
    i8 **lines;
    usize len;
    usize line_count;
} Nst_SourceText;

/**
 * The structure representing a position inside a source file
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
 * The structure representing an error with a determined position
 *
 * @param occurred: whether the struct contains a valid error
 * @param start: the start position of the error
 * @param end: the end position of the error
 * @param name: the name of the error (e.g. Value Error, Type Error ecc.)
 * @param message: the message of the error
 */
NstEXP typedef struct _Nst_Error {
    bool occurred;
    Nst_Pos start;
    Nst_Pos end;
    Nst_StrObj *name;
    Nst_StrObj *message;
} Nst_Error;

/**
 * The structure representing an error occurred during an operation and that
 * does not yet have a position
 *
 * @param name: the name of the error (e.g. Value Error, Type Error ecc.)
 * @param message: the message of the error
 */
NstEXP typedef struct _Nst_OpErr {
    Nst_StrObj *name;
    Nst_StrObj *message;
} Nst_OpErr;

/**
 * The structure containing the full traceback of the error.
 *
 * @param error: the error of the traceback
 * @param positions: the list of positions that led to the error
 */
NstEXP typedef struct _Nst_Traceback {
    Nst_Error error;
    Nst_LList *positions;
} Nst_Traceback;

/* Sets how the error message is printed (with or without ANSI escapes) */
NstEXP void NstC Nst_set_color(bool color);
/* Forces a copy of the position */
NstEXP Nst_Pos NstC Nst_copy_pos(Nst_Pos pos);
/* Creates an empty position, with no valid text */
NstEXP Nst_Pos NstC Nst_no_pos(void);
/* Prints a formatted Nst_Error */
NstEXP void NstC Nst_print_error(Nst_Error err);
/* Prints a formatted Nst_Trackeback */
NstEXP void NstC Nst_print_traceback(Nst_Traceback tb);

/* Frees a heap allocated text source, text can be NULL. No error is set. */
NstEXP void NstC Nst_free_src_text(Nst_SourceText *text);

/**
 * Sets the global operation error with the given name and message
 *
 * @brief It takes a reference of both name and message.
 */
NstEXP void NstC _Nst_set_error(Nst_StrObj *name, Nst_StrObj *msg);
/**
 * Sets the global operation error with the given message and "Syntax Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC _Nst_set_syntax_error(Nst_StrObj *msg);
/**
 * Sets the global operation error with the given message and "Memory Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC _Nst_set_memory_error(Nst_StrObj *msg);
/**
 * Sets the global operation error with the given message and "Type Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC _Nst_set_type_error(Nst_StrObj *msg);
/**
 * Sets the global operation error with the given message and "Value Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC _Nst_set_value_error(Nst_StrObj *msg);
/**
 * Sets the global operation error with the given message and "Math Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC _Nst_set_math_error(Nst_StrObj *msg);
/**
 * Sets the global operation error with the given message and "Call Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC _Nst_set_call_error(Nst_StrObj *msg);
/**
 * Sets the global operation error with the given message and "Import Error"
 * as the name.
 *
 * @brief It takes a reference of the message.
 */
NstEXP void NstC _Nst_set_import_error(Nst_StrObj *msg);

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

/* Returns whether the global operation error is set. */
NstEXP bool NstC Nst_error_occurred(void);
/* Returns the global operation error, it does not create a copy. */
NstEXP Nst_OpErr *NstC Nst_error_get(void);
/* Clears the global operation error, even if it is not set. */
NstEXP void NstC Nst_error_clear(void);

/* Initializes the traceback of Nst_state. */
NstEXP bool NstC Nst_traceback_init(void);
/* Frees the traceback of Nst_state. */
NstEXP void NstC Nst_traceback_delete(void);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
