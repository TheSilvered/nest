#ifndef JSON_LEXER_H
#define JSON_LEXER_H

#include "nest.h"

extern bool comments;

#define JSON_SYNTAX_ERROR(msg, path, pos) \
    NST_SET_SYNTAX_ERROR(nst_sprintf( \
        "JSON: " msg ", file \"%s\", line %lli, column %lli", \
        path, (Nst_Int)(pos).line, (Nst_Int)(pos).col));

typedef enum _JSONTokenType
{
    JSON_LBRACKET,
    JSON_RBRACKET,
    JSON_LBRACE,
    JSON_RBRACE,
    JSON_COMMA,
    JSON_COLON,
    JSON_VALUE,
    JSON_EOF
}
JSONTokenType;

Nst_LList *json_tokenize(i8        *path,
                         i8        *text,
                         usize      text_len,
                         bool       fix_encoding,
                         Nst_OpErr *err);

#endif // !JSON_LEXER_H