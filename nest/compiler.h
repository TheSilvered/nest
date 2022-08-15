#ifndef COMPILER_H
#define COMPILER_H

#include "simple_types.h"
#include "instructions.h"
#include "nodes.h"

Nst_InstructionList *nst_compile(Nst_Node *code);
void nst_print_bytecode(Nst_InstructionList *ls, int indent);

#endif // !COMPILER_H
