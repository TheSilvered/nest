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

#endif
