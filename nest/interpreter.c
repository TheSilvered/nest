#include <assert.h>
#include <windows.h>
#include <direct.h>
#include "interpreter.h"
#include "error_internal.h"
#include "obj_ops.h"
#include "hash.h"
#include "tokens.h"
#include "iter.h"

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

static inline void run_instruction(Nst_RuntimeInstruction inst);
static inline void exe_no_op();
static inline void exe_pop_val(Nst_RuntimeInstruction inst);
static inline void exe_for_start(Nst_RuntimeInstruction inst);
static inline void exe_for_is_done(Nst_RuntimeInstruction inst);
static inline void exe_for_get_val(Nst_RuntimeInstruction inst);
static inline void exe_for_advance(Nst_RuntimeInstruction inst);
static inline void exe_return_val();
static inline void exe_set_val_loc(Nst_RuntimeInstruction inst);
static inline void exe_jump(Nst_RuntimeInstruction inst);
static inline void exe_jumpif_t(Nst_RuntimeInstruction inst);
static inline void exe_jumpif_f(Nst_RuntimeInstruction inst);
static inline void exe_jumpif_zero(Nst_RuntimeInstruction inst);
static inline void exe_type_check(Nst_RuntimeInstruction inst);
static inline void exe_hash_check(Nst_RuntimeInstruction inst);
static inline void exe_set_val(Nst_RuntimeInstruction inst);
static inline void exe_get_val(Nst_RuntimeInstruction inst);
static inline void exe_push_val(Nst_RuntimeInstruction inst);
static inline void exe_set_cont_val(Nst_RuntimeInstruction inst);
static inline void exe_op_call(Nst_RuntimeInstruction inst);
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

static Nst_SeqObj *make_argv(int argc, char **argv);

void nst_run(Nst_BcFuncObj *main_func, int argc, char **argv)
{
    // nst_state global variable initialization
    char *cwd_buf = malloc(sizeof(char) * MAX_PATH);
    bool *error_occurred = malloc(sizeof(bool));
    if ( cwd_buf == NULL || error_occurred == NULL )
        return;

    *error_occurred = false;
    Nst_StrObj *cwd = AS_STR(nst_new_string_raw(_getcwd(cwd_buf, MAX_PATH), true));
    Nst_Traceback tb = { NULL, LList_new() };
    Nst_Int i = 0;

    nst_state.traceback = &tb;
    nst_state.vt = nst_new_var_table(NULL, cwd, make_argv(argc, argv));
    nst_state.idx = &i;
    nst_state.error_occurred = error_occurred;
    nst_state.curr_path = cwd;
    nst_state.v_stack = nst_new_val_stack();
    nst_state.f_stack = nst_new_call_stack();
    nst_state.loaded_libs = LList_new();
    nst_state.lib_paths = LList_new();
    nst_state.lib_handles = LList_new();

    register Nst_InstructionList *curr_inst_ls = main_func->body;

    nst_push_func(
        nst_state.f_stack,
        main_func,
        nst_no_pos(),
        nst_no_pos(),
        NULL,
        0
    );

    for ( ; nst_state.f_stack->current_size > 0; i++ )
    {
        if ( i >= (Nst_Int)curr_inst_ls->total_size )
        {
            Nst_FuncCall call = nst_pop_func(nst_state.f_stack);
            if ( nst_peek_val(nst_state.v_stack) != (Nst_Obj *)nst_state.vt->vars )
                destroy_obj((Nst_Obj *)nst_state.vt->vars);
            free(nst_state.vt);
            dec_ref(call.func);
            nst_state.vt = call.vt;
            i = call.idx;
            Nst_BcFuncObj *func = nst_peek_func(nst_state.f_stack).func;
            curr_inst_ls = func == NULL ? NULL : func->body;
            continue;
        }

        Nst_RuntimeInstruction inst = curr_inst_ls->instructions[i];
        run_instruction(inst);

        if ( *(nst_state.error_occurred) )
            break;

        // only OP_CALL pushes a function on the stack
        if ( inst.id == NST_IC_OP_CALL || inst.id == NST_IC_FOR_START ||
             inst.id == NST_IC_FOR_IS_DONE || inst.id == NST_IC_FOR_GET_VAL || 
             inst.id == NST_IC_FOR_ADVANCE )
            curr_inst_ls = nst_peek_func(nst_state.f_stack).func->body;
    }

    if ( *(nst_state.error_occurred) )
    {
        Nst_FuncCall *calls = nst_state.f_stack->stack;
        for ( Nst_Int i = 0, n = nst_state.f_stack->current_size - 1; i < n; i++ )
        {
            if ( calls[n - i].start.filename == NULL ) // i.e. there is no valid position
                continue;
            LList_append(nst_state.traceback->positions, &(calls[n - i].start), false);
            LList_append(nst_state.traceback->positions, &(calls[n - i].end)  , false);
        }

        nst_print_traceback(*nst_state.traceback);
        free(nst_state.traceback->error);

        nst_destroy_map(nst_state.vt->vars);
        free(nst_state.vt);
    }

    // Freeing nst_state
    LList_destroy(nst_state.traceback->positions, NULL);
    free(error_occurred);
    dec_ref(cwd);
    nst_destroy_v_stack(nst_state.v_stack);
    nst_destroy_f_stack(nst_state.f_stack);
    for ( LLNode *n = nst_state.loaded_libs->head; n != NULL; n = n->next )
    {
        void (*free_lib_func)() = (void (*)())GetProcAddress(n->value, "free_lib");
        if ( free_lib_func != NULL )
            free_lib_func();
    }
    LList_destroy(nst_state.loaded_libs, (void (*)(void *))FreeLibrary);
    LList_destroy(nst_state.lib_paths, free);
    LList_destroy(nst_state.lib_handles, free);
}

