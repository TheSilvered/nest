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
 * Compiles the AST.
 *
 * @brief Both ast and error are expected to be not NULL. The AST is destroyed
 * when passed to this function.
 *
 * @param ast: the AST to compile, will be freed by the function
 * @param is_module: whether the AST is of an imported module or of the main
 * file
 * @param error: the error set if one occurs
 *
 * @return The function returns the compiled Nst_InstList or NULL if an error
 * occurred.
 */
NstEXP Nst_InstList *NstC Nst_compile(Nst_Node *ast, bool is_module,
                                      Nst_Error *error);

/**
 * Prints an Nst_InstList.
 *
 * @brief This function is called when using the -b option.
 *
 * @param ls: the instruction list to print, it is expected to be valid
 */
NstEXP void NstC Nst_print_bytecode(Nst_InstList *ls);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !COMPILER_H
