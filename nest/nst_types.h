#ifndef NST_TYPES_H
#define NST_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "map.h"
#include "sequence.h"
#include "nodes.h"
#include "llist.h"

typedef int64_t Nst_int;
typedef double Nst_real;
typedef char Nst_bool;
typedef char Nst_byte;
typedef struct Nst_string
{
    size_t len;
    char *value;
    bool allocated;
}
Nst_string;
typedef FILE Nst_iofile;
typedef struct Nst_func
{
    Node *body;
    LList *args;
} Nst_func;

#define NST_TRUE 1
#define NST_FALSE 0

#define AS_INT(ptr)  ((Nst_int *)(ptr))
#define AS_REAL(ptr) ((Nst_real *)(ptr))
#define AS_BYTE(ptr) ((Nst_byte *)(ptr))
#define AS_BOOL(ptr) ((Nst_bool *)(ptr))
#define AS_STR(ptr)  ((Nst_string *)(ptr))
#define AS_MAP(ptr)  ((Nst_map *)(ptr))
#define AS_SEQ(ptr)  ((Nst_sequence *)(ptr))

Nst_string *new_string_empty();
Nst_string *new_string_raw(char *val, bool allocated);
Nst_string *new_string(char *val, size_t len, bool allocated);
Nst_string *copy_string(Nst_string *src);

void str_set_raw(Nst_string *str, char *val, bool allocated);
void str_set(Nst_string *str, char *val, size_t len, bool allocated);
void destroy_string(Nst_string *str);

Nst_int *new_int(Nst_int value);
Nst_real *new_real(Nst_real value);
Nst_bool *new_bool(Nst_bool value);

#endif // !NST_TYPES_H