/*static void complete_function(size_t final_stack_size)
{
    if ( nst_state.f_stack->current_size == 0 )
        return;

    register Nst_InstructionList *curr_inst_ls = nst_peek_func(nst_state.f_stack).func->body;

    for ( ; nst_state.f_stack->current_size > final_stack_size; (*nst_state.idx)++ )
    {
        if ( *nst_state.idx >= (Nst_Int)curr_inst_ls->total_size )
        {
            Nst_FuncCall call = nst_pop_func(nst_state.f_stack);
            nst_destroy_map(nst_state.vt->vars);
            free(nst_state.vt);
            dec_ref(call.func);
            nst_state.vt = call.vt;
            *nst_state.idx = call.idx;
            Nst_BcFuncObj *func = nst_peek_func(nst_state.f_stack).func;
            curr_inst_ls = func == NULL ? NULL : func->body;
            continue;
        }

        Nst_RuntimeInstruction inst = curr_inst_ls->instructions[*nst_state.idx];
        run_instruction(inst);

        if ( *(nst_state.error_occurred) )
            break;

        // only OP_CALL pushes a function on the stack
        if ( inst.id == NST_IC_OP_CALL )
            curr_inst_ls = nst_peek_func(nst_state.f_stack).func->body;
    }
}*/

Nst_MapObj *nst_run_module(char *file_name)
{
    return NULL;
}

Nst_Obj *nst_call_func(Nst_BcFuncObj *func, Nst_Obj **args, Nst_OpErr *err)
{
    return NULL;
}

