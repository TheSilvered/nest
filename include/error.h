/**
 * @file error.h
 *
 * @brief Error management interface
 *
 * @author TheSilvered
 */
#ifndef ERROR_H
#define ERROR_H

#include "source_loader.h"
#include "str.h"
#include "dyn_array.h"

/**
 * Correctly formats the error message for using the wrong number of arguments.
 */
#define _Nst_WRONG_ARG_NUM(func_arg_num, arg_num)                             \
    Nst_sprintf(                                                              \
        "the function expected at most %zi argument%s but %zi %s passed",     \
        func_arg_num, func_arg_num == 1 ? "" : "s",                           \
        arg_num, arg_num == 1 ? "was" : "were")

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The structure representing a position inside a source file.
 *
 * @param line: the line of the position, the first is line 0
 * @param col: the column of the position, the first is 0
 * @param text: the text this position refers to
 */
NstEXP typedef struct _Nst_Pos {
    i32 line;
    i32 col;
    Nst_SourceText *text;
} Nst_Pos;

/**
 * The structure representing a text span inside a source file.
 *
 * @param start_line: the starting line, the first is line 0
 * @param start_col: the starting column, the first is column 0
 * @param end_line: the ending line, included in the span
 * @param end_col: the ending column, included in the span
 * @param text: the text this position refers to
 */
NstEXP typedef struct _Nst_Span {
    i32 start_line;
    i32 start_col;
    i32 end_line;
    i32 end_col;
    Nst_SourceText *text;
} Nst_Span;

/**
 * The structure containing the full traceback of the error.
 *
 * @param error_occurred: whether the traceback contains an error
 * @param error_name: the name of the error (e.g. `Value Error`, `Type Error`
 * etc.)
 * @param error_msg: the message of the error
 * @param positions: the list of positions that led to the error
 */
NstEXP typedef struct _Nst_Traceback {
    bool error_occurred;
    Nst_ObjRef *error_name;
    Nst_ObjRef *error_msg;
    Nst_DynArray positions;
} Nst_Traceback;

/**
 * Set how the error message is printed (with or without ANSI color escapes).
 */
NstEXP void NstC Nst_error_set_color(bool color);
/* Create an empty position with no valid text. */
NstEXP Nst_Pos NstC Nst_pos_empty(void);

/* Make a new span from a start and an end position. */
NstEXP Nst_Span NstC Nst_span_new(Nst_Pos start, Nst_Pos end);
/* Make a new span with the same start and end positions. */
NstEXP Nst_Span NstC Nst_span_from_pos(Nst_Pos pos);
/* Create an empty span with no valid text. */
NstEXP Nst_Span NstC Nst_span_empty(void);
/* Make a span that includes both `span1` and `span2`. */
NstEXP Nst_Span NstC Nst_span_join(Nst_Span span1, Nst_Span span2);
/* Expand a span to include the given position. */
NstEXP Nst_Span NstC Nst_span_expand(Nst_Span span, Nst_Pos pos);
/**
 * @return The start position of a span.
 */
NstEXP Nst_Pos NstC Nst_span_start(Nst_Span span);
/**
 * @return The end position of a span.
 */
NstEXP Nst_Pos NstC Nst_span_end(Nst_Span span);

void _Nst_error_init(void);
/* Print the error traceback. */
NstEXP void NstC Nst_error_print(void);

/**
 * Set the error with the given name and message.
 *
 * @brief It takes one reference from both `name` and `msg`.
 */
NstEXP void NstC Nst_error_set(Nst_ObjRef *name, Nst_ObjRef *msg);
/**
 * Set the error with the given message and "Syntax Error" as the name.
 *
 * @brief It takes one reference from `msg`.
 */
NstEXP void NstC Nst_error_set_syntax(Nst_ObjRef *msg);
/**
 * Set the error with the given message and "Memory Error" as the name.
 *
 * @brief It takes one reference from `msg`.
 */
NstEXP void NstC Nst_error_set_memory(Nst_ObjRef *msg);
/**
 * Set the error with the given message and "Type Error" as the name.
 *
 * @brief It takes one reference from `msg`.
 */
NstEXP void NstC Nst_error_set_type(Nst_ObjRef *msg);
/**
 * Set the error with the given message and "Value Error" as the name.
 *
 * @brief It takes one reference from `msg`.
 */
NstEXP void NstC Nst_error_set_value(Nst_ObjRef *msg);
/**
 * Set the error with the given message and "Math Error" as the name.
 *
 * @brief It takes one reference from `msg`.
 */
NstEXP void NstC Nst_error_set_math(Nst_ObjRef *msg);
/**
 * Set the error with the given message and "Call Error" as the name.
 *
 * @brief It takes one reference from `msg`.
 */
NstEXP void NstC Nst_error_set_call(Nst_ObjRef *msg);
/**
 * Set the error with the given message and "Import Error" as the name.
 *
 * @brief It takes one reference from `msg`.
 */
NstEXP void NstC Nst_error_set_import(Nst_ObjRef *msg);

/**
 * Set the error creating a `String` object from `msg` and using "Syntax Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setc_syntax(const char *msg);
/**
 * Set the error creating a `String` object from `msg` and using "Memory Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setc_memory(const char *msg);
/**
 * Set the error creating a `String` object from `msg` and using "Type Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setc_type(const char *msg);
/**
 * Set the error creating a `String` object from `msg` and using "Value Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setc_value(const char *msg);
/**
 * Set the error creating a `String` object from `msg` and using "Math Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setc_math(const char *msg);
/**
 * Set the error creating a `String` object from `msg` and using "Call Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setc_call(const char *msg);
/**
 * Set the error creating a `String` object from `msg` and using "Import Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setc_import(const char *msg);

/**
 * Set the error creating a formatted `String` object and using "Syntax Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setf_syntax(Nst_WIN_FMT const char *fmt, ...)
                                       Nst_NIX_FMT(1, 2);
/**
 * Set the error creating a formatted `String` object and using "Memory Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setf_memory(Nst_WIN_FMT const char *fmt, ...)
                                       Nst_NIX_FMT(1, 2);
/**
 * Set the error creating a formatted `String` object and using "Type Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setf_type(Nst_WIN_FMT const char *fmt, ...)
                                     Nst_NIX_FMT(1, 2);
/**
 * Set the error creating a formatted `String` object and using "Value Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setf_value(Nst_WIN_FMT const char *fmt, ...)
                                      Nst_NIX_FMT(1, 2);
/**
 * Set the error creating a formatted `String` object and using "Math Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setf_math(Nst_WIN_FMT const char *fmt, ...)
                                     Nst_NIX_FMT(1, 2);
/**
 * Set the error creating a formatted `String` object and using "Call Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setf_call(Nst_WIN_FMT const char *fmt, ...)
                                     Nst_NIX_FMT(1, 2);
/**
 * Set the error creating a formatted `String` object and using "Import Error"
 * as the name.
 */
NstEXP void NstC Nst_error_setf_import(Nst_WIN_FMT const char *fmt, ...)
                                       Nst_NIX_FMT(1, 2);

/* Set the global operation error with a memory error of failed allocation. */
NstEXP void NstC Nst_error_failed_alloc(void);

/* Add a pair of positions to the error. */
NstEXP void NstC Nst_error_add_span(Nst_Span span);

/**
 * @return Whether an error has occurred.
 */
NstEXP bool NstC Nst_error_occurred(void);
/**
 * @return The traceback.
 */
NstEXP Nst_Traceback *NstC Nst_error_get(void);
/* Clear the traceback. If no error has occurred nothing is done. */
NstEXP void NstC Nst_error_clear(void);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
