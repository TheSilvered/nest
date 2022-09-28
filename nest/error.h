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
#define UNEXPECTED_NEWLINE "unescaped line feed not allowed on single-line strings"
#define INVALID_SYMBOL "invalid symbol"
#define INT_TOO_BIG "Int literal's value is too large"
#define REAL_TOO_BIG "Real literal's value is too large"
#define BAD_REAL_LITEARL "invalid Real literal"
#define INVALID_ESCAPE "invalid escape sequence"
#define UNCLOSED_STR_LITERAL "string literal was never closed"
#define UNCLOSED_COMMENT "multiline comment was never closed"
#define UNEXPECTED_TOK "unexpected token"
#define MISSING_BRACKET "unmatched '['"
#define BAD_RETURN "'=>' ouside of a function"
#define BAD_CONTINUE "'..' outside of a loop"
#define BAD_BREAK "';' outside of a loop"
#define EXPECTED_BRACKET "expected '['"
#define EXPECTED_RETURN "expected '=>'"
#define EXPECTED_R_BRACKET "expected ']'"
#define EXPECTED_QUESTION_MARK "expected '?'"
#define EXPECTED_IDENT "expected identifier"
#define EXPECTED_VALUE "expected value"
#define INVALID_EXPRESSION "invalid expression"
#define EXPECTED_OP "expected stack or local stack operator"
#define EXPECTED_IDENT_OR_EXTR "expected identifier or extraction"
#define MISSING_PAREN "unmatched '('"
#define MISSING_VECTOR_BRACE "unmatched '<{'"
#define EXPECTED_COMMA_OR_BRACE "expected ',' or '}'"
#define EXPECTED_BRACE "expected '}'"
#define EXPECTED_COLON "expected ':'"
#define EXPECTED_TYPE(type) "expected type '" type "', got '%s' instead"
#define EXPECTED_TYPES "expected type '%s', got '%s' instead"
#define UNHASHABLE_TYPE "unhashable type '%s'"
#define INDEX_OUT_OF_BOUNDS(type) "index %lli out of bounds for '" type "' of size %zi"
#define TOO_MANY_ARGS(name) "too many arguments were given to '" name "'"
#define TOO_FEW_ARGS(name) "too few arguments were given to '" name "'"
#define TOO_MANY_ARGS_FUNC "too many arguments were passed to the function"
#define TOO_FEW_ARGS_FUNC "too few arguments were passed to the function"
#define ZERO_RANGE_STEP "step must not be zero"
#define EXPECTED_BOOL_ITER_IS_DONE "expected 'Bool' type from '_is_done_', got type '%s' instead"
#define FILE_NOT_FOUND "file '%s' not found"
#define FILE_NOT_DLL "the file was not a valid DLL"
#define NO_LIB_INIT "the DLL does not specify a 'lib_init' function"
#define NO_GET_FUNC_PTRS "the DLL does not specify a 'get_func_ptrs' function"
#define WRONG_TYPE_FOR_ARG(type) "expected type '" type "' for argument %zi, got type '%s' instead"
#define ARG_NUM_DOESNT_MATCH "argument number doesn't match to the given types"
#define INCVALID_TYPE_LETTER "invalid letter in type string"
#define RAN_OUT_OF_MEMORY "ran out of memory"
#define NEGATIVE_SIZE_FOR_SEQUENCE "the specified quantity is negative"
#define CALL_STACK_SIZE_EXCEEDED "the maximum call stack size (1000 calls) was exceeded"

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
    Nst_Pos start;
    Nst_Pos end;
    char *name;
    char *message;
}
Nst_Error;

typedef struct
{
    char *name;
    char *message;
}
Nst_OpErr;

typedef struct
{
    Nst_Error *error;
    LList *positions;
}
Nst_Traceback;

Nst_Pos nst_copy_pos(Nst_Pos pos);
Nst_Pos nst_no_pos();
void nst_print_error(Nst_Error err);
void nst_print_traceback(Nst_Traceback tb);

char *_nst_format_type_error(const char *format, char *type_name);
char *_nst_format_types_error(const char *format, char *type_name1, char *type_name2);
char *_nst_format_idx_error(const char *format, int64_t idx, size_t seq_len);
char *_nst_format_fnf_error(const char *format, char *file_name);
char *_nst_format_arg_error(const char *format, char *type_name, size_t idx);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