static inline void run_instruction(Nst_RuntimeInstruction inst)
{
    switch ( inst.id )
    {
    case NST_IC_POP_VAL:      exe_pop_val(inst);          break;
    case NST_IC_FOR_START:    exe_for_start(inst);        break;
    case NST_IC_FOR_IS_DONE:  exe_for_is_done(inst);      break;
    case NST_IC_FOR_GET_VAL:  exe_for_get_val(inst);      break;
    case NST_IC_FOR_ADVANCE:  exe_for_advance(inst);      break;
    case NST_IC_RETURN_VAL:   exe_return_val();           break;
    case NST_IC_SET_VAL_LOC:  exe_set_val_loc(inst);      break;
    case NST_IC_JUMP:         exe_jump(inst);             break;
    case NST_IC_JUMPIF_T:     exe_jumpif_t(inst);         break;
    case NST_IC_JUMPIF_F:     exe_jumpif_f(inst);         break;
    case NST_IC_JUMPIF_ZERO:  exe_jumpif_zero(inst);      break;
    case NST_IC_TYPE_CHECK:   exe_type_check(inst);       break;
    case NST_IC_HASH_CHECK:   exe_hash_check(inst);       break;
    case NST_IC_SET_VAL:      exe_set_val(inst);          break;
    case NST_IC_GET_VAL:      exe_get_val(inst);          break;
    case NST_IC_PUSH_VAL:     exe_push_val(inst);         break;
    case NST_IC_SET_CONT_VAL: exe_set_cont_val(inst);     break;
    case NST_IC_OP_CALL:      exe_op_call(inst);          break;
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
    }
}

static inline void exe_no_op() {}

static inline void exe_pop_val(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *obj = nst_pop_val(nst_state.v_stack);
    dec_ref(obj);
}

static inline void exe_for_inst(Nst_RuntimeInstruction inst, Nst_IterObj *iter, Nst_BcFuncObj *func)
{
    if ( func->cbody != NULL )
    {
        Nst_OpErr err = { "", "" };
        Nst_Obj *res = func->cbody(inst.int_val, &iter->value, &err);

        if ( res == NULL )
        {
            SET_ERROR(
                _NST_SET_GENERAL_ERROR,
                inst.start,
                inst.end,
                err.message
            );
            nst_state.traceback->error->name = err.name;
        }
        else
        {
            nst_push_val(nst_state.v_stack, res);
            dec_ref(res);
        }
    }
    else
    {
        nst_push_val(nst_state.v_stack, NULL);
        nst_push_val(nst_state.v_stack, iter->value);
        nst_push_val(nst_state.v_stack, func);
        exe_op_call(inst);
    }
}

static inline void exe_for_start(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = AS_ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->start);
}

static inline void exe_for_is_done(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = AS_ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->is_done);
}

static inline void exe_for_get_val(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = AS_ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->get_val);
}

static inline void exe_for_advance(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_IterObj *iter = AS_ITER(nst_peek_val(nst_state.v_stack));
    exe_for_inst(inst, iter, iter->advance);
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
    *nst_state.idx = nst_peek_func(nst_state.f_stack).func->body->total_size;
    dec_ref(result);
}

static inline void exe_set_val_loc(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = nst_pop_val(nst_state.v_stack);
    nst_set_val(nst_state.vt, inst.val, val);
    dec_ref(val);
}

static inline void exe_jump(Nst_RuntimeInstruction inst)
{
    *nst_state.idx = inst.int_val - 1;
}

static inline void exe_jumpif_t(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t_bool, NULL);
    dec_ref(top_val);

    if ( result == nst_true )
        *nst_state.idx = inst.int_val - 1;
    dec_ref(result);
}

static inline void exe_jumpif_f(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *top_val = nst_pop_val(nst_state.v_stack);
    Nst_Obj *result = nst_obj_cast(top_val, nst_t_bool, NULL);
    dec_ref(top_val);

    if ( result == nst_false )
        *nst_state.idx = inst.int_val - 1;
    dec_ref(result);
}

