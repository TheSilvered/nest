/* Parser for the token list */

#ifndef PARSER_H
#define PARSER_H

#include "nodes.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Parses the token list
NstEXP Nst_Node *nst_parse(Nst_LList *tokens, Nst_Error *error);
// Prints the AST like using the -a flag
NstEXP void nst_print_ast(Nst_Node *ast);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !PARSER_H
