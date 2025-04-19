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

NstEXP Nst_SourceText *NstC Nst_source_load(Nst_CLArgs *inout_args);
NstEXP Nst_SourceText *NstC Nst_source_from_sv(Nst_StrView sv);
NstEXP Nst_SourceText *NstC Nst_source_from_file(const char *path,
                                                 Nst_EncodingID encoding);

NstEXP void NstC Nst_source_text_destroy(Nst_SourceText *text);

#ifdef __cplusplus
}
#endif

#endif // !SOURCE_LOADER_H