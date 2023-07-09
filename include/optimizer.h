/* AST an Bytecode optimizer */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "compiler.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// Optimizes the AST, can return NULL and print an error
NstEXP Nst_Node *NstC Nst_optimize_ast(Nst_Node *ast, Nst_Error *error);
// Optimizes the bytecode, can return NULL and print an error
NstEXP Nst_InstList *NstC Nst_optimize_bytecode(Nst_InstList *bc,
                                                bool optimize_builtins,
                                                Nst_Error *error);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OPTIMIZER_H
