/**
 * @file program.h
 *
 * @brief Functions for managing execution states
 *
 * @author TheSilvered
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#include "argv_parser.h"
#include "runtime_stack.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A Nest program.
 *
 * @param main_func: the main function of a program
 * @param argv: arguments passed to the program
 * @param source_path: the path of the main file
 */
NstEXP typedef struct _Nst_Program {
    Nst_ObjRef *main_func;
    Nst_ObjRef *argv;
    Nst_ObjRef *source_path;
} Nst_Program;

/**
 * The kind of execution to perform a the program.
 *
 * @param Nst_EK_ERROR: an error occurred when creating the program
 * @param Nst_EK_RUN: the program can be run
 * @param Nst_EK_INFO: only info was requested (e.g. tokens), the program must
 * not be run
 */
NstEXP typedef enum _Nst_ExecutionKind {
    Nst_EK_ERROR,
    Nst_EK_RUN,
    Nst_EK_INFO
} Nst_ExecutionKind;

/**
 * Initialize a `Nst_Program` to run.
 *
 * @param prog: the program to initialize
 * @param args: the arguments for the program
 *
 * @return How to handle the execution of `es`, see `Nst_ExecutionKind`. The
 * error is set when `Nst_EK_ERROR` is returned.
 */
NstEXP Nst_ExecutionKind NstC Nst_prog_init(Nst_Program *prog, Nst_CLArgs args);
/* Destroys the contents of a program. */
NstEXP void NstC Nst_prog_destroy(Nst_Program *es);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !PROGRAM_H
