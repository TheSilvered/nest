/* Error interface */

#ifndef ERROR_H
#define ERROR_H

#include "str.h"
#include "llist.h"

// error messages

#define _NST_EM_INVALID_CHAR "invalid character"
#define _NST_EM_UNEXPECTED_NEWLINE "single-quote strings cannot stretch " \
    "multiple lines"
#define _NST_EM_INVALID_SYMBOL "invalid symbol"
#define _NST_EM_INT_TOO_BIG "Int literal's value is too large"
#define _NST_EM_REAL_TOO_BIG "Real literal's value is too large"
#define _NST_EM_BAD_INT_LITERAL "invalid Int literal"
#define _NST_EM_BAD_BYTE_LITERAL "invalid Byte literal"
#define _NST_EM_BAD_REAL_LITERAL "invalid Real literal"
#define _NST_EM_INVALID_ESCAPE "invalid escape sequence"
#define _NST_EM_OPEN_STR_LITERAL "string literal was never closed"
#define _NST_EM_OPEN_COMMENT "multiline comment was never closed"
#define _NST_EM_UNEXPECTED_TOK "unexpected token"
#define _NST_EM_MISSING_BRACKET "unmatched '['"
#define _NST_EM_BAD_RETURN "'=>' outside of a function"
#define _NST_EM_BAD_CONTINUE "'..' outside of a loop"
#define _NST_EM_BAD_BREAK "';' outside of a loop"
#define _NST_EM_EXPECTED_BRACKET "expected '['"
#define _NST_EM_EXPECTED_RETURN_OR_BRACKET "expected '=>' or '['"
#define _NST_EM_EXPECTED_R_BRACKET "expected ']'"
#define _NST_EM_EXPECTED_IF "expected '?'"
#define _NST_EM_EXPECTED_IDENT "expected an identifier"
#define _NST_EM_EXPECTED_VALUE "expected a value"
#define _NST_EM_INVALID_EXPRESSION "invalid expression"
#define _NST_EM_EXPECTED_OP "expected stack or local stack operator"
#define _NST_EM_EXPECTED_IDENT_OR_EXTR "expected an identifier or an extraction"
#define _NST_EM_MISSING_PAREN "unmatched '('"
#define _NST_EM_MISSING_VBRACE "unmatched '<{'"
#define _NST_EM_EXPECTED_COMMA_OR_BRACE "expected ',' or '}'"
#define _NST_EM_EXPECTED_BRACE "expected '}'"
#define _NST_EM_EXPECTED_VBRACE "expected '}>'"
#define _NST_EM_EXPECTED_COLON "expected ':'"
#define _NST_EM_EXPECTED_CATCH "expected '?!'"
#define _NST_EM_RANGE_STEP_ZERO "the step cannot be zero"
#define _NST_EM_FILE_NOT_DLL "the file is not a valid DLL"
#define _NST_EM_FILE_NOT_SO "the file is not a valid shared object"
#define _NST_EM_ARG_NUM_DOESNT_MATCH(f) f ": the number of arguments and " \
    "types does not match"
#define _NST_EM_INVALID_TYPE_LETTER(f) f ": invalid type string"
#define _NST_EM_RAN_OUT_OF_MEMORY "ran out of memory"
#define _NST_EM_NEGATIVE_SIZE_FOR_SEQ "the length of the sequence cannot be negative"
#define _NST_EM_CALL_STACK_SIZE_EXCEEDED "the maximum call stack size (1000 " \
    "calls) was exceeded"
#define _NST_EM_DIVISION_BY_ZERO "division by zero"
#define _NST_EM_MODULO_BY_ZERO "modulo by zero"
#define _NST_EM_COMPLEX_POW "fractional power of a negative number"
#define _NST_EM_INVALID_CASTING "invalid type cast from '%s' to '%s'"
#define _NST_EM_CIRC_IMPORT "circular import"
#define _NST_EM_LIB_INIT_FAILED "the module failed to initialize"
#define _NST_EM_COMPOUND_ASSIGMENT "cannot unpack values in a compound assigment"
#define _NST_EM_BAD_INT_BASE "the base must be between 2 and 36"
#define _NST_EM_INVALID_CPID "the encoding selected is not supported"

