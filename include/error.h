/**
 * @file error.h
 *
 * @brief Error management interface
 *
 * @author TheSilvered
 */
#ifndef ERROR_H
#define ERROR_H

#include "str.h"
#include "dyn_array.h"

/* Correctly formats the error message for the wrong number of arguments. */
#define _Nst_WRONG_ARG_NUM(func_arg_num, arg_num)                             \
    Nst_sprintf(                                                              \
        "the function expected at most %zi argument%s but %zi %s passed",     \
        func_arg_num, func_arg_num == 1 ? "" : "s",                           \
        arg_num, arg_num == 1 ? "was" : "were")

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The structure where the source text of a Nest file is kept.
 *
 * @param allocated: whether `text`, `path` and `lines` are heap allocated
 * @param text: the UTF-8 text of the file
 * @param path: the path of the file
 * @param lines: the beginning of each line of the file
 * @param len: the length in bytes of `text`
 * @param lines_len: the number of lines in the file
 */
NstEXP typedef struct _Nst_SourceText {
    bool allocated;
    char *text;
    char *path;
    char **lines;
    usize text_len;
    usize lines_len;
} Nst_SourceText;

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
    Nst_Obj *error_name;
    Nst_Obj *error_msg;
    Nst_DynArray positions;
} Nst_Traceback;

/* Sets how the error message is printed (with or without ANSI escapes). */
NstEXP void NstC Nst_error_set_color(bool color);
/* Creates an empty position with no valid text. */
NstEXP Nst_Pos NstC Nst_pos_empty(void);

/* Make a new span from a start and an end posiiton. */
NstEXP Nst_Span NstC Nst_span_new(Nst_Pos start, Nst_Pos end);
/* Make a new span with the same start and end. */
NstEXP Nst_Span NstC Nst_span_from_pos(Nst_Pos pos);
/* Creates an empty span with no valid text. */
NstEXP Nst_Span NstC Nst_span_empty(void);
/* Make a span that includes both. */
NstEXP Nst_Span NstC Nst_span_join(Nst_Span span1, Nst_Span span2);
/* Expands a span to include the given position. */
NstEXP Nst_Span NstC Nst_span_expand(Nst_Span span, Nst_Pos pos);
/* Get the start position of a span. */
NstEXP Nst_Pos NstC Nst_span_start(Nst_Span span);
/* Get the end position of a span. */
NstEXP Nst_Pos NstC Nst_span_end(Nst_Span span);

/* Initializes the traceback of the current `Nst_state`. */
NstEXP void NstC Nst_tb_init(Nst_Traceback *tb);
/* Frees the traceback of the current `Nst_state`. */
NstEXP void NstC Nst_tb_destroy(Nst_Traceback *tb);
/* Adds a pair of positions to an `Nst_Traceback`. */
NstEXP void NstC Nst_tb_add_span(Nst_Traceback *tb, Nst_Span span);
/* Prints a formatted `Nst_Traceback`. */
NstEXP void NstC Nst_tb_print(Nst_Traceback *tb);

/* Initializes the fields of a `Nst_SourceText` struct. */
NstEXP void NstC Nst_source_text_init(Nst_SourceText *src);
/**
 * @brief Frees a heap allocated text source, `text` can be `NULL`. No error
 * is set.
 */
NstEXP void NstC Nst_source_text_destroy(Nst_SourceText *text);

/**
 * Sets the error with the given name and message.
 *
 * @brief It takes a reference from both `name` and `msg`.
 */
NstEXP void NstC Nst_error_set(Nst_Obj *name, Nst_Obj *msg);
/**
 * Sets the error with the given message and "Syntax Error" as the name.
 *
 * @brief It takes a reference from `msg`.
 */
NstEXP void NstC Nst_error_set_syntax(Nst_Obj *msg);
/**
 * Sets the error with the given message and "Memory Error" as the name.
 *
 * @brief It takes a reference from `msg`.
 */
NstEXP void NstC Nst_error_set_memory(Nst_Obj *msg);
/**
 * Sets the error with the given message and "Type Error" as the name.
 *
 * @brief It takes a reference from `msg`.
 */
NstEXP void NstC Nst_error_set_type(Nst_Obj *msg);
/**
 * Sets the error with the given message and "Value Error" as the name.
 *
 * @brief It takes a reference from `msg`.
 */
NstEXP void NstC Nst_error_set_value(Nst_Obj *msg);
/**
 * Sets the error with the given message and "Math Error" as the name.
 *
 * @brief It takes a reference from `msg`.
 */
NstEXP void NstC Nst_error_set_math(Nst_Obj *msg);
/**
 * Sets the error with the given message and "Call Error" as the name.
 *
 * @brief It takes a reference from `msg`.
 */