static inline void exe_jumpif_zero(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK;
    Nst_Obj *val = nst_peek_val(nst_state.v_stack);
    if ( AS_INT(val) == 0 )
        *nst_state.idx = inst.int_val - 1;
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

static inline void exe_set_cont_val(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK_SIZE(3);
    Nst_Obj *idx = nst_pop_val(nst_state.v_stack);
    Nst_Obj *cont = nst_pop_val(nst_state.v_stack);
    Nst_Obj *val = nst_peek_val(nst_state.v_stack);

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

        bool res = nst_set_value_seq(cont, AS_INT(idx), val);

        if ( !res )
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
        }

        dec_ref(cont);
        dec_ref(idx);
    }
    else if ( cont->type == nst_t_map )
    {
        bool res = nst_map_set(cont, idx, val);
        if ( !res )
        {
            SET_ERROR(
                _NST_SET_TYPE_ERROR,
                inst.start,
                inst.end,
                _nst_format_type_error(UNHASHABLE_TYPE, idx->type_name)
            );
        }

        dec_ref(cont);
        dec_ref(idx);
    }
    else
        SET_ERROR(
            _NST_SET_TYPE_ERROR,
            inst.start,
            inst.end,
            _nst_format_type_error(
                EXPECTED_TYPE("Array', 'Vector' or 'Map"),
                cont->type_name
            )
        );
}

static inline void exe_op_call(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK_SIZE(inst.int_val + 1);
    Nst_BcFuncObj *func = AS_BFUNC(nst_pop_val(nst_state.v_stack));

    if ( func->arg_num != inst.int_val )
    {
        SET_ERROR(
            _NST_SET_CALL_ERROR,
            inst.start,
            inst.end,
            inst.int_val > (Nst_Int)func->arg_num ? TOO_MANY_ARGS_FUNC
                                                  : TOO_FEW_ARGS_FUNC
        );

        dec_ref(func);
        return;
    }

    if ( func->cbody != NULL )
    {
        Nst_OpErr err = { "", "" };
        Nst_Obj **args;
        Nst_Obj *arg;
        bool args_allocated = false;

        if ( inst.int_val == 0 )
            args = NULL;
        else if ( inst.int_val == 1 )
        {
            arg = nst_pop_val(nst_state.v_stack);
            args = &arg;
        }
        else
        {
            args = malloc(sizeof(Nst_Obj *) * inst.int_val);
            if ( args == NULL )
                return;

            for ( Nst_Int i = 0; i < inst.int_val; i++ )
                args[i] = nst_pop_val(nst_state.v_stack);
            args_allocated = true;
        }

        Nst_Obj *res = func->cbody(inst.int_val, args, &err);

        for ( Nst_Int i = 0; i < inst.int_val; i++ )
            dec_ref(args[i]);

        if ( args_allocated )
            free(args);

        if ( res == NULL )
        {
            SET_ERROR(
                _NST_SET_GENERAL_ERROR,
                inst.start,
                inst.end,
                err.message
            );
            nst_state.traceback->error->name = err.name;
        }
        else
        {
            nst_push_val(nst_state.v_stack, res);
            dec_ref(res);
        }

        dec_ref(func);
        return;
    }

    assert(func->body != NULL);

    bool res = nst_push_func(
        nst_state.f_stack,
        func,
        inst.start,
        inst.end,
        nst_state.vt,
        *nst_state.idx
    );
    *nst_state.idx = -1;

    if ( !res )
    {
        SET_ERROR(
            _NST_SET_CALL_ERROR,
            inst.start,
            inst.end,
            CALL_STACK_SIZE_EXCEEDED
        );

        dec_ref(func);
        return;
    }

    Nst_VarTable *new_vt;

    if ( nst_state.vt->global_table == NULL )
        new_vt = nst_new_var_table(nst_state.vt, NULL, NULL);
    else
        new_vt = nst_new_var_table(nst_state.vt->global_table, NULL, NULL);

    for ( Nst_Int i = 0; i < inst.int_val; i++ )
    {
        Nst_Obj *val = nst_pop_val(nst_state.v_stack);
        nst_set_val(new_vt, func->args[inst.int_val - i - 1], val);
        dec_ref(val);
    }
    nst_state.vt = new_vt;
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

    Nst_Obj *iter = nst_new_iter(
        AS_BFUNC(new_cfunc(1, nst_num_iter_start)),
        AS_BFUNC(new_cfunc(1, nst_num_iter_advance)),
        AS_BFUNC(new_cfunc(1, nst_num_iter_is_done)),
        AS_BFUNC(new_cfunc(1, nst_num_iter_get_val)),
        data_seq
    );

    nst_push_val(nst_state.v_stack, iter);
    dec_ref(iter);
}

