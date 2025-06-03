/**
 * @file parser.h
 *
 * @brief Token parser into AST
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
 * Parse a list of tokens into an abstract syntax tree.
 *
 * @param tokens: the tokens to be parsed
 *
 * @return The AST or `NULL` on failure. The error is set.
 */
NstEXP Nst_Node *NstC Nst_parse(Nst_DynArray *tokens);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !PARSER_H
