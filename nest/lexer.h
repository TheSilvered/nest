#ifndef Nst_LEXER_H
#define Nst_LEXER_H

#include "llist.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Returns an LList of tokens given the path to a file
LList *nst_ftokenize(char *filename);
// Returns an LList of tokens given the text, it's length and a filename
LList *nst_tokenize(char *text, size_t text_len, char *filename);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_LEXER_H
