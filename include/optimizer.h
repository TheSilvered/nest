/**
 * @file optimizer.h
 *
 * @brief Ast and bytecode optimizer
 *
 * @author TheSilvered
 */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "compiler.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Optimize the AST.
 *
 * @brief The AST is destroyed if an error occurs.
 *
 * @param ast: the AST to optimize
 *
 * @return The optimized AST on success and `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_Node *NstC Nst_optimize_ast(Nst_Node *ast);

/**
 * Optimize an instruction list. This function is guaranteed to not fail.
 *
 * @param ls: the instruction list to optimize
 * @param optimize_builtins: whether accesses to builtins such as Int and false
 * should be replaced by their values when they are not modified
 */
NstEXP void NstC Nst_optimize_ilist(Nst_InstList *ls, bool optimize_builtins);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OPTIMIZER_H
