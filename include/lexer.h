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
 * Opens and tokenizes a file.
 *
 * @param filename: the path to the file
 * @param encoding: the supposed encoding of the file, if set to
 * `Nst_EID_UNKNOWN` it will be detected automatically
 * @param opt_level: where the optimization level is stored if specified with
 * file arguments
 * @param no_default: where the `--no-default` option is stored if specified
 * with file arguments
 * @param src_text: where the source of the file is saved
 *
 * @return A `Nst_DynArray` of `Nst_Tok`. On failure it has length `0`. The
 * error is set.
 */
NstEXP Nst_DynArray NstC Nst_tokenizef(const char *filename,
                                       Nst_EncodingID encoding, i32 *opt_level,
                                       bool *no_default,
                                       Nst_SourceText *src_text);
/**
 * Tokenizes text.
 *
 * @param text: the text to tokenize
 *
 * @return A `Nst_DynArray` of `Nst_Tok`. On failure it has length `0`. The
 * error is set.
 */
NstEXP Nst_DynArray NstC Nst_tokenize(Nst_SourceText *text);

/**
 * Adds the `lines` array to the given text.
 *
 * @brief On failure the lines field of the struct is set to `NULL` and
 * lines_len to `0`.
 *
 * @param text: the text to add the line starts to
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_add_lines(Nst_SourceText *text);

/**
 * Re-encodes a file to be UTF-8.
 *
 * @param text: the text to re-encode
 * @param encoding: the encoding of the text, if `Nst_EID_UNKNOWN` it is
 * detected automatically
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_normalize_encoding(Nst_SourceText *text,
                                        Nst_EncodingID encoding);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LEXER_H
