/* AST an Bytecode optimizer */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

// Optimizes the AST, can return NULL and print an error
Nst_Node *nst_optimize_ast(Nst_Node *ast, Nst_Error *error);
// Optimizes the bytecode, can return NULL and print an error
Nst_InstList *nst_optimize_bytecode(Nst_InstList *bc,
                                    bool optimize_builtins,
                                    Nst_Error *error);

#ifdef __cplusplus
}
#endif

#endif // !OPTIMIZER_H