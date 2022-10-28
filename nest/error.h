/* Error interface */

#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>
#include <stdint.h>
#include "llist.h"

#define NST_E_SYNTAX_ERROR (char *)"Syntax Error"
#define NST_E_MEMORY_ERROR (char *)"Memory Error"
#define NST_E_VALUE_ERROR (char *)"Value Error"
#define NST_E_TYPE_ERROR (char *)"Type Error"
#define NST_E_CALL_ERROR (char *)"Call Error"
#define NST_E_MATH_ERROR (char *)"Math Error"
#define NST_E_GENERAL_ERROR (char *)"Unknown Error"

#define NST_E_CSYNTAX_ERROR "Syntax Error"
#define NST_E_CMEMORY_ERROR "Memory Error"
#define NST_E_CVALUE_ERROR "Value Error"
#define NST_E_CTYPE_ERROR "Type Error"
#define NST_E_CCALL_ERROR "Call Error"
#define NST_E_CMATH_ERROR "Math Error"
#define NST_E_CGENERAL_ERROR "Unknown Error"

// error messages
#define _NST_EM_UNEXPECTED_NEWLINE (char *)"unescaped line feed not allowed on single-line strings"
#define _NST_EM_INVALID_SYMBOL (char *)"invalid symbol"
#define _NST_EM_INT_TOO_BIG (char *)"Int literal's value is too large"
#define _NST_EM_REAL_TOO_BIG (char *)"Real literal's value is too large"
#define _NST_EM_BAD_REAL_LITEARL (char *)"invalid Real literal"
#define _NST_EM_INVALID_ESCAPE (char *)"invalid escape sequence"
#define _NST_EM_UNCLOSED_STR_LITERAL (char *)"string literal was never closed"
#define _NST_EM_UNCLOSED_COMMENT (char *)"multiline comment was never closed"
#define _NST_EM_UNEXPECTED_TOK (char *)"unexpected token"
#define _NST_EM_MISSING_BRACKET (char *)"unmatched '['"
#define _NST_EM_BAD_RETURN (char *)"'=>' ouside of a function"
#define _NST_EM_BAD_CONTINUE (char *)"'..' outside of a loop"
#define _NST_EM_BAD_BREAK (char *)"';' outside of a loop"
#define _NST_EM_EXPECTED_BRACKET (char *)"expected '['"
#define _NST_EM_EXPECTED_RETURN (char *)"expected '=>'"
#define _NST_EM_EXPECTED_R_BRACKET (char *)"expected ']'"
#define _NST_EM_EXPECTED_QUESTION_MARK (char *)"expected '?'"
#define _NST_EM_EXPECTED_IDENT (char *)"expected identifier"
#define _NST_EM_EXPECTED_VALUE (char *)"expected value"
#define _NST_EM_INVALID_EXPRESSION (char *)"invalid expression"
#define _NST_EM_EXPECTED_OP (char *)"expected stack or local stack operator"
#define _NST_EM_EXPECTED_IDENT_OR_EXTR (char *)"expected identifier or extraction"
#define _NST_EM_MISSING_PAREN (char *)"unmatched '('"
#define _NST_EM_MISSING_VECTOR_BRACE (char *)"unmatched '<{'"
#define _NST_EM_EXPECTED_COMMA_OR_BRACE (char *)"expected ',' or '}'"
#define _NST_EM_EXPECTED_BRACE (char *)"expected '}'"
#define _NST_EM_EXPECTED_COLON (char *)"expected ':'"
#define _NST_EM_EXPECTED_TYPE(type) (char *)("expected type '" type "', got '%s' instead")
#define _NST_EM_EXPECTED_TYPES (char *)"expected type '%s', got '%s' instead"
#define _NST_EM_UNHASHABLE_TYPE (char *)"unhashable type '%s'"
#define _NST_EM_INDEX_OUT_OF_BOUNDS(type) (char *)("index %lli out of bounds for '" type "' of size %zi")
#define _NST_EM_TOO_MANY_ARGS(name) (char *)("too many arguments were given to '" name "'")
#define _NST_EM_TOO_FEW_ARGS(name) (char *)("too few arguments were given to '" name "'")
#define _NST_EM_WRONG_ARG_NUM (char *)"the function expected %zi arguments but %lli were passed"
#define _NST_EM_ZERO_RANGE_STEP (char *)"step must not be zero"
#define _NST_EM_EXPECTED_BOOL_ITER_IS_DONE (char *)"expected 'Bool' type from '_is_done_', got type '%s' instead"
#define _NST_EM_FILE_NOT_FOUND (char *)"file '%s' not found"
#define _NST_EM_FILE_NOT_DLL (char *)"the file is not a valid DLL"
#define _NST_EM_FILE_NOT_SO (char *)"the file is not a valid shared object"
#define _NST_EM_NO_LIB_INIT (char *)"the DLL does not specify a 'lib_init' function"
#define _NST_EM_NO_GET_FUNC_PTRS (char *)"the DLL does not specify a 'get_func_ptrs' function"
#define _NST_EM_WRONG_TYPE_FOR_ARG(type) (char *)("expected type '" type "' for argument %zi, got type '%s' instead")
#define _NST_EM_ARG_NUM_DOESNT_MATCH (char *)"argument number doesn't match to the given types"
#define _NST_EM_INCVALID_TYPE_LETTER (char *)"invalid letter in type string"
#define _NST_EM_RAN_OUT_OF_MEMORY (char *)"ran out of memory"
#define _NST_EM_NEGATIVE_SIZE_FOR_SEQUENCE (char *)"the specified quantity is negative"
#define _NST_EM_CALL_STACK_SIZE_EXCEEDED (char *)"the maximum call stack size (1000 calls) was exceeded"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    long line;
    long col;
    char *filename;
    char *text;
    size_t text_len;
}
Nst_Pos;

typedef struct
{
    bool occurred;
    Nst_Pos start;
    Nst_Pos end;
    char *name;
    char *message;
}
Nst_Error;

typedef struct
{
    const char *name;
    const char *message;
}
Nst_OpErr;

typedef struct
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
char *_nst_format_error(const char *format, const char *format_args, ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
