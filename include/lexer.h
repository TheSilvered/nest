/* Lexer & Tokenizer for the code */

#ifndef LEXER_H
#define LEXER_H

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Returns an LList of tokens given the path to a file
Nst_LList *nst_tokenizef(i8             *filename,
                         bool            force_cp1252,
                         i32            *opt_level,
                         bool           *no_default,
                         Nst_SourceText *src_text,
                         Nst_Error      *error);
// Returns an LList of tokens given the text, it's length and a filename
Nst_LList *nst_tokenize(Nst_SourceText *text, Nst_Error *error);

void nst_add_lines(Nst_SourceText *text);
void nst_normalize_encoding(Nst_SourceText *text,
                            bool            is_cp1252,
                            Nst_Error      *error);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LEXER_H
