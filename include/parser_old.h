/* [docs:ignore_file] */
#if 0
/**
 * @file parser_old.h
 *
 * @brief Token parser into AST
 *
 * @author TheSilvered
 */

#ifndef PARSER_OLD_H
#define PARSER_OLD_H

#include "nodes_old.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Parses a list of tokens into an abstract syntax tree.
 *
 * @brief The list of tokens is destroyed when calling this function.
 *
 * @param tokens: the tokens to be parsed
 *
 * @return The AST or `NULL` on failure. The error is set.
 */
NstEXP Nst_Node__old *NstC Nst_parse__old(Nst_LList *tokens);
/* Prints the AST to the Nest standard output. */
NstEXP void NstC Nst_print_ast__old(Nst_Node__old *ast);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !PARSER_OLD_H
#endif
;