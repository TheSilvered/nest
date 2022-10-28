#ifndef SET_ERROR_INTERNAL_H
#define SET_ERROR_INTERNAL_H

#include "error.h"

#define _NST_SET_ERROR(error, e_start, e_end, e_name, msg) \
    do { \
        (error)->occurred = true; \
        (error)->start = e_start; \
        (error)->end = e_end; \
        (error)->name = (char *)e_name; \
        (error)->message = (char *)(msg); \
    } while ( 0 )

#define _NST_SET_SYNTAX_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, NST_E_SYNTAX_ERROR, msg)

#define _NST_SET_MEMORY_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, NST_E_MEMORY_ERROR, msg)

#define _NST_SET_TYPE_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, NST_E_TYPE_ERROR, msg)

#define _NST_SET_VALUE_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, NST_E_VALUE_ERROR, msg)

#define _NST_SET_MATH_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, NST_E_MATH_ERROR, msg)

#define _NST_SET_CALL_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, NST_E_CALL_ERROR, msg)

#define _NST_SET_GENERAL_ERROR(error, e_start, e_end, msg) \
    _NST_SET_ERROR(error, e_start, e_end, NST_E_GENERAL_ERROR, msg)

#endif // !SET_ERROR_INTERNAL
