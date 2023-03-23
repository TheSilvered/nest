#ifndef ERROR_INTERNAL_H
#define ERROR_INTERNAL_H

#include "error.h"

#define _NST_SET_ERROR(error, e_start, e_end, e_name, msg) \
    do { \
        (error)->occurred = true; \
        (error)->start = e_start; \
        (error)->end = e_end; \
        (error)->name = STR(nst_inc_ref(e_name)); \
        (error)->message = STR(msg); \
    } while ( 0 )

#define _NST_SET_RAW_ERROR(error, e_start, e_end, e_name, msg) \
    do { \
        (error)->occurred = true; \
        (error)->start = e_start; \
        (error)->end = e_end; \
        (error)->name = STR(nst_inc_ref(e_name)); \
        Nst_OpErr _err_ = { NULL, NULL }; \
        (error)->message = STR(nst_string_new_c_raw((const i8 *)(msg), false, &_err_)); \
        if ( _err_.name != NULL ) { \
            nst_dec_ref(e_name); \
            (error)->name = _err_.name; \
            (error)->message = _err_.message; \
        } \
    } while ( 0 )

#define _NST_SET_SYNTAX_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, nst_s.e_SyntaxError, msg)

#define _NST_SET_MEMORY_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, nst_s.e_MemoryError, msg)

#define _NST_SET_TYPE_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, nst_s.e_TypeError, msg)

#define _NST_SET_VALUE_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, nst_s.e_ValueError, msg)

#define _NST_SET_MATH_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, nst_s.e_MathError, msg)

#define _NST_SET_CALL_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, nst_s.e_CallError, msg)

#define _NST_SET_IMPORT_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, nst_s.e_ImportError, msg)

#define _NST_SET_RAW_SYNTAX_ERROR(error, e_start, e_end, msg) \
    _NST_SET_RAW_ERROR(error, e_start, e_end, nst_s.e_SyntaxError, msg)

#define _NST_SET_RAW_MEMORY_ERROR(error, e_start, e_end, msg) \
    _NST_SET_RAW_ERROR(error, e_start, e_end, nst_s.e_MemoryError, msg)

#define _NST_SET_RAW_TYPE_ERROR(error, e_start, e_end, msg) \
    _NST_SET_RAW_ERROR(error, e_start, e_end, nst_s.e_TypeError, msg)

#define _NST_SET_RAW_VALUE_ERROR(error, e_start, e_end, msg) \
    _NST_SET_RAW_ERROR(error, e_start, e_end, nst_s.e_ValueError, msg)

#define _NST_SET_RAW_MATH_ERROR(error, e_start, e_end, msg) \
    _NST_SET_RAW_ERROR(error, e_start, e_end, nst_s.e_MathError, msg)

#define _NST_SET_RAW_CALL_ERROR(error, e_start, e_end, msg) \
    _NST_SET_RAW_ERROR(error, e_start, e_end, nst_s.e_CallError, msg)

#define _NST_SET_RAW_IMPORT_ERROR(error, e_start, e_end, msg) \
    _NST_SET_RAW_ERROR(error, e_start, e_end, nst_s.e_ImportError, msg)

#define _NST_FAILED_ALLOCATION(error, e_start, e_end) \
    _NST_SET_ERROR(error, e_start, e_end, \
                   nst_s.e_MemoryError, nst_inc_ref(nst_s.o_failed_alloc))

#define _NST_SET_ERROR_FROM_OP_ERR(error, op_err, e_start, e_end) do { \
    _NST_SET_ERROR(error, e_start, e_end, (op_err)->name, (op_err)->message); \
    nst_dec_ref((op_err)->name); \
    (op_err)->name = NULL; \
    (op_err)->message = NULL; \
    } while (0)

#endif // !ERROR_INTERNAL_H
