/* Parser for the token list */

#ifndef Nst_PARSER_H
#define Nst_PARSER_H

#include "nodes.h"
#include "llist.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Parses the token list
Nst_Node *nst_parse(LList *tokens, Nst_Error *error);
// Prints the AST like using the -a flag
void nst_print_ast(Nst_Node *ast);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_PARSER_H