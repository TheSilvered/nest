#ifndef Nst_LEXER_H
#define Nst_LEXER_H

#include "llist.h"

// Returns an LList of tokens given the path to a file
LList *ftokenize(char *filename);
// Returns an LList of tokens given the text, it's length and a filename
LList *tokenize(char *text, size_t text_len, char *filename);
#endif // !Nst_LEXER_H