NstEXP void NstC Nst_error_set_call(Nst_Obj *msg);
/**
 * Sets the error with the given message and "Import Error" as the name.
 *
 * @brief It takes a reference from `msg`.
 */
NstEXP void NstC Nst_error_set_import(Nst_Obj *msg);

/**
 * @brief Sets the error creating a `String` object from `msg` and using
 * "Syntax Error" as the name.
 */
NstEXP void NstC Nst_error_setc_syntax(const char *msg);
/**
 * @brief Sets the error creating a `String` object from `msg` and using
 * "Memory Error" as the name.
 */
NstEXP void NstC Nst_error_setc_memory(const char *msg);
/**
 * @brief Sets the error creating a `String` object from `msg` and using
 * "Type Error" as the name.
 */
NstEXP void NstC Nst_error_setc_type(const char *msg);
/**
 * @brief Sets the error creating a `String` object from `msg` and using
 * "Value Error" as the name.
 */
NstEXP void NstC Nst_error_setc_value(const char *msg);
/**
 * @brief Sets the error creating a `String` object from `msg` and using
 * "Math Error" as the name.
 */
NstEXP void NstC Nst_error_setc_math(const char *msg);
/**
 * @brief Sets the error creating a `String` object from `msg` and using
 * "Call Error" as the name.
 */
NstEXP void NstC Nst_error_setc_call(const char *msg);
/**
 * @brief Sets the error creating a `String` object from `msg` and using
 * "Import Error" as the name.
 */
NstEXP void NstC Nst_error_setc_import(const char *msg);

/**
 * @brief Set the error creating a formatted `String` object and using
 * "Syntax Error" as the name
 */
NstEXP void NstC Nst_error_setf_syntax(Nst_WIN_FMT const char *fmt, ...)
                                       Nst_NIX_FMT(1, 2);
/**
 * @brief Set the error creating a formatted `String` object and using
 * "Memory Error" as the name
 */
NstEXP void NstC Nst_error_setf_memory(Nst_WIN_FMT const char *fmt, ...)
                                       Nst_NIX_FMT(1, 2);
/**
 * @brief Set the error creating a formatted `String` object and using
 * "Type Error" as the name
 */
NstEXP void NstC Nst_error_setf_type(Nst_WIN_FMT const char *fmt, ...)
                                     Nst_NIX_FMT(1, 2);
/**
 * @brief Set the error creating a formatted `String` object and using
 * "Value Error" as the name
 */
NstEXP void NstC Nst_error_setf_value(Nst_WIN_FMT const char *fmt, ...)
                                      Nst_NIX_FMT(1, 2);
/**
 * @brief Set the error creating a formatted `String` object and using
 * "Math Error" as the name
 */
NstEXP void NstC Nst_error_setf_math(Nst_WIN_FMT const char *fmt, ...)
                                     Nst_NIX_FMT(1, 2);
/**
 * @brief Set the error creating a formatted `String` object and using
 * "Call Error" as the name
 */
NstEXP void NstC Nst_error_setf_call(Nst_WIN_FMT const char *fmt, ...)
                                     Nst_NIX_FMT(1, 2);
/**
 * @brief Set the error creating a formatted `String` object and using
 * "Import Error" as the name
 */
NstEXP void NstC Nst_error_setf_import(Nst_WIN_FMT const char *fmt, ...)
                                       Nst_NIX_FMT(1, 2);

/* Sets the global operation error with a memory error of failed allocation. */
NstEXP void NstC Nst_error_failed_alloc(void);

/* Adds a pair of positions to the current error. */
NstEXP void NstC Nst_error_add_span(Nst_Span span);

/* [docs:ignore_sym Nst_EK_NONE] */
/* [docs:ignore_sym Nst_EK_LOCAL] */
/* [docs:ignore_sym Nst_EK_GLOBAL] */
/**
 * The kinds of errors that can occur during execution.
 *
 * @param Nst_EK_NONE: no error occurred
 * @param Nst_EK_LOCAL: an error has occurred in the current execution state
 * @param Nst_EK_GLOBAL: an error has occurred while no execution state was set
 */
typedef enum _Nst_ErrorKind {
    Nst_EK_NONE = 0,
    Nst_EK_LOCAL,
    Nst_EK_GLOBAL
} Nst_ErrorKind;

/**
 * @return Whether an error has occurred in the current execution. Check
 * `Nst_ErrorKind` to understand better the return value.
 */
NstEXP Nst_ErrorKind NstC Nst_error_occurred(void);
/**
 * @return The traceback of the current execution or that of the interpreter if
 * no execution state is set.
 */
NstEXP Nst_Traceback *NstC Nst_error_get(void);
/**
 * @brief Clears the traceback of both the current execution and of the
 * interpreter. If no error has occurred nothing is done.
 */
NstEXP void NstC Nst_error_clear(void);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ERROR_H