static inline void exe_stack_op(Nst_RuntimeInstruction inst)
{
    CHECK_V_STACK_SIZE(2);
    Nst_Obj *ob2 = nst_pop_val(nst_state.v_stack);
    Nst_Obj *ob1 = nst_pop_val(nst_state.v_stack);
    Nst_Obj *res = NULL;
    Nst_OpErr err = { "", "" };

    switch ( inst.int_val )
    {
    case NST_TT_ADD:    res = nst_obj_add(ob1, ob2, &err);   break;
    case NST_TT_SUB:    res = nst_obj_sub(ob1, ob2, &err);   break;
    case NST_TT_MUL:    res = nst_obj_mul(ob1, ob2, &err);   break;
    case NST_TT_DIV:    res = nst_obj_div(ob1, ob2, &err);   break;
    case NST_TT_POW:    res = nst_obj_pow(ob1, ob2, &err);   break;
    case NST_TT_MOD:    res = nst_obj_mod(ob1, ob2, &err);   break;
    case NST_TT_B_AND:  res = nst_obj_bwand(ob1, ob2, &err); break;
    case NST_TT_B_OR:   res = nst_obj_bwor(ob1, ob2, &err);  break;
    case NST_TT_B_XOR:  res = nst_obj_bwxor(ob1, ob2, &err); break;
    case NST_TT_LSHIFT: res = nst_obj_bwls(ob1, ob2, &err);  break;
    case NST_TT_RSHIFT: res = nst_obj_bwrs(ob1, ob2, &err);  break;
    case NST_TT_CONCAT: res = nst_obj_concat(ob1, ob2, &err);break;
    case NST_TT_L_AND:  res = nst_obj_lgand(ob1, ob2, &err); break;
    case NST_TT_L_OR:   res = nst_obj_lgor(ob1, ob2, &err);  break;
    case NST_TT_L_XOR:  res = nst_obj_lgxor(ob1, ob2, &err); break;
    case NST_TT_GT:     res = nst_obj_gt(ob1, ob2, &err);    break;
    case NST_TT_LT:     res = nst_obj_lt(ob1, ob2, &err);    break;
    case NST_TT_EQ:     res = nst_obj_eq(ob1, ob2, &err);    break;
    case NST_TT_NEQ:    res = nst_obj_ne(ob1, ob2, &err);    break;
    case NST_TT_GTE:    res = nst_obj_ge(ob1, ob2, &err);    break;
    case NST_TT_LTE:    res = nst_obj_le(ob1, ob2, &err);    break;
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
    case NST_TT_LEN:    res = nst_obj_len(obj, &err);   break;
    case NST_TT_L_NOT:  res = nst_obj_lgnot(obj, &err); break;
    case NST_TT_B_NOT:  res = nst_obj_bwnot(obj, &err); break;
    case NST_TT_STDOUT: res = nst_obj_stdout(obj, &err);break;
    case NST_TT_STDIN:  res = nst_obj_stdin(obj, &err); break;
    case NST_TT_NEG:    res = nst_obj_neg(obj, &err);   break;
    case NST_TT_TYPEOF: res = nst_obj_typeof(obj, &err);break;
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
        nst_set_value_seq(seq, inst.int_val - i, curr_obj);
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
    Nst_Obj *seq = inst.id == NST_IC_MAKE_ARR_REP ? nst_new_array(size)
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
        dec_ref(val);
        dec_ref(key);
    }

    nst_push_val(nst_state.v_stack, map);
    dec_ref(map);
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

static Nst_SeqObj *make_argv(int argc, char **argv)
{
    Nst_SeqObj *args = AS_SEQ(nst_new_array(argc - 1));

    for ( int i = 1; i < argc; i++ )
    {
        Nst_Obj *val = nst_new_string_raw(argv[i], false);
        nst_set_value_seq(args, i - 1, val);
        dec_ref(val);
    }

    return args;
}
