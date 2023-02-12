/* Lexer & Tokenizer for the code */

#ifndef Nst_LEXER_H
#define Nst_LEXER_H

#include "llist.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Returns an LList of tokens given the path to a file
Nst_LList *nst_tokenizef(char           *filename,
                     bool            force_cp1252,
                     int            *opt_level,
                     bool           *no_default,
                     Nst_SourceText *src_text,
                     Nst_Error      *error);
// Returns an LList of tokens given the text, it's length and a filename
Nst_LList *nst_tokenize(Nst_SourceText *text, Nst_Error *error);

void nst_add_lines(Nst_SourceText *text);
void nst_normalize_encoding(Nst_SourceText *text,
                            bool            is_cp1252,
                            Nst_Error      *error);
int nst_check_utf8_bytes(unsigned char *bytes, size_t len);
int nst_cp1252_to_utf8(char *str, char byte);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_LEXER_H
