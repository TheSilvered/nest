#ifndef SET_ERROR_INTERNAL_H
#define SET_ERROR_INTERNAL_H

#include "error.h"

#define SET_SYNTAX_ERROR_INT(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = SYNTAX_ERROR; \
    error->message = msg

#define SET_MEMORY_ERROR_INT(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = MEMORY_ERROR; \
    error->message = msg

#define SET_TYPE_ERROR_INT(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = TYPE_ERROR; \
    error->message = msg

#define SET_VALUE_ERROR_INT(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = VALUE_ERROR; \
    error->message = msg

#define SET_MATH_ERROR_INT(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = MATH_ERROR; \
    error->message = msg

#define SET_CALL_ERROR_INT(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = CALL_ERROR; \
    error->message = msg

#define SET_GENERAL_ERROR_INT(error, e_start, e_end, msg) \
    error->start = e_start; \
    error->end = e_end; \
    error->name = GENERAL_ERROR; \
    error->message = msg

#endif
