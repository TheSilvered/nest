/* Lexer & Tokenizer for the code */

#ifndef Nst_LEXER_H
#define Nst_LEXER_H

#include "llist.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Returns an LList of tokens given the path to a file
LList *nst_ftokenize(char           *filename,
                     bool            force_cp1252,
                     Nst_SourceText *src_text,
                     Nst_Error      *error);
// Returns an LList of tokens given the text, it's length and a filename
LList *nst_tokenize(Nst_SourceText *text, Nst_Error *error);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_LEXER_H
