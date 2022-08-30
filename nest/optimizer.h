#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "nodes.h"
#include "compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

Nst_Node *nst_optimize_ast(Nst_Node *ast);
Nst_InstructionList *nst_optimize_bytecode(Nst_InstructionList *bc, bool optimize_builtins);

#ifdef __cplusplus
}
#endif

#endif // !OPTIMIZER_H