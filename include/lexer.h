/**
 * @file lexer.h
 *
 * @brief Lexer and tokenizer for Nest code
 *
 * @author TheSilvered
 */

#ifndef LEXER_H
#define LEXER_H

#include "encoding.h"
#include "dyn_array.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Tokenizes text.
 *
 * @param text: the text to tokenize
 *
 * @return A `Nst_DynArray` of `Nst_Tok`. On failure it has length `0`. The
 * error is set.
 */
NstEXP Nst_DynArray NstC Nst_tokenize(Nst_SourceText *text);


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LEXER_H
