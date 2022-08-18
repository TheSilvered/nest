#include <assert.h>
#include <windows.h>
#include "interpreter.h"
#include "error_internal.h"
#include "obj_ops.h"
#include "hash.h"
#include "tokens.h"

#define SET_ERROR(err_macro, start, end, message) \
    do { \
        Nst_Error *error = malloc(sizeof(Nst_Error)); \
        if ( error == NULL ) \
        { \
            errno = ENOMEM; \
            return; \
        } \
        err_macro(error, start, end, message); \
        nst_state.traceback->error = error; \
        *nst_state.error_occurred = true; \
    } while ( 0 )

#define SET_OP_ERROR(start_pos, end_pos, op_err) \
    do { \
        Nst_Error *error = malloc(sizeof(Nst_Error)); \
        if ( error == NULL ) \
        { \
            errno = ENOMEM; \
            return; \
        } \
        error->start = start_pos; \
        error->end = end_pos; \
        error->name = op_err.name; \
        error->message = op_err.message; \
        nst_state.traceback->error = error; \
        *nst_state.error_occurred = true; \
    } while ( 0 )

#define CHECK_V_STACK assert(nst_state.v_stack->current_size != 0)
#define CHECK_V_STACK_SIZE(size) assert((Nst_Int)(nst_state.v_stack->current_size) >= size)
#define CHECK_F_STACK assert(nst_state.f_stack->current_size != 0)

Nst_ExecutionState nst_state;

static void run_instruction(Nst_InstructionList *inst_ls, Nst_Int *idx);
static inline void exe_no_op();
static inline void exe_pop_val(Nst_RuntimeInstruction inst);
static inline void exe_push_func(Nst_RuntimeInstruction inst);
static inline void exe_pop_func();
static inline void exe_for_start();
static inline void exe_return_val();
static inline void exe_for_advance();
static inline void exe_set_val_loc(Nst_RuntimeInstruction inst);
static inline void exe_jump(Nst_Int *idx, Nst_RuntimeInstruction inst);
static inline void exe_jumpif_t(Nst_Int *idx, Nst_RuntimeInstruction inst);
static inline void exe_jumpif_f(Nst_Int *idx, Nst_RuntimeInstruction inst);
static inline void exe_jumpif_zero(Nst_Int *idx, Nst_RuntimeInstruction inst);
static inline void exe_type_check(Nst_RuntimeInstruction inst);
static inline void exe_hash_check(Nst_RuntimeInstruction inst);
static inline void exe_set_val(Nst_RuntimeInstruction inst);
static inline void exe_get_val(Nst_RuntimeInstruction inst);
static inline void exe_push_val(Nst_RuntimeInstruction inst);
static inline void exe_set_cont_val();
static inline void exe_op_call();
static inline void exe_op_cast(Nst_RuntimeInstruction inst);
static inline void exe_op_range(Nst_RuntimeInstruction inst);
static inline void exe_stack_op(Nst_RuntimeInstruction inst);
static inline void exe_local_op(Nst_RuntimeInstruction inst);
static inline void exe_op_import();
static inline void exe_op_extract(Nst_RuntimeInstruction inst);
static inline void exe_dec_int();
static inline void exe_new_obj();
static inline void exe_dup();
static inline void exe_make_seq(Nst_RuntimeInstruction inst);
static inline void exe_make_seq_rep(Nst_RuntimeInstruction inst);
static inline void exe_make_map(Nst_RuntimeInstruction inst);
static inline void exe_for_is_done();
static inline void exe_for_get_val();

void nst_run(Nst_InstructionList *inst_ls, int argc, char **argv)
{

}

Nst_MapObj *nst_run_module(char *file_name)
{
    return NULL;
}

Nst_Obj *nst_call_func(Nst_BcFuncObj *func, Nst_Obj **args, Nst_OpErr *err)
{
    return NULL;
}

