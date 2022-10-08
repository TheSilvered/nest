/* AST an Bytecode optimizer */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "nodes.h"
#include "compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

// Optimizes the AST, can return NULL and print an error
Nst_Node *nst_optimize_ast(Nst_Node *ast);
// Optimizes the bytecode, can return NULL and print an error
Nst_InstructionList *nst_optimize_bytecode(Nst_InstructionList *bc, bool optimize_builtins);

#ifdef __cplusplus
}
#endif

#endif // !OPTIMIZER_H