/* Command-line arguments parser */

#ifndef ARGV_PARSER_H
#define ARGV_PARSER_H

#include "typedefs.h"

// Parses the command line arguments
// Return values:
// * -1 fail
// * 0 success
// * 1 success, stop the program (retured when an info message is printed)
EXPORT
i32 _nst_parse_args(i32 argc, i8 **argv,
                    bool *print_tokens,
                    bool *print_ast,
                    bool *print_bytecode,
                    bool *force_execution,
                    bool *monochrome,
                    bool *force_cp1252,
                    bool *no_default,
                    i32  *opt_level,
                    i8  **command,
                    i8  **filename,
                    i32  *args_start);

#ifdef WINDOWS
EXPORT bool _nst_wargv_to_argv(int       argc,
                               wchar_t **wargv,
                               i8     ***argv,
                               i8      **argv_content);
EXPORT void _nst_set_console_mode();
#endif // !WINDOWS

#endif // !ARGV_PARSER_H
