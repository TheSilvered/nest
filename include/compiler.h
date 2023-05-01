/* AST compiler */

#ifndef COMPILER_H
#define COMPILER_H

#include "instructions.h"
#include "nodes.h"

#ifdef __cplusplus
extern "C" {
#endif

// Compiles the AST
EXPORT Nst_InstList *nst_compile(Nst_Node *ast, bool is_module, Nst_Error *error);

// Prints an Nst_InstructionList as if using the -b flag in the command
EXPORT void nst_print_bytecode(Nst_InstList *ls, i32 indent);

#ifdef __cplusplus
}
#endif

#endif // !COMPILER_H
