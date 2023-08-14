/**
 * @file argv_parser.h
 *
 * @brief Compiler from AST to bytecode
 *
 * @author TheSilvered
 */

#ifndef ARGV_PARSER_H
#define ARGV_PARSER_H

#include "encoding.h"

#ifdef Nst_WIN
#include <windows.h>
#endif // !Nst_WIN

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A structure representing the command-line arguments of Nest.
 *
 * @brief Note: command and filename cannot both be set
 *
 * @param print_tokens: whether the tokens of the program should be printed
 * @param print_ast: whether the AST of the program should be printed
 * @param print_bytecode: whether the bytecode of the program should be printed
 * @param force_execution: whether to execute the program when print_tokens,
 * print_ast or print_bytecode are true
 * @param encoding: the encoding of the file to open, ignored if it is passed
 * through the command
 * @param no_default: whether to initialize the program with default variables
 * such as true, false, Int, Str etc...
 * @param opt_level: the optimization level of the program 0 through 3
 * @param command: the code to execute passed as a command line argument
 * @param filename: the file to execute
 * @param args_start: the index where the arguments for the Nest program start
 */
typedef struct _Nst_CLArgs {
    bool print_tokens, print_ast, print_bytecode;
    bool force_execution;
    Nst_CPID encoding;
    bool no_default;
    i32 opt_level;
    i8 *command, *filename;
    i32 args_start;
} Nst_CLArgs;

/**
 * Parses command-line arguments.
 *
 * @brief All arguments are expected to be valid and the pointers cannot be
 * NULL.
 *
 * @param argc: length of argv
 * @param argv: the command-line arguments array
 * @param print_tokens: set to true when the -t option is used or to false
 * otherwise
 * @param print_ast: to true when the -a option is used or to false otherwise
 * @param print_bytecode: set to true when the -b option is used or to false
 * otherwise
 * @param force_execution: set to true when the -f option is used or to false
 * otherwise
 * @param encoding: set to the specified encoding or to Nst_CP_UNKNOWN if not
 * given
 * @param no_default: set to true when the -D option is used or to false
 * otherwise
 * @param opt_level: set to the specified optimization level or to 3 if not
 * given
 * @param command: set to the value given when usint the -c option or to NULL
 * if not specified
 * @param filename: set to the file path given or to NULL if -c is used
 * @param args_start: the index where the arguments for the Nest program start
 *
 * @return -1 on failure, 0 on success where the program can continue, 1 on
 * success when the program should stop because a message was printed
 */
NstEXP i32 NstC _Nst_parse_args(i32 argc, i8 **argv, Nst_CLArgs *cl_args);

/**
 * @return returns true if ANSI escapes are supported on the current console
 * and false otherwise
 */
NstEXP bool Nst_supports_color(void);

#ifdef Nst_WIN

/**
 * WINDOWS ONLY Re-encodes Unicode artuments to UTF-8.
 *
 * @brief All arguments are expected to be valid and the pointers cannot be
 * NULL.
 *
 * @param argc: the length of wargv
 * @param wargv: the arguments given
 * @param argv: the pointer where the re-encoded arguments are put
 */
NstEXP bool NstC _Nst_wargv_to_argv(int argc, wchar_t **wargv, i8 ***argv);

/* WINDOWS-ONLY Initialises the console */
NstEXP void NstC _Nst_set_console_mode(void);

#endif // !Nst_WIN

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ARGV_PARSER_H
