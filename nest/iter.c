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
    Nst_IterObj *iter = AS_ITER(alloc_obj(
        sizeof(Nst_IterObj),
        nst_t_iter,
        nst_destroy_iter
    ));
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

void nst_destroy_iter(Nst_IterObj *iter)
{
    dec_ref(iter->start);
    dec_ref(iter->advance);
    dec_ref(iter->is_done);
    dec_ref(iter->get_val);
    dec_ref(iter->value);
}

NST_FUNC_SIGN(nst_num_iter_start)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    AS_INT(val->objs[0]) = AS_INT(val->objs[1]);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_num_iter_advance)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    register Nst_Obj *idx_obj = val->objs[0];
    AS_INT(val->objs[0]) += AS_INT(val->objs[3]);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_num_iter_is_done)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    register Nst_Int idx = AS_INT(objs[0]);
    register Nst_Int stop = AS_INT(objs[2]);
    register Nst_Int step = AS_INT(objs[3]);

    if ( step > 0 )
    {
        if ( idx < stop )
            NST_RETURN_FALSE;
        else
            NST_RETURN_TRUE;
    }
    else
    {
        if ( idx > stop )
            NST_RETURN_FALSE;
        else
            NST_RETURN_TRUE;
    }
}

NST_FUNC_SIGN(nst_num_iter_get_val)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    return nst_new_int(AS_INT(val->objs[0]));
}

NST_FUNC_SIGN(nst_seq_iter_start)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_seq_iter_advance)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    AS_INT(val->objs[0]) += 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_seq_iter_is_done)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    register size_t seq_len = AS_SEQ(objs[1])->len;

    if ( seq_len == 0 || AS_INT(objs[0]) >= (Nst_Int)seq_len )
        NST_RETURN_TRUE;
    else
        NST_RETURN_FALSE;
}

NST_FUNC_SIGN(nst_seq_iter_get_val)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    register Nst_Obj *obj = AS_SEQ(val->objs[1])->objs[AS_INT(val->objs[0])];

    inc_ref(obj);
    return obj;
}

NST_FUNC_SIGN(nst_str_iter_start)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    AS_INT(val->objs[0]) = 0;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_str_iter_advance)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    AS_INT(val->objs[0]) += 1;
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(nst_str_iter_is_done)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;
    register size_t str_len = AS_STR(objs[1])->len;

    if ( str_len == 0 || AS_INT(objs[0]) >= (Nst_Int)str_len )
        NST_RETURN_TRUE;
    else
        NST_RETURN_FALSE;
}

NST_FUNC_SIGN(nst_str_iter_get_val)
{
    Nst_SeqObj *val = AS_SEQ(args[0]);
    Nst_Obj **objs = val->objs;

    return nst_string_get_idx(objs[1], AS_INT(objs[0]));
}