static void run_instruction(Nst_InstructionList *inst_ls, Nst_Int *idx)
{
    Nst_RuntimeInstruction inst = inst_ls->instructions[*idx];
    Nst_Obj *obj = NULL;
    Nst_Obj *obj2 = NULL;
    Nst_Obj *obj3 = NULL;
    Nst_Int num = 0;
    Nst_OpErr err = { "", "" };

    switch ( inst.id )
    {
    case NST_IC_POP_VAL:      exe_pop_val(inst);          break;
    case NST_IC_PUSH_FUNC:    exe_push_func(inst);        break;
    case NST_IC_POP_FUNC:     exe_pop_func();             break;
    case NST_IC_FOR_START:    exe_for_start();            break;
    case NST_IC_RETURN_VAL:   exe_return_val();           break;
    case NST_IC_FOR_ADVANCE:  exe_for_advance();          break;
    case NST_IC_SET_VAL_LOC:  exe_set_val_loc(inst);      break;
    case NST_IC_JUMP:         exe_jump(idx, inst);        break;
    case NST_IC_JUMPIF_T:     exe_jumpif_t(idx, inst);    break;
    case NST_IC_JUMPIF_F:     exe_jumpif_f(idx, inst);    break;
    case NST_IC_JUMPIF_ZERO:  exe_jumpif_zero(idx, inst); break;
    case NST_IC_TYPE_CHECK:   exe_type_check(inst);       break;
    case NST_IC_HASH_CHECK:   exe_hash_check(inst);       break;
    case NST_IC_SET_VAL:      exe_set_val(inst);          break;
    case NST_IC_GET_VAL:      exe_get_val(inst);          break;
    case NST_IC_PUSH_VAL:     exe_push_val(inst);         break;
    case NST_IC_SET_CONT_VAL: exe_set_cont_val();         break;
    case NST_IC_OP_CALL:      exe_op_call();              break;
    case NST_IC_OP_CAST:      exe_op_cast(inst);          break;
    case NST_IC_OP_RANGE:     exe_op_range(inst);         break;
    case NST_IC_STACK_OP:     exe_stack_op(inst);         break;
    case NST_IC_LOCAL_OP:     exe_local_op(inst);         break;
    case NST_IC_OP_IMPORT:    exe_op_import();            break;
    case NST_IC_OP_EXTRACT:   exe_op_extract(inst);       break;
    case NST_IC_DEC_INT:      exe_dec_int();              break;
    case NST_IC_NEW_OBJ:      exe_new_obj();              break;
    case NST_IC_DUP:          exe_dup();                  break;
    case NST_IC_MAKE_ARR:
    case NST_IC_MAKE_VEC:     exe_make_seq(inst);         break;
    case NST_IC_MAKE_ARR_REP:
    case NST_IC_MAKE_VEC_REP: exe_make_seq_rep(inst);     break;
    case NST_IC_MAKE_MAP:     exe_make_map(inst);         break;
    case NST_IC_FOR_IS_DONE:  exe_for_is_done();          break;
    case NST_IC_FOR_GET_VAL:  exe_for_get_val();          break;
    }
}

static inline void exe_no_op()
{
    CHECK_V_STACK;
    dec_ref(nst_pop_val(nst_state.v_stack));
}

static inline void exe_pop_val(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);
    dec_ref(obj);
}

static inline void exe_push_func(Nst_RuntimeInstruction inst)
{
    assert(false);
}

static inline void exe_pop_func()
{
    CHECK_F_STACK;
    nst_pop_func(nst_state.f_stack);
}

static inline void exe_for_start()
{
    assert(false);
}

static inline void exe_return_val()
{
    Nst_Obj *result = nst_pop_val(nst_state.v_stack);
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);

    while ( obj != NULL )
    {
        dec_ref(obj);
        obj = nst_pop_val(nst_state.v_stack);
    }

    nst_push_val(nst_state.v_stack, result);
}

static inline void exe_for_advance()
{
    assert(false);
}

static inline void exe_set_val_loc(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);
    nst_set_val(nst_state.vt, inst.val, val);
    dec_ref(val);
}

static inline void exe_jump(Nst_Int *idx, Nst_RuntimeInstruction inst)
{
    *idx = inst.int_val - 1;
}

