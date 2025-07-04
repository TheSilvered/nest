/**
 * @file argv_parser.h
 *
 * @brief Command-line arguments parser
 *
 * @author TheSilvered
 */

#ifndef ARGV_PARSER_H
#define ARGV_PARSER_H

#include "encoding.h"

#ifdef Nst_MSVC
#include <windows.h>
#endif // !Nst_MSVC

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A structure representing the command-line arguments of Nest.
 *
 * @brief Note: `command` and `filename` cannot both be set.
 *
 * @param print_tokens: whether the tokens of the program should be printed
 * @param print_ast: whether the AST of the program should be printed
 * @param print_instructions: whether the instructions of the program should be
 * printed
 * @param print_bytecode: whether the bytecode of the program should be printed
 * @param force_execution: whether to execute the program when `print_tokens`,
 * `print_ast` or `print_bytecode` are true
 * @param encoding: the encoding of the file to open, ignored if it is passed
 * through the command
 * @param no_default: whether to initialize the program with default variables
 * such as `true`, `false`, `Int`, `Str` etc...
 * @param opt_level: the optimization level of the program 0 through 3
 * @param command: the code to execute passed as a command line argument
 * @param filename: the file to execute
 * @param args_start: the index where the arguments for the Nest program start
 */
NstEXP typedef struct _Nst_CLArgs {
    bool print_tokens, print_ast, print_instructions, print_bytecode;
    bool force_execution;
    bool no_default;
    u8 opt_level;
    Nst_EncodingID encoding;
    i32 args_start;
    i32 argc;
    char **argv;
    char *command, *filename;
} Nst_CLArgs;

/**
 * Initialize a `Nst_CLArgs` struct with default values.
 *
 * @param args: the struct to initialize
 * @param argc: the number of arguments passed to main
 * @param argv: the array of arguments passed to main
 */
NstEXP void NstC Nst_cl_args_init(Nst_CLArgs *args, int argc, char **argv);

/**
 * Parse command-line arguments.
 *
 * @param cl_args: the struct where to put the parsed arguments, it must be
 * initialized with `Nst_cl_args_init`
 *
 * @return `-1` on failure, `0` on success where the program can continue, `1`
 * on success when the program should stop because an info message was printed.
 */
NstEXP i32 NstC Nst_cl_args_parse(Nst_CLArgs *cl_args);

/**
 * @return `true` if ANSI escapes are supported on the current console and
 * `false` otherwise.
 */
NstEXP bool NstC Nst_supports_color(void);

/* Overrides the value returned by `Nst_supports_color`. */
NstEXP void NstC _Nst_supports_color_override(bool value);

#ifdef Nst_MSVC

/**
 * WINDOWS ONLY Re-encode Unicode (UTF-16) arguments to UTF-8.
 *
 * @param argc: the length of `wargv`
 * @param wargv: the arguments given
 * @param argv: the pointer where the re-encoded arguments are put
 *
 * @return `true` on success and `false` on failure. No error is set.
 */
NstEXP bool NstC _Nst_wargv_to_argv(int argc, wchar_t **wargv, char ***argv);

/* WINDOWS ONLY Initialize the console. */
NstEXP void NstC _Nst_console_mode_init(void);

#endif // !Nst_MSVC

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ARGV_PARSER_H
