/* Error interface */

#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>
#include <stdint.h>
#include "str.h"
#include "llist.h"

// error messages
#define _NST_EM_INVALID_CHAR "invalid character"
#define _NST_EM_UNEXPECTED_NEWLINE "unescaped line feed not allowed on single-line strings"
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
#define _NST_EM_BAD_RETURN "'=>' ouside of a function"
#define _NST_EM_BAD_CONTINUE "'..' outside of a loop"
#define _NST_EM_BAD_BREAK "';' outside of a loop"
#define _NST_EM_EXPECTED_BRACKET "expected '['"
#define _NST_EM_EXPECTED_RETURN "expected '=>'"
#define _NST_EM_EXPECTED_R_BRACKET "expected ']'"
#define _NST_EM_EXPECTED_IF "expected '?'"
#define _NST_EM_EXPECTED_IDENT "expected identifier"
#define _NST_EM_EXPECTED_VALUE "expected value"
#define _NST_EM_INVALID_EXPRESSION "invalid expression"
#define _NST_EM_EXPECTED_OP "expected stack or local stack operator"
#define _NST_EM_EXPECTED_IDENT_OR_EXTR "expected identifier or extraction"
#define _NST_EM_MISSING_PAREN "unmatched '('"
#define _NST_EM_MISSING_VBRACE "unmatched '<{'"
#define _NST_EM_EXPECTED_COMMA_OR_BRACE "expected ',' or '}'"
#define _NST_EM_EXPECTED_BRACE "expected '}'"
#define _NST_EM_EXPECTED_COLON "expected ':'"
#define _NST_EM_EXPECTED_CATCH "expected '?!'"
#define _NST_EM_RANGE_STEP_ZERO "step must not be zero"
#define _NST_EM_FILE_NOT_DLL "the file is not a valid DLL"
#define _NST_EM_FILE_NOT_SO "the file is not a valid shared object"
#define _NST_EM_ARG_NUM_DOESNT_MATCH "argument number doesn't match to the given types"
#define _NST_EM_INVALID_TYPE_LETTER "invalid letter in type string"
#define _NST_EM_RAN_OUT_OF_MEMORY "ran out of memory"
#define _NST_EM_NEGATIVE_SIZE_FOR_SEQ "the length of the sequence cannot be negative"
#define _NST_EM_CALL_STACK_SIZE_EXCEEDED "the maximum call stack size (1000 calls) was exceeded"
#define _NST_EM_DIVISION_BY_ZERO "division by zero"
#define _NST_EM_MODULO_BY_ZERO "modulo by zero"
#define _NST_EM_COMPLEX_POW "fractional power of a negative number"
#define _NST_EM_INVALID_CASTING "invalid type cast from '%s' to '%s'"
#define _NST_EM_CIRC_IMPORT "circular import"
#define _NST_EM_LIB_INIT_FAILED "the module failed to initialize"

#define _NST_EM_EXPECTED_TYPE(type) "expected type '" type "', got '%s' instead"
#define _NST_EM_EXPECTED_TYPES "expected type '%s', got '%s' instead"
#define _NST_EM_UNHASHABLE_TYPE "unhashable type '%s'"
#define _NST_EM_INDEX_OUT_OF_BOUNDS(type) "index %lli out of bounds for '" type "' of size %zi"
#define _NST_EM_TOO_MANY_ARGS(name) "too many arguments were passed to '" name "'"
#define _NST_EM_TOO_FEW_ARGS(name) "too few arguments were passed to '" name "'"
#define _NST_EM_WRONG_ARG_NUM "the function expected %zi argument%s but %lli %s passed"
#define _NST_EM_EXPECTED_BOOL_ITER_IS_DONE "expected 'Bool' type from '_is_done_', got type '%s' instead"
#define _NST_EM_FILE_NOT_FOUND "file '%s' not found"
#define _NST_EM_WRONG_TYPE_FOR_ARG(type) "expected type '" type "' for argument %zi, got type '%s' instead"
#define _NST_EM_MISSING_FUNC(func) "missing '" func "' for a custom iterator"
#define _NST_EM_INVALID_OPERAND_TYPE(operand) "invalid type '%s' for '" operand "'"
#define _NST_EM_NO_LIB_FUNC(func) "the library does not specify a '" func "' function"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_SourceText
{
    char *text;
    char *path;
    char **lines;
    size_t len;
    size_t line_count;
}
Nst_SourceText;

typedef struct _Nst_Pos
{
    long line;
    long col;
    Nst_SourceText *text;
}
Nst_Pos;

typedef struct _Nst_Error
{
    bool occurred;
    Nst_Pos start;
    Nst_Pos end;
    Nst_StrObj *name;
    Nst_StrObj *message;
}
Nst_Error;

typedef struct _Nst_OpErr
{
    Nst_StrObj *name;
    Nst_StrObj *message;
}
Nst_OpErr;

typedef struct _Nst_Traceback
{
    Nst_Error error;
    LList *positions;
}
Nst_Traceback;

// Sets how the error message is printed (with or withoud ANSI color escapes)
void nst_set_color(bool color);
// Creates a new Nst_Pos on the stack
Nst_Pos nst_copy_pos(Nst_Pos pos);
// An Nst_Pos that is empty, (col=0, line=0, filename=NULL, text=NULL, text_len=0)
Nst_Pos nst_no_pos();
// Prints a formatted Nst_Error
void nst_print_error(Nst_Error err);
// Prints a formatted Nst_Trackeback
void nst_print_traceback(Nst_Traceback tb);

// Allocates a string on the heap and prints the format
// format_args is the type of the arguments passed, can be:
//   s: char *,
//   u: size_t,
//   i: Nst_Int
Nst_StrObj *_nst_format_error(const char *format, const char *format_args, ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
