#ifndef JSON_LEXER_H
#define JSON_LEXER_H

#include "nest.h"

extern bool comments;

#define JSON_SYNTAX_ERROR(msg, path, pos) \
    NST_SET_SYNTAX_ERROR(_nst_format_error( \
        "JSON: " msg ", file \"%s\", line %lli, column %lli", \
        "sii", \
        path, (Nst_Int)(pos).line, (Nst_Int)(pos).col));

enum JSONTokenType
{
    JSON_LBRACKET,
    JSON_RBRACKET,
    JSON_LBRACE,
    JSON_RBRACE,
    JSON_COMMA,
    JSON_COLON,
    JSON_VALUE,
    JSON_EOF
};

LList *json_tokenize(char      *path,
                     char      *text,
                     size_t     text_len,
                     bool       fix_encoding,
                     Nst_OpErr *err);

#endif // !JSON_LEXER_H