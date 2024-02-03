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
 * Optimizes the AST.
 *
 * @brief The AST is destroyed if an error occurs.
 *
 * @param ast: the AST to optimize
 *
 * @return The optimized AST on success and `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_Node__old *NstC Nst_optimize_ast(Nst_Node__old *ast);

/**
 * Optimizes bytecode.
 *
 * @brief The bytecode is destroyed if an error occurs.
 *
 * @param bc: the bytecode to optimize
 * @param optimize_builtins: whether accesses to builtins such as Int and false
 * should be replaced by their values when they are not modified
 *
 * @return The optimized bytecode on success and `NULL` on failure. The error
 * is set.
 */
NstEXP Nst_InstList *NstC Nst_optimize_bytecode(Nst_InstList *bc,
                                                bool optimize_builtins);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OPTIMIZER_H
