#ifndef Nst_PARSER_H
#define Nst_PARSER_H

#include "nodes.h"
#include "llist.h"

Node *parse(LList *tokens);
void print_ast(Node *ast);

#endif // !Nst_PARSER_H