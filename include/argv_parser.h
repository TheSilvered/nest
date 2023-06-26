/* Command-line arguments parser */

#ifndef ARGV_PARSER_H
#define ARGV_PARSER_H

#include "encoding.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Parses the command line arguments
// Return values:
// * -1 fail
// * 0 success
// * 1 success, stop the program (retured when an info message is printed)
EXPORT i32 _nst_parse_args(i32 argc, i8 **argv,
                           bool     *print_tokens,
                           bool     *print_ast,
                           bool     *print_bytecode,
                           bool     *force_execution,
                           Nst_CPID *encoding,
                           bool     *no_default,
                           i32      *opt_level,
                           i8      **command,
                           i8      **filename,
                           i32      *args_start);

EXPORT bool nst_supports_color(void);

#ifdef WINDOWS
EXPORT bool _nst_wargv_to_argv(int       argc,
                               wchar_t **wargv,
                               i8     ***argv,
                               i8      **argv_content);
EXPORT void _nst_set_console_mode(void);

usize _nst_windows_stdin_read(i8 *buf, usize size, usize count, void *f_value);

#endif // !WINDOWS

#ifdef __cplusplus
}
#endif

#endif // !ARGV_PARSER_H
