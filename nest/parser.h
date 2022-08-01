#ifndef Nst_PARSER_H
#define Nst_PARSER_H

#include "nodes.h"
#include "llist.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

Nst_Node *nst_parse(LList *tokens);
void nst_print_ast(Nst_Node *ast);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_PARSER_H