#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "nodes.h"
#include "compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

Nst_Node *nst_optimize_ast(Nst_Node *ast);

#ifdef __cplusplus
}
#endif

#endif // !OPTIMIZER_H