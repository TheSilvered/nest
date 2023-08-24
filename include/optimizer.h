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
 * @param error: the pointer to place the error in
 *
 * @return The optimized AST on success and `NULL` on failure. No operation
 * error is set.
 */
NstEXP Nst_Node *NstC Nst_optimize_ast(Nst_Node *ast, Nst_Error *error);
/**
 * Optimizes bytecode.
 *
 * @brief The bytecode is destroyed if an error occurs.
 *
 * @param bc: the bytecode to optimize
 * @param optimize_builtins: whether accesses to builtins such as Int and false
 * should be replaced by their values when they are not modified
 * @param error: the pointer to place the error in
 *
 * @return The optimized bytecode on success and `NULL` on failure.
 * No operation error is set.
 */
NstEXP Nst_InstList *NstC Nst_optimize_bytecode(Nst_InstList *bc,
                                                bool optimize_builtins,
                                                Nst_Error *error);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !OPTIMIZER_H