static inline void exe_jumpif_t(Nst_Int *idx, Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t_bool, NULL);
    dec_ref(top_val);

    if ( result == nst_true )
        *idx = inst.int_val - 1;
    dec_ref(result);
}

static inline void exe_jumpif_f(Nst_Int *idx, Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t_bool, NULL);
    dec_ref(top_val);

    if ( result == nst_false )
        *idx = inst.int_val - 1;
    dec_ref(result);
}

static inline void exe_jumpif_zero(Nst_Int *idx, Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);
    if ( AS_INT(val) == 0 )
        *idx = inst.int_val - 1;
    dec_ref(val);
}

static inline void exe_type_check(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_peek_val(nst_state.v_stack);
    if ( obj->type != inst.val )
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            inst.start,
            inst.end,
            _nst_format_types_error(
                EXPECTED_TYPES,
                AS_STR(inst.val)->value,
                obj->type_name
            );
        );
}

static inline void exe_hash_check(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_peek_val(nst_state.v_stack);
    nst_hash_obj(obj);
    if ( obj->hash == -1 )
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            inst.start,
            inst.end,
            _nst_format_type_error(UNHASHABLE_TYPE, obj->type_name);
        );
}

static inline void exe_set_val(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    nst_set_val(
        nst_state.vt,
        inst.val,
        nst_peek_val(nst_state.v_stack)
    );
}

static inline void exe_get_val(Nst_RuntimeInstruction inst)
{
    Nst_Obj *obj = nst_get_val(nst_state.vt, inst.val);
    if ( obj == NULL )
        nst_push_val(nst_state.v_stack, nst_null);
    else
    {
        nst_push_val(nst_state.v_stack, obj);
        dec_ref(obj);
    }
}

static inline void exe_push_val(Nst_RuntimeInstruction inst)
{
    nst_push_val(nst_state.v_stack, inst.val);
}

static inline void exe_set_cont_val()
{
    assert(false);
}

static inline void exe_op_call()
{
    assert(false);
}

static inline void exe_op_cast(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *type = nst_pop_val(nst_state.v_stack);

    Nst_OpErr err = { "", "" };
    Nst_Obj *res = nst_obj_cast(val, type, &err);

    if ( res == NULL )
        SET_OP_ERROR(inst.start, inst.end, err);
    else
    {
        nst_push_val(nst_state.v_stack, res);
        dec_ref(res);
    }

    dec_ref(val);
    dec_ref(type);
}

static inline void exe_op_range(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK_SIZE(inst.int_val);
    Nst_Obj *stop = nst_pop_val(nst_state.v_stack);
    Nst_Obj *step = NULL;
    Nst_Obj *start = NULL;

    if ( inst.int_val == 3 )
    {
        step = nst_pop_val(nst_state.v_stack);
        start = nst_pop_val(nst_state.v_stack);
    }
    else
    {
        start = nst_pop_val(nst_state.v_stack);

        if ( AS_INT(start) <= AS_INT(stop) )
            step = nst_new_int(1);
        else
            step = nst_new_int(-1);
    }

    Nst_Obj *idx = nst_new_int(0);

    Nst_Obj *data_seq = nst_new_array(4);
    nst_set_value_seq(data_seq, 0, idx);
    nst_set_value_seq(data_seq, 1, start);
    nst_set_value_seq(data_seq, 2, stop);
    nst_set_value_seq(data_seq, 3, step);

    dec_ref(idx);
    dec_ref(start);
    dec_ref(stop);
    dec_ref(step);

    Nst_Obj *iter = new_iter(
        AS_FUNC(new_cfunc(1, nst_num_iter_start)),
        AS_FUNC(new_cfunc(1, nst_num_iter_advance)),
        AS_FUNC(new_cfunc(1, nst_num_iter_is_done)),
        AS_FUNC(new_cfunc(1, nst_num_iter_get_val)),
        data_seq
    );

    nst_push_val(nst_state.v_stack, iter);
    dec_ref(iter);
}

