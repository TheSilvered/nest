/**
 * @file parser.h
 *
 * @brief Token parser into AST.
 *
 * @author TheSilvered
 */

#ifndef PARSER_H
#define PARSER_H

#include "nodes.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Parses a list of tokens into an abstract syntax tree.
 *
 * @brief The list of tokens is destroyed when calling this function.
 *
 * @param tokens: the tokens to be parsed
 * @param error: a pointer to put the error into
 *
 * @return The AST or NULL on failure. No operation error is set.
 */
NstEXP Nst_Node *NstC Nst_parse(Nst_LList *tokens, Nst_Error *error);
/* Prints the AST to the Nest standart output. */
NstEXP void NstC Nst_print_ast(Nst_Node *ast);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !PARSER_H
