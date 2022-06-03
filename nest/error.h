#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>
#include "llist.h"

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
char *format_idx_error(const char *format, size_t idx, size_t seq_len);

#define SYNTAX_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = "Syntax Error"; \
    error->message = msg

#define MEMORY_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = "Memory Error"; \
    error->message = msg

#define TYPE_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = "Type Error"; \
    error->message = msg

#define VALUE_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = "Value Error"; \
    error->message = msg

#define GENERAL_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = "Unknown Error"; \
    error->message = msg

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
#define EXPECTED_COLON "expected ':'"
#define EXPECTED_TYPE(type) "expected type ' " type "', got '%s' instead"
#define UNHASHABLE_TYPE "unhashable type '%s'"
#define INDEX_OUT_OF_BOUNDS "index %zi out of bounds for sequence of size %zi"

#endif // !ERROR_H