static inline void exe_stack_op(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *ob1 = nst_pop_val(nst_state.v_stack);
    Nst_Obj *ob2 = nst_pop_val(nst_state.v_stack);
    Nst_Obj *res = NULL;
    Nst_OpErr err = { "", "" };

    switch ( inst.int_val )
    {
    case NST_TT_ADD:    res = nst_obj_add(ob1, ob2, &err);
    case NST_TT_SUB:    res = nst_obj_sub(ob1, ob2, &err);
    case NST_TT_MUL:    res = nst_obj_mul(ob1, ob2, &err);
    case NST_TT_DIV:    res = nst_obj_div(ob1, ob2, &err);
    case NST_TT_POW:    res = nst_obj_pow(ob1, ob2, &err);
    case NST_TT_MOD:    res = nst_obj_mod(ob1, ob2, &err);
    case NST_TT_B_AND:  res = nst_obj_bwand(ob1, ob2, &err);
    case NST_TT_B_OR:   res = nst_obj_bwor(ob1, ob2, &err);
    case NST_TT_B_XOR:  res = nst_obj_bwxor(ob1, ob2, &err);
    case NST_TT_LSHIFT: res = nst_obj_bwls(ob1, ob2, &err);
    case NST_TT_RSHIFT: res = nst_obj_bwrs(ob1, ob2, &err);
    case NST_TT_CONCAT: res = nst_obj_concat(ob1, ob2, &err);
    case NST_TT_L_AND:  res = nst_obj_lgand(ob1, ob2, &err);
    case NST_TT_L_OR:   res = nst_obj_lgor(ob1, ob2, &err);
    case NST_TT_L_XOR:  res = nst_obj_lgxor(ob1, ob2, &err);
    case NST_TT_GT:     res = nst_obj_gt(ob1, ob2, &err);
    case NST_TT_LT:     res = nst_obj_lt(ob1, ob2, &err);
    case NST_TT_EQ:     res = nst_obj_eq(ob1, ob2, &err);
    case NST_TT_NEQ:    res = nst_obj_ne(ob1, ob2, &err);
    case NST_TT_GTE:    res = nst_obj_ge(ob1, ob2, &err);
    case NST_TT_LTE:    res = nst_obj_le(ob1, ob2, &err);
    }

    if ( res == NULL )
        SET_OP_ERROR(inst.start, inst.end, err);
    else
    {
        nst_push_val(nst_state.v_stack, res);
        dec_ref(res);
    }

    dec_ref(ob1);
    dec_ref(ob2);
}

static inline void exe_local_op(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);
    Nst_Obj *res = NULL;
    Nst_OpErr err = { "", "" };

    switch ( inst.int_val )
    {
    case NST_TT_LEN:    res = nst_obj_len(obj, &err);
    case NST_TT_L_NOT:  res = nst_obj_lgnot(obj, &err);
    case NST_TT_B_NOT:  res = nst_obj_bwnot(obj, &err);
    case NST_TT_STDOUT: res = nst_obj_stdout(obj, &err);
    case NST_TT_STDIN:  res = nst_obj_stdin(obj, &err);
    case NST_TT_NEG:    res = nst_obj_neg(obj, &err);
    case NST_TT_TYPEOF: res = nst_obj_typeof(obj, &err);
    }

    if ( res == NULL )
    {
        SET_OP_ERROR(inst.start, inst.end, err);
        dec_ref(obj);
        return;
    }

    nst_push_val(nst_state.v_stack, res);
    dec_ref(obj);
    dec_ref(res);
}

static inline void exe_op_import()
{
    assert(false);
}

