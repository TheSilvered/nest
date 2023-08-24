/**
 * @file iter.h
 *
 * @brief Lexer and tokenizer for Nest code
 *
 * @author TheSilvered
 */

#ifndef LEXER_H
#define LEXER_H

#include "encoding.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Opens and tokenizes a file.
 *
 * @param filename: the path to the file
 * @param encoding: the supposed encoding of the file, if set to
 * `Nst_CP_UNKNOWN` it will be detected automatically
 * @param opt_level: where the optimization level is stored if specified with
 * file arguments
 * @param no_default: where the `--no-default` option is stored if specified
 * with file arguments
 * @param src_text: where the source of the file is saved
 * @param error: where the error is put if it occurs
 *
 * @return A `Nst_LList` of tokens or `NULL` on failure. No error is set.
 */
NstEXP Nst_LList *NstC Nst_tokenizef(i8 *filename, Nst_CPID encoding,
                                     i32 *opt_level, bool *no_default,
                                     Nst_SourceText *src_text,
                                     Nst_Error *error);
/**
 * Tokenizes text.
 *
 * @param text: the text to tokenize
 * @param error: where the error is put if it occurs
 *
 * @return A `Nst_LList` of tokens or `NULL` on failure. No global operation
 * error is set.
 */
NstEXP Nst_LList *NstC Nst_tokenize(Nst_SourceText *text, Nst_Error *error);

/**
 * Adds the `lines` array to the given text.
 *
 * @brief On failure the lines field of the struct is set to `NULL` and
 * lines_len to `0`.
 *
 * @param text: the text to add the line starts to
 *
 * @return `true` on success and `false` on failure. No error is set.
 */
NstEXP bool NstC Nst_add_lines(Nst_SourceText *text);

/**
 * Re-encodes a file to be UTF-8.
 *
 * @param text: the text to re-encode
 * @param encoding: the encoding of the text, if `Nst_CP_UNKNWON` it is
 * detected automatically
 * @param error: where the error is put if it occurs
 *
 * @return `true` on success and `false` on failure. No global operation error
 * is set.
 */
NstEXP bool NstC Nst_normalize_encoding(Nst_SourceText *text, Nst_CPID encoding,
                                        Nst_Error *error);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LEXER_H
