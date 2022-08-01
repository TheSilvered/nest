#ifndef SET_ERROR_INTERNAL_H
#define SET_ERROR_INTERNAL_H

#include "error.h"

#define _NST_SET_SYNTAX_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = NST_E_SYNTAX_ERROR; \
    error->message = msg

#define _NST_SET_MEMORY_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = NST_E_MEMORY_ERROR; \
    error->message = msg

#define _NST_SET_TYPE_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = NST_E_TYPE_ERROR; \
    error->message = msg

#define _NST_SET_VALUE_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = NST_E_VALUE_ERROR; \
    error->message = msg

#define _NST_SET_MATH_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = NST_E_MATH_ERROR; \
    error->message = msg

#define _NST_SET_CALL_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = NST_E_CALL_ERROR; \
    error->message = msg

#define _NST_SET_GENERAL_ERROR(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = NST_E_GENERAL_ERROR; \
    error->message = msg

#endif
