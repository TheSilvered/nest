/**
 * @file source_loader.h
 *
 * @brief Functions for loading Nest source files.
 *
 * @author TheSilvered
 */

#ifndef SOURCE_LOADER_H
#define SOURCE_LOADER_H

#include "argv_parser.h"
#include "str_view.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The structure where the source text of a Nest file is kept.
 *
 * @param text: the UTF-8 text of the file
 * @param path: the path of the file
 * @param lines: the beginning of each line of the file
 * @param len: the length in bytes of `text`
 * @param lines_len: the number of lines in the file
 */
NstEXP typedef struct _Nst_SourceText {
    char *text;
    char *path;
    char **lines;
    usize text_len;
    usize lines_len;
} Nst_SourceText;

bool _Nst_source_loader_init(void);
void _Nst_source_loader_quit(void);

/**
 * Load a `Nst_SourceText` from command line arguments.
 *
 * @brief The resulting text is managed by Nest and does not need to be freed.
 *
 * @return The source text of the specified file or command, encoded in UTF-8.
 * On failure the function returns `NULL` and the error is set.
 */
NstEXP Nst_SourceText *NstC Nst_source_load(Nst_CLArgs *inout_args);
/**
 * Load a `Nst_SourceText` from a string view.
 *
 * @brief The resulting text is heap allocated and must be destroyed with
 * `Nst_source_text_destroy`.
 *
 * @brief Warning: do not use this function to load code that is to be executed
 * by Nest. Use `Nst_source_load` instead, putting the source in the `command`
 * field of `Nst_CLArgs`.
 *
 * @param sv: the string view from which the contents are taken to be used in
 * the source text, it is expected to be encoded in UTF-8
 *
 * @return The source text created with the contents of `sv`, encoded in UTF-8.
 * On failure the function returns `NULL` and the error is set.
 */
NstEXP Nst_SourceText *NstC Nst_source_from_sv(Nst_StrView sv);
/**
 * Load a `Nst_SourceText` from a file.
 *
 * @brief The resulting text is heap allocated and must be destroyed with
 * `Nst_source_text_destroy`.
 *
 * @brief Warning: do not use this function to load code that is to be executed
 * by Nest. Use `Nst_source_load` instead, putting the path in the `filename`
 * field and the encoding in the `encoding` field of `Nst_CLArgs`.
 *
 * @param path: the path to the file
 * @param encoding: the encoding to use when reading the file, if it is set to
 * `Nst_EID_UNKNOWN` it will be determined automatically
 *
 * @return The source text created with the contents of `sv`, encoded in UTF-8.
 * On failure the function returns `NULL` and the error is set.
 */
NstEXP Nst_SourceText *NstC Nst_source_from_file(const char *path,
                                                 Nst_EncodingID encoding);

/* Destroy the contents of a `Nst_SourceText` and free it. */
NstEXP void NstC Nst_source_text_destroy(Nst_SourceText *text);

#ifdef __cplusplus
}
#endif

#endif // !SOURCE_LOADER_H