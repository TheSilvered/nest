#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>
#include <stdint.h>
#include "llist.h"

#define SYNTAX_ERROR (char *)"Syntax Error"
#define MEMORY_ERROR (char *)"Memory Error"
#define VALUE_ERROR (char *)"Value Error"
#define TYPE_ERROR (char *)"Type Error"
#define CALL_ERROR (char *)"Call Error"
#define MATH_ERROR (char *)"Math Error"
#define GENERAL_ERROR (char *)"Unknown Error"

#define CSYNTAX_ERROR "Syntax Error"
#define CMEMORY_ERROR "Memory Error"
#define CVALUE_ERROR "Value Error"
#define CTYPE_ERROR "Type Error"
#define CCALL_ERROR "Call Error"
#define CMATH_ERROR "Math Error"
#define CGENERAL_ERROR "Unknown Error"

// error messages
#define UNEXPECTED_NEWLINE "unescaped line feed not allowed on single-line strings"
#define INVALID_SYMBOL "invalid symbol"
#define INT_TOO_BIG "int literal's value is too large"
#define REAL_TOO_BIG "real literal's value is too large"
#define BAD_REAL_LITEARL "invalid real literal"
#define INVALID_ESCAPE "invalid escape sequence"
#define UNCLOSED_STR_LITERAL "string literal was never closed"
#define UNEXPECTED_TOK "unexpected token"
#define MISSING_BRACKET "unmatched opening braket"
#define BAD_RETURN "'=>' ouside of a function"
#define BAD_CONTINUE "'..' outside of a loop"
#define BAD_BREAK "';' outside of a loop"
#define EXPECTED_BRACKET "expected '['"
#define EXPECTED_IDENT "expected identifier"
#define EXPECTED_VALUE "expected value"
#define INVALID_EXPRESSION "invalid expression"
#define EXPECTED_OP "expected stack or local stack operator"
#define EXPECTED_IDENT_OR_EXTR "expected identifier or extraction"
#define MISSING_PAREN "unmatched parenthesis"
#define MISSING_VECTOR_BRACE "unmatched vector brace"
#define EXPECTED_COMMA_OR_BRACE "expected ',' or '}'"
#define EXPECTED_BRACE "expected '}'"
#define EXPECTED_COLON "expected ':'"
#define EXPECTED_TYPE(type) "expected type '" type "', got '%s' instead"
#define UNHASHABLE_TYPE "unhashable type '%s'"
#define INDEX_OUT_OF_BOUNDS(type) "index %lli out of bounds for '" type "' of size %zi"
#define TOO_MANY_ARGS(name) "too many arguments were given to" name
#define TOO_FEW_ARGS(name) "too few arguments were given to" name
#define TOO_MANY_ARGS_FUNC "too many arguments were passed to the function"
#define TOO_FEW_ARGS_FUNC "too few arguments were passed to the function"
#define ZERO_RANGE_STEP "step must not be zero"
#define EXPECTED_BOOL_ITER_IS_DONE "expected 'Bool' type from ':is_done:', got type '%s' instead"
#define FILE_NOT_FOUND "file '%s' not found"
#define FILE_NOT_DLL "the file was not a valid DLL"
#define NO_LIB_INIT "the DLL does not specify a 'lib_init' function"
#define NO_GET_FUNC_PTRS "the DLL does not specify a 'get_func_ptrs' function"
#define WRONG_TYPE_FOR_ARG(type) "expected type '" type "' for argument %zi, got type '%s' instead"
#define ARG_NUM_DOESNT_MATCH "argument number doesn't match to the given types"
#define INCVALID_TYPE_LETTER "invalid letter in type string"
#define RAN_OUT_OF_MEMORY "ran out of memory"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct Pos
{
    size_t line;
    size_t col;
    char *filename;
    char *text;
    size_t text_len;
}
Pos;

typedef struct Nst_Error
{
    Pos start;
    Pos end;
    char *name;
    char *message;
}
Nst_Error;

typedef struct OpErr
{
    char *name;
    char *message;
}
OpErr;

typedef struct Nst_Traceback
{
    Nst_Error *error;
    LList *positions;
}
Nst_Traceback;

Pos copy_pos(Pos pos);
void print_error(Nst_Error err);
void print_traceback(Nst_Traceback tb);

char *format_type_error(const char *format, char *type_name);
char *format_idx_error(const char *format, int64_t idx, size_t seq_len);
char *format_fnf_error(const char *format, char *file_name);
char *format_arg_error(const char *format, char *type_name, size_t idx);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
