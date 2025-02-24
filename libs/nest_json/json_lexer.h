#ifndef JSON_LEXER_H
#define JSON_LEXER_H

#include "nest.h"

extern bool comments;
extern bool nan_and_inf;

const bool comments_default = false;
const bool nan_and_inf_default = false;

#define JSON_SYNTAX_ERROR(msg, path, pos)                                     \
    Nst_error_setf_syntax(                                                    \
        "JSON: " msg ", file \"%s\", line %lli, column %lli",                 \
        path, (i64)(pos).line, (i64)(pos).col)

typedef enum _JSONTokenType {
    JSON_LBRACKET,
    JSON_RBRACKET,
    JSON_LBRACE,
    JSON_RBRACE,
    JSON_COMMA,
    JSON_COLON,
    JSON_VALUE,
    JSON_EOF
} JSONTokenType;

Nst_LList *json_tokenize(i8 *path, i8 *text, usize text_len,
                         bool readonly_text, Nst_EncodingID encoding);

#endif // !JSON_LEXER_H
