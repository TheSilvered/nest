#ifndef JSON_LEXER_H
#define JSON_LEXER_H

#include "nest.h"

extern bool comments;
extern bool split_strings;

enum JSONTokenType
{
    JSON_LBRACKET,
    JSON_RBRACKET,
    JSON_LBRACE,
    JSON_RBRACE,
    JSON_COMMA,
    JSON_COLON,
    JSON_VALUE
};

LList *tokenize(char      *path,
                char      *text,
                size_t     text_len,
                bool       fix_encoding,
                Nst_OpErr *err);

#endif // !JSON_LEXER_H