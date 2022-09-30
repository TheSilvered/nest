#ifndef ARGV_PARSER_H
#define ARGV_PARSER_H

#include <stdbool.h>

// -1: failed parsing
// 1: parsed succesfully, can stop the program
// 0: parsed succesfully, can continue the program
int nst_parse_args(int argc, char **argv,
                   bool *print_tokens,
                   bool *print_ast,
                   bool *print_bytecode,
                   bool *force_execution,
                   int  *opt_level,
                   char **command,
                   char **filename);

#endif