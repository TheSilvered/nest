/**
 * @file error_internal.h
 *
 * @brief Macros to set a Nst_Error
 *
 * @author TheSilvered
 */

#ifndef ERROR_INTERNAL_H
#define ERROR_INTERNAL_H

#include "error.h"

#define _Nst_SET_ERROR(error, e_start, e_end, e_name, msg) do {               \
    (error)->occurred = true;                                                 \
    (error)->start = e_start;                                                 \
    (error)->end = e_end;                                                     \
    (error)->name = STR(Nst_inc_ref(e_name));                                 \
    (error)->message = STR(msg);                                              \
    Nst_error_clear();                                                        \
    } while (0)

#define _Nst_SET_RAW_ERROR(error, e_start, e_end, e_name, msg)  do {          \
    (error)->occurred = true;                                                 \
    (error)->start = e_start;                                                 \
    (error)->end = e_end;                                                     \
    (error)->name = STR(Nst_inc_ref(e_name));                                 \
    (error)->message = STR(Nst_string_new_c_raw((const i8 *)(msg), false));   \
    if (Nst_error_occurred()) {                                               \
        Nst_dec_ref(e_name);                                                  \
        (error)->name = STR(Nst_inc_ref(Nst_error_get()->name));              \
        (error)->message = STR(Nst_inc_ref(Nst_error_get()->message));        \
    }                                                                         \
    Nst_error_clear();                                                        \
    } while (0)

#define _Nst_SET_SYNTAX_ERROR(error, e_start, e_end, msg)                     \
    _Nst_SET_ERROR(error, e_start, e_end, Nst_s.e_SyntaxError, msg)

#define _Nst_SET_MEMORY_ERROR(error, e_start, e_end, msg)                     \
    _Nst_SET_ERROR(error, e_start, e_end, Nst_s.e_MemoryError, msg)

#define _Nst_SET_TYPE_ERROR(error, e_start, e_end, msg)                       \
    _Nst_SET_ERROR(error, e_start, e_end, Nst_s.e_TypeError, msg)

#define _Nst_SET_VALUE_ERROR(error, e_start, e_end, msg)                      \
    _Nst_SET_ERROR(error, e_start, e_end, Nst_s.e_ValueError, msg)

#define _Nst_SET_MATH_ERROR(error, e_start, e_end, msg)                       \
    _Nst_SET_ERROR(error, e_start, e_end, Nst_s.e_MathError, msg)

#define _Nst_SET_CALL_ERROR(error, e_start, e_end, msg)                       \
    _Nst_SET_ERROR(error, e_start, e_end, Nst_s.e_CallError, msg)

#define _Nst_SET_IMPORT_ERROR(error, e_start, e_end, msg)                     \
    _Nst_SET_ERROR(error, e_start, e_end, Nst_s.e_ImportError, msg)

#define _Nst_SET_RAW_SYNTAX_ERROR(error, e_start, e_end, msg)                 \
    _Nst_SET_RAW_ERROR(error, e_start, e_end, Nst_s.e_SyntaxError, msg)

#define _Nst_SET_RAW_MEMORY_ERROR(error, e_start, e_end, msg)                 \
    _Nst_SET_RAW_ERROR(error, e_start, e_end, Nst_s.e_MemoryError, msg)

#define _Nst_SET_RAW_TYPE_ERROR(error, e_start, e_end, msg)                   \
    _Nst_SET_RAW_ERROR(error, e_start, e_end, Nst_s.e_TypeError, msg)

#define Nst_SET_RAW_VALUE_ERROR(error, e_start, e_end, msg)                   \
    _Nst_SET_RAW_ERROR(error, e_start, e_end, Nst_s.e_ValueError, msg)

#define _Nst_SET_RAW_MATH_ERROR(error, e_start, e_end, msg)                   \
    _Nst_SET_RAW_ERROR(error, e_start, e_end, Nst_s.e_MathError, msg)

#define _Nst_SET_RAW_CALL_ERROR(error, e_start, e_end, msg)                   \
    _Nst_SET_RAW_ERROR(error, e_start, e_end, Nst_s.e_CallError, msg)

#define _Nst_SET_RAW_IMPORT_ERROR(error, e_start, e_end, msg)                 \
    _Nst_SET_RAW_ERROR(error, e_start, e_end, Nst_s.e_ImportError, msg)

#define _Nst_FAILED_ALLOCATION(error, e_start, e_end)                         \
    _Nst_SET_ERROR(                                                           \
        error,                                                                \
        e_start, e_end,                                                       \
        Nst_s.e_MemoryError,                                                  \
        Nst_inc_ref(Nst_s.o_failed_alloc))

#define _Nst_SET_ERROR_FROM_OP_ERR(error, e_start, e_end) do {                \
    Nst_OpErr *_op_err_ = Nst_error_get();                                    \
    _Nst_SET_ERROR(                                                           \
        error,                                                                \
        e_start, e_end,                                                       \
        _op_err_->name,                                                       \
        Nst_inc_ref(_op_err_->message));                                      \
    } while (0)

#endif // !ERROR_INTERNAL_H