#define _NST_EM_EXPECTED_TYPE(type) "expected type '" type "', got '%s' instead"
#define _NST_EM_EXPECTED_TYPES "expected type '%s', got '%s' instead"
#define _NST_EM_UNHASHABLE_TYPE "unhashable type '%s'"
#define _NST_EM_INDEX_OUT_OF_BOUNDS(type) "index %lli out of bounds for '" \
    type "' of size %zi"
#define _NST_EM_LEFT_ARGS_NUM(op, count, s) "'" op "' expects only " count \
    " argument" s " on the left"
#define _NST_EM_WRONG_ARG_NUM "the function expected at most %zi argument%s " \
    "but %lli %s passed"
#define _NST_EM_EXPECTED_BOOL_ITER_IS_DONE "expected 'Bool' type from " \
    "'_is_done_', got type '%s' instead"
#define _NST_EM_FILE_NOT_FOUND "file '%s' not found"
#define _NST_EM_WRONG_TYPE_FOR_ARG(type) "expected type '" type "' for " \
    "argument %zi, got type '%s' instead"
#define _NST_EM_WRONG_TYPE_FOR_ARG2 "expected type '%s' for argument %zi, got " \
    "type '%s' instead"
#define _NST_EM_MISSING_FUNC(func) "missing '" func "' for a custom iterator"
#define _NST_EM_INVALID_OPERAND_TYPE(operand) "invalid type '%s' for '" operand "'"
#define _NST_EM_NO_LIB_FUNC(func) "the library does not specify a '" func "' function"
#define _NST_EM_WRONG_UNPACK_LENGTH "expected %lli items to unpack but the " \
    "sequence only contains %zi"
#define _NST_EM_MAP_TO_SEQ_TYPE_ERR(i) "expected each element to be a Vector " \
    "or an Array, found a '%s' instead at " i " %zi"
#define _NST_EM_MAP_TO_SEQ_LEN_ERR(i) "expected a sequence of length 2, found " \
    "one of length %zi instead at " i " %zi"
#define _NST_EM_MAP_TO_SEQ_HASH(i) "unhashable object found at " i " %zi"
#define _NST_EM_INVALID_ENCODING "could not decode byte %ib for %s encoding"
#define _NST_EM_INVALID_DECODING "could not decode code point U+%06X for %s encoding"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef struct _Nst_SourceText
{
    i8 *text;
    i8 *path;
    i8 **lines;
    usize len;
    usize line_count;
}
Nst_SourceText;

EXPORT typedef struct _Nst_Pos
{
    i32 line;
    i32 col;
    Nst_SourceText *text;
}
Nst_Pos;

EXPORT typedef struct _Nst_Error
{
    bool occurred;
    Nst_Pos start;
    Nst_Pos end;
    Nst_StrObj *name;
    Nst_StrObj *message;
}
Nst_Error;

EXPORT typedef struct _Nst_OpErr
{
    Nst_StrObj *name;
    Nst_StrObj *message;
}
Nst_OpErr;

EXPORT typedef struct _Nst_Traceback
{
    Nst_Error error;
    Nst_LList *positions;
}
Nst_Traceback;

// Sets how the error message is printed (with or without ANSI color escapes)
EXPORT void nst_set_color(bool color);
// Creates a new Nst_Pos on the stack
EXPORT Nst_Pos nst_copy_pos(Nst_Pos pos);
// An Nst_Pos that is empty, (col=0, line=0, filename=NULL, text=NULL, text_len=0)
EXPORT Nst_Pos nst_no_pos(void);
// Prints a formatted Nst_Error
EXPORT void nst_print_error(Nst_Error err);
// Prints a formatted Nst_Trackeback
EXPORT void nst_print_traceback(Nst_Traceback tb);

// Frees a heap allocated text source
EXPORT void nst_free_src_text(Nst_SourceText *text);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
