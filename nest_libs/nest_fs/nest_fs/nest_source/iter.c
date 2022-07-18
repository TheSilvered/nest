#include <errno.h>
#include <stdlib.h>
#include "iter.h"
#include "lib_import.h"
#include "sequence.h"
#include "simple_types.h"
#include "str.h"

Nst_Obj *new_iter(
    Nst_FuncObj *start,
    Nst_FuncObj *advance,
    Nst_FuncObj *is_done,
    Nst_FuncObj *get_val,
    Nst_Obj *value)
{
    Nst_IterObj *iter = AS_ITER(alloc_obj(sizeof(Nst_IterObj), nst_t_iter, destroy_iter));
    if ( iter == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    iter->start = start;
    iter->advance = advance;
    iter->is_done = is_done;
    iter->get_val = get_val;
    iter->value = value;

    return (Nst_Obj *)iter;
}

void destroy_iter(Nst_IterObj *iter)
{
    dec_ref(iter->start);
    dec_ref(iter->advance);
    dec_ref(iter->is_done);
    dec_ref(iter->get_val);
    dec_ref(iter->value);
}

Nst_Obj *num_iter_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    AS_INT(val->objs[0]) = AS_INT(val->objs[1]);
    RETURN_NULL;
}

Nst_Obj *num_iter_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    register Nst_Obj *idx_obj = val->objs[0];
    AS_INT(val->objs[0]) += AS_INT(val->objs[3]);
    RETURN_NULL;
}

Nst_Obj *num_iter_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    register Nst_int idx = AS_INT(objs[0]);
    register Nst_int stop = AS_INT(objs[2]);
    register Nst_int step = AS_INT(objs[3]);

    if ( step > 0 )
    {
        if ( idx < stop )
            RETURN_FALSE;
        else
            RETURN_TRUE;
    }
    else
    {
        if ( idx > stop )
            RETURN_FALSE;
        else
            RETURN_TRUE;
    }
}

Nst_Obj *num_iter_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    return new_int(AS_INT(val->objs[0]));
}

Nst_Obj *seq_iter_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    RETURN_NULL;
}

Nst_Obj *seq_iter_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    AS_INT(val->objs[0]) += 1;
    RETURN_NULL;
}

Nst_Obj *seq_iter_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    register size_t seq_len = AS_SEQ(objs[1])->len;

    if ( seq_len == 0 || AS_INT(objs[0]) >= (Nst_int)seq_len )
        RETURN_TRUE;
    else
        RETURN_FALSE;
}

Nst_Obj *seq_iter_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    register Nst_Obj *obj = AS_SEQ(val->objs[1])->objs[AS_INT(val->objs[0])];

    inc_ref(obj);
    return obj;
}

Nst_Obj *str_iter_start(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    RETURN_NULL;
}

Nst_Obj *str_iter_advance(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    AS_INT(val->objs[0]) += 1;
    RETURN_NULL;
}

Nst_Obj *str_iter_is_done(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    register size_t str_len = AS_STR(objs[1])->len;

    if ( str_len == 0 || AS_INT(objs[0]) >= (Nst_int)str_len )
        RETURN_TRUE;
    else
        RETURN_FALSE;
}

Nst_Obj *str_iter_get_val(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    char *ch = calloc(2, sizeof(char));
    if ( ch == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    ch[0] = AS_STR(objs[1])->value[AS_INT(objs[0])];

    return new_string(ch, 1, true);
}
