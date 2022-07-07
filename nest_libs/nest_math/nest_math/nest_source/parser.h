#ifndef Nst_PARSER_H
#define Nst_PARSER_H

#include "nodes.h"
#include "llist.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

Node *parse(LList *tokens);
void print_ast(Node *ast);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_PARSER_H