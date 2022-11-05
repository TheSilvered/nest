/* AST compiler */

#ifndef COMPILER_H
#define COMPILER_H

#include "instructions.h"
#include "nodes.h"

// Compiles the AST
Nst_InstructionList *nst_compile(Nst_Node *code, bool is_module);
// Prints an Nst_InstructionList as if using the -b flag in the command
void nst_print_bytecode(Nst_InstructionList *ls, int indent);

#endif // !COMPILER_H
