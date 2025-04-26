/**
 * @file compiler.h
 *
 * @brief Compiler from AST to bytecode
 *
 * @author TheSilvered
 */

#ifndef COMPILER_H
#define COMPILER_H

#include "instructions.h"
#include "nodes.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Compile the AST.
 *
 * @param ast: the AST to compile
 * @param is_module: whether the AST is of an imported module or of the main
 * file
 *
 * @return The compiled `Nst_InstList`. If an error occurred the resulting
 * instruction list has no instructions. The error is set.
 */
NstEXP Nst_InstList NstC Nst_compile(Nst_Node *ast, bool is_module);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !COMPILER_H