static inline void exe_op_extract(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *idx = nst_pop_val(nst_state.v_stack);
    Nst_Obj *cont = nst_pop_val(nst_state.v_stack);
    Nst_Obj *res = NULL;

    if ( cont->type == nst_t_arr || cont->type == nst_t_vect )
    {
        if ( idx->type != nst_t_int )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                inst.start,
                inst.end,
                _nst_format_type_error(EXPECTED_TYPE("Int"), idx->type_name)
            );

            dec_ref(cont);
            dec_ref(idx);
            return;
        }

        res = nst_get_value_seq(cont, AS_INT(idx));

        if ( res == NULL )
        {
            SET_ERROR(
                _NST_SET_VALUE_ERROR,
                inst.start,
                inst.end,
                _nst_format_idx_error(
                    cont->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                            : INDEX_OUT_OF_BOUNDS("Vector"),
                    AS_INT(idx),
                    AS_SEQ(cont)->len
                )
            );

            dec_ref(cont);
            dec_ref(idx);
            return;
        }

        nst_push_val(nst_state.v_stack, res);
    }
    else
    {
        assert(cont->type == nst_t_map);
        res = nst_map_get(cont, idx);

        if ( res == NULL )
        {
            if ( idx->hash != -1 )
                nst_push_val(nst_state.v_stack, nst_null);
            else
            {
                SET_ERROR(
                    _NST_SET_VALUE_ERROR,
                    inst.start,
                    inst.start,
                    _nst_format_type_error(UNHASHABLE_TYPE, idx->type_name)
                );
            }

            dec_ref(cont);
            dec_ref(idx);
            return;
        }
        else
            nst_push_val(nst_state.v_stack, res);
    }

    dec_ref(res);
    dec_ref(cont);
    dec_ref(idx);
}

static inline void exe_dec_int()
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_peek_val(nst_state.v_stack);
    AS_INT(obj) -= 1;
}

static inline void exe_new_obj()
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_peek_val(nst_state.v_stack);
    Nst_Obj *new_obj = nst_new_int(AS_INT(obj));
    nst_push_val(nst_state.v_stack, new_obj);
    dec_ref(new_obj);
}

static inline void exe_dup()
{
    CHECK_V_STACK;
    nst_push_val(nst_state.v_stack, nst_peek_val(nst_state.v_stack));
}

static inline void exe_make_seq(Nst_RuntimeInstruction inst)
{
    Nst_Obj *seq = inst.id == NST_IC_MAKE_ARR ? nst_new_array(inst.int_val)
                                              : nst_new_vector(inst.int_val);
    CHECK_V_STACK_SIZE(inst.int_val);

    for ( Nst_Int i = 1; i <= inst.int_val; i++ )
    {
        Nst_Obj *curr_obj = nst_pop_val(nst_state.v_stack);
        nst_set_value_seq(curr_obj, inst.int_val - i, curr_obj);
        dec_ref(curr_obj);
    }

    nst_push_val(nst_state.v_stack, seq);
    dec_ref(seq);
}

static inline void exe_make_seq_rep(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *size_obj = nst_pop_val(nst_state.v_stack);
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);

    Nst_Int size = AS_INT(size_obj);
    dec_ref(size_obj);
    Nst_Obj *seq = inst.id == NST_IC_MAKE_ARR ? nst_new_array(size)
                                              : nst_new_vector(size);

    for ( Nst_Int i = 1; i <= size; i++ )
        nst_set_value_seq(seq, inst.int_val - i, val);

    nst_push_val(nst_state.v_stack, seq);
    dec_ref(seq);
    dec_ref(val);
}

static inline void exe_make_map(Nst_RuntimeInstruction inst)
{
    Nst_Obj *map = nst_new_map();
    CHECK_V_STACK_SIZE(inst.int_val);

    for ( Nst_Int i = 0; i < inst.int_val; i++ )
    {
        Nst_Obj *val = nst_pop_val(nst_state.v_stack);
        Nst_Obj *key = nst_pop_val(nst_state.v_stack);
        i++;
        nst_map_set(map, key, val);
    }

    nst_push_val(nst_state.v_stack, map);
    dec_ref(map);
}

static inline void exe_for_is_done()
{
    assert(false);
}

static inline void exe_for_get_val()
{
    assert(false);
}

size_t nst_get_full_path(char *file_path, char **buf, char **file_part)
{
    char *path = malloc(sizeof(char) * MAX_PATH);
    if ( path == NULL )
        return 0;

    DWORD path_len = GetFullPathNameA(file_path, MAX_PATH, path, file_part);
    if ( path_len > MAX_PATH )
    {
        free(path);
        path = malloc(sizeof(char) * path_len);
        if ( path == NULL )
            return 0;
        path_len = GetFullPathNameA(file_path, path_len, path, file_part);
    }

    *buf = path;
    return path_len;
}
