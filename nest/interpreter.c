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
        nst_state->traceback->error = error; \
        nst_state->error_occurred = true; \
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
        nst_state->traceback->error = error; \
        nst_state->error_occurred = true; \
    } while ( 0 )

#define CHECK_V_STACK assert(nst_state->v_stack->current_size != 0)
#define CHECK_V_STACK_SIZE(size) assert(nst_state->v_stack->current_size >= size)
#define CHECK_F_STACK assert(nst_state->f_stack->current_size != 0)

Nst_ExecutionState *nst_state;

static void run_instruction(Nst_InstructionList *inst_ls, Nst_Int *idx);

void nst_run(Nst_InstructionList *inst_ls, int argc, char **argv)
{
    
}

Nst_MapObj *nst_run_module(char *file_name);
Nst_Obj *nst_call_func(Nst_BcFuncObj *func, Nst_Obj **args, Nst_OpErr *err);

static void run_instruction(Nst_InstructionList *inst_ls, Nst_Int *idx)
{
    Nst_RuntimeInstruction inst = inst_ls->instructions[*idx];
    Nst_Obj *obj = NULL;
    Nst_Obj *obj2 = NULL;
    Nst_Obj *obj3 = NULL;
    Nst_Int num = NULL;
    Nst_OpErr err = { "", "" };

    switch ( inst.id )
    {
    case NST_IC_POP_VAL:
        CHECK_V_STACK;
        dec_ref(nst_pop_val(nst_state->v_stack));
        break;

    case NST_IC_PUSH_FUNC:
        if ( !nst_push_func(nst_state->f_stack, nst_peek_val(nst_state->v_stack)) )
            SET_ERROR(
                _NST_SET_CALL_ERROR,
                inst.start,
                inst.end,
                CALL_STACK_SIZE_EXCEEDED
            );
        break;

    case NST_IC_POP_FUNC:
        CHECK_F_STACK;
        nst_pop_func(nst_state->f_stack);
        break;
    
    case NST_IC_FOR_START: break;                                // TODO
    case NST_IC_RETURN_VAL:
        obj = nst_pop_val(nst_state->v_stack);
        obj2 = nst_pop_val(nst_state->v_stack);

        while ( obj2 != NULL )
        {
            dec_ref(obj2);
            obj2 = nst_pop_val(nst_state->v_stack);
        }

        nst_push_val(nst_state->v_stack, obj);
        break;

    case NST_IC_FOR_ADVANCE: break;                               // TODO
    case NST_IC_SET_VAL_LOC:
        CHECK_V_STACK;
        obj = nst_pop_val(nst_state->v_stack);
        nst_set_val(nst_state->vt, inst.val, obj);
        dec_ref(obj);
        break;

    case NST_IC_JUMP:
        *idx = inst.int_val - 1;
        break;

    case NST_IC_JUMPIF_T:
        CHECK_V_STACK;
        obj = nst_pop_val(nst_state->v_stack);
        obj2 = nst_obj_cast(obj, nst_t_bool, NULL);
        dec_ref(obj);

        if ( nst_pop_val(nst_state->v_stack) == nst_true )
            *idx = inst.int_val - 1;
        dec_ref(obj2);
        break;

    case NST_IC_JUMPIF_F:
        CHECK_V_STACK;
        obj = nst_pop_val(nst_state->v_stack);
        obj2 = nst_obj_cast(obj, nst_t_bool, NULL);
        dec_ref(obj);

        if ( nst_pop_val(nst_state->v_stack) == nst_false )
            *idx = inst.int_val - 1;
        dec_ref(obj2);
        break;

    case NST_IC_JUMPIF_ZERO:
        CHECK_V_STACK;
        obj = nst_pop_val(nst_state->v_stack);
        if ( AS_INT(obj) == 0 )
            *idx = inst.int_val - 1;
        dec_ref(obj);
        break;

    case NST_IC_TYPE_CHECK:
        CHECK_V_STACK;
        obj = nst_peek_val(nst_state->v_stack);
        if ( obj->type != inst.val )
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                inst.start,
                inst.end,
                _nst_format_types_error(EXPECTED_TYPES, AS_STR(inst.val)->value, obj->type_name);
        );
        break;

    case NST_IC_HASH_CHECK:
        CHECK_V_STACK;
        obj = nst_peek_val(nst_state->v_stack);
        nst_hash_obj(obj);
        if ( obj->hash == -1 )
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                inst.start,
                inst.end,
                _nst_format_type_error(UNHASHABLE_TYPE, obj->type_name);
            );
        break;

    case NST_IC_SET_VAL:
        CHECK_V_STACK;
        nst_set_val(nst_state->vt, inst.val, nst_peek_val(nst_state->v_stack));
        break;

    case NST_IC_GET_VAL:
        obj = nst_get_val(nst_state->vt, inst.val);
        if ( obj == NULL )
            nst_push_val(nst_state->vt, nst_null);
        else
            nst_push_val(nst_state->vt, obj);
        break;

    case NST_IC_PUSH_VAL:
        nst_push_val(nst_state->v_stack, inst.val);
        break;

    case NST_IC_SET_CONT_VAL: break;
    case NST_IC_OP_CALL: break;
    case NST_IC_OP_CAST: break;
    case NST_IC_OP_RANGE: break;
    case NST_IC_STACK_OP:
        CHECK_V_STACK_SIZE(2);
        obj = nst_pop_val(nst_state->v_stack);
        obj2 = nst_pop_val(nst_state->v_stack);

        switch ( inst.int_val )
        {
        case NST_TT_ADD:    obj3 = nst_obj_add(obj, obj2, &err);
        case NST_TT_SUB:    obj3 = nst_obj_sub(obj, obj2, &err);
        case NST_TT_MUL:    obj3 = nst_obj_mul(obj, obj2, &err);
        case NST_TT_DIV:    obj3 = nst_obj_div(obj, obj2, &err);
        case NST_TT_POW:    obj3 = nst_obj_pow(obj, obj2, &err);
        case NST_TT_MOD:    obj3 = nst_obj_mod(obj, obj2, &err);
        case NST_TT_B_AND:  obj3 = nst_obj_bwand(obj, obj2, &err);
        case NST_TT_B_OR:   obj3 = nst_obj_bwor(obj, obj2, &err);
        case NST_TT_B_XOR:  obj3 = nst_obj_bwxor(obj, obj2, &err);
        case NST_TT_LSHIFT: obj3 = nst_obj_bwls(obj, obj2, &err);
        case NST_TT_RSHIFT: obj3 = nst_obj_bwrs(obj, obj2, &err);
        case NST_TT_CONCAT: obj3 = nst_obj_concat(obj, obj2, &err);
        case NST_TT_L_AND:  obj3 = nst_obj_lgand(obj, obj2, &err);
        case NST_TT_L_OR:   obj3 = nst_obj_lgor(obj, obj2, &err);
        case NST_TT_L_XOR:  obj3 = nst_obj_lgxor(obj, obj2, &err);
        case NST_TT_GT:     obj3 = nst_obj_gt(obj, obj2, &err);
        case NST_TT_LT:     obj3 = nst_obj_lt(obj, obj2, &err);
        case NST_TT_EQ:     obj3 = nst_obj_eq(obj, obj2, &err);
        case NST_TT_NEQ:    obj3 = nst_obj_ne(obj, obj2, &err);
        case NST_TT_GTE:    obj3 = nst_obj_ge(obj, obj2, &err);
        case NST_TT_LTE:    obj3 = nst_obj_le(obj, obj2, &err);
        }

        if ( obj3 == NULL )
        {
            SET_OP_ERROR(inst.start, inst.end, err);
            dec_ref(obj3);
            break;
        }

        nst_push_val(nst_state->v_stack, obj3);
        dec_ref(obj);
        dec_ref(obj2);
        dec_ref(obj3);
        break;

    case NST_IC_LOCAL_OP:
        CHECK_V_STACK;
        obj = nst_pop_val(nst_state->v_stack);

        switch ( inst.int_val )
        {
        case NST_TT_LEN:    obj2 = nst_obj_len(obj, &err);
        case NST_TT_L_NOT:  obj2 = nst_obj_lgnot(obj, &err);
        case NST_TT_B_NOT:  obj2 = nst_obj_bwnot(obj, &err);
        case NST_TT_STDOUT: obj2 = nst_obj_stdout(obj, &err);
        case NST_TT_STDIN:  obj2 = nst_obj_stdin(obj, &err);
        case NST_TT_NEG:    obj2 = nst_obj_neg(obj, &err);
        case NST_TT_TYPEOF: obj2 = nst_obj_typeof(obj, &err);
        }

        if ( obj2 == NULL )
        {
            SET_OP_ERROR(inst.start, inst.end, err);
            dec_ref(obj);
            break;
        }

        nst_push_val(nst_state->v_stack, obj2);
        dec_ref(obj);
        dec_ref(obj2);
        break;

    case NST_IC_OP_IMPORT: break;

    case NST_IC_OP_EXTRACT:
        CHECK_V_STACK_SIZE(2);
        obj  = nst_pop_val(nst_state->v_stack); // Index
        obj2 = nst_pop_val(nst_state->v_stack); // Container

        if ( obj2->type == nst_t_arr || obj2->type == nst_t_vect )
        {
            if ( obj->type != nst_t_int )
            {
                SET_ERROR(
                    _NST_SET_TYPE_ERROR,
                    inst.start,
                    inst.end,
                    _nst_format_type_error(EXPECTED_TYPE("Int"), obj->type_name)
                );

                dec_ref(obj2);
                dec_ref(obj);
                break;
            }

            num = AS_INT(obj);
            obj3 = nst_get_value_seq(obj2, num);

            if ( obj3 == NULL )
            {
                SET_ERROR(
                    _NST_SET_VALUE_ERROR,
                    inst.start,
                    inst.end,
                    _nst_format_idx_error(
                        obj2->type == nst_t_arr ? INDEX_OUT_OF_BOUNDS("Array")
                                                : INDEX_OUT_OF_BOUNDS("Vector"),
                        idx,
                        AS_SEQ(obj2)->len
                    )
                );

                dec_ref(obj2);
                dec_ref(obj);
                break;
            }

            nst_push_val(nst_state->v_stack, obj3);
        }
        else
        {
            assert(obj2->type == nst_t_map);
            obj3 = nst_map_get(obj2, obj);

            if ( obj3 == NULL )
            {
                if ( obj->hash != -1 )
                    nst_push_val(nst_state->v_stack, nst_null);
                else
                {
                    SET_ERROR(
                        _NST_SET_VALUE_ERROR,
                        inst.start,
                        inst.start,
                        _nst_format_type_error(UNHASHABLE_TYPE, obj->type_name)
                    );
                }

                dec_ref(obj2);
                dec_ref(obj);
                break;
            }
            else
                nst_push_val(nst_state->v_stack, obj3);
        }

        dec_ref(obj3);
        dec_ref(obj2);
        dec_ref(obj);
        break;

    case NST_IC_DEC_INT:
        CHECK_V_STACK;
	obj = nst_peek_val(nst_state->v_stack);
	AS_INT(obj) -= 1;
        break;

    case NST_IC_NEW_OBJ:
        CHECK_V_STACK;
        obj = nst_peek_val(nst_state->v_stack);
        obj2 = nst_new_int(AS_INT(obj));
        nst_push_val(nst_state->v_stack, obj2);
        dec_ref(obj2);
        break;

    case NST_IC_DUP:
        CHECK_V_STACK;
        nst_push_val(nst_state->v_stack, nst_peek_val(nst_state->f_stack));
        break;

    case NST_IC_MAKE_ARR:
    case NST_IC_MAKE_VEC:
        obj = inst.id == NST_IC_MAKE_ARR ? nst_new_array(inst.int_val)
                                         : nst_new_vector(inst.int_val);
        CHECK_V_STACK_SIZE(inst.int_val);

        for ( Nst_Int i = 1; i <= inst.int_val; i++ )
        {
            obj2 = nst_pop_val(nst_state->v_stack);
            nst_set_value_seq(obj, inst.int_val - i, obj2);
            dec_ref(obj2);
        }

        nst_push_val(nst_state->v_stack, obj);
        dec_ref(obj);
        break;

    case NST_IC_MAKE_ARR_REP:
    case NST_IC_MAKE_VEC_REP:
        CHECK_V_STACK_SIZE(2);
        obj2 = nst_pop_val(nst_state->v_stack);
        obj3 = nst_pop_val(nst_state->v_stack);

        num = AS_INT(obj2);
        obj = inst.id == NST_IC_MAKE_ARR ? nst_new_array(num)
                                         : nst_new_vector(num);

        for ( Nst_Int i = 1; i <= num; i++ )
            nst_set_value_seq(obj, inst.int_val - i, obj3);
        
        nst_push_val(nst_state->v_stack, obj);
        dec_ref(obj);
        dec_ref(obj3);
        break;

    case NST_IC_MAKE_MAP:
        obj = nst_new_map();
        CHECK_V_STACK_SIZE(inst.int_val);

        for ( Nst_Int i = 0; i < inst.int_val; i++ )
        {
            obj2 = nst_pop_val(nst_state->v_stack); // Value
            obj3 = nst_pop_val(nst_state->v_stack); // Key
            i++;
            nst_map_set(obj, obj3, obj2);
        }

        break;

    case NST_IC_FOR_IS_DONE: break;
    case NST_IC_FOR_GET_VAL: break;
    }
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
