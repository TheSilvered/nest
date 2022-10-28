/* Command-line arguments parser */

#ifndef ARGV_PARSER_H
#define ARGV_PARSER_H

#include <stdbool.h>

/* Parses the command line arguments
* Return values:
*   -1 fail
*   0 success
*   1 success, stop the program (retured when the help message is printed)
*/
int nst_parse_args(int argc, char **argv,
                   bool *print_tokens,
                   bool *print_ast,
                   bool *print_bytecode,
                   bool *force_execution,
                   bool *monochrome,
                   int  *opt_level,
                   char **command,
                   char **filename,
                   int  *args_start);

#endif