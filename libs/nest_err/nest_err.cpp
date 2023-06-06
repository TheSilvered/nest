#include <cstdlib>
#include "nest_err.h"

#define FUNC_COUNT 2

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;
    Nst_OpErr err = { nullptr, nullptr };

    func_list_[idx++] = NST_MAKE_FUNCDECLR(try_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_err_names_, 0);

#if __LINE__ - FUNC_COUNT != 16
#error
#endif

    lib_init_ = err.name == nullptr;
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

Nst_Obj *make_pos(Nst_Pos start, Nst_Pos end)
{
    if ( start.text == nullptr )
    {
        return nullptr;
    }

    Nst_Obj *map = nst_map_new(nullptr);

    Nst_Obj *arr1 = nst_array_new(2, nullptr);
    Nst_Obj *arr2 = nst_array_new(2, nullptr);
    Nst_Obj *file_str = nst_string_new_c_raw(start.text->path, false, nullptr);

    SEQ(arr1)->objs[0] = nst_int_new(start.line, nullptr);
    SEQ(arr1)->objs[1] = nst_int_new(start.col, nullptr);
    SEQ(arr2)->objs[0] = nst_int_new(end.line, nullptr);
    SEQ(arr2)->objs[1] = nst_int_new(end.col, nullptr);

    nst_map_set_str(map, "file", file_str, nullptr);
    nst_map_set_str(map, "start", arr1, nullptr);
    nst_map_set_str(map, "end", arr2, nullptr);

    nst_dec_ref(file_str);
    nst_dec_ref(arr1);
    nst_dec_ref(arr2);

    return map;
}

Nst_Obj *success(Nst_Obj *val)
{
    Nst_Obj *map = nst_map_new(nullptr);
    nst_map_set_str(map, "value", val, nullptr);
    nst_map_set_str(map, "error", nst_null(), nullptr);
    nst_map_set_str(map, "traceback", nst_null(), nullptr);
    nst_dec_ref(val);

    return map;
}

Nst_Obj *failure(Nst_OpErr *err, bool catch_exit)
{
    Nst_Obj *map = nst_map_new(nullptr);
    Nst_Obj *error_map = nst_map_new(nullptr);
    Nst_Obj *error_name_str;
    Nst_Obj *error_message_str;
    Nst_Obj *error_pos;
    Nst_Obj *error_traceback;
    Nst_ExecutionState *state = nst_get_state();
    nst_map_set_str(map, "value", nst_null(), nullptr);

    if ( state->traceback.error.occurred )
    {
        Nst_Error error = state->traceback.error;
        error_name_str = OBJ(error.name);
        error_message_str = OBJ(error.message);

        if ( OBJ(error_name_str) == nst_null() && !catch_exit )
        {
            nst_dec_ref(map);
            nst_dec_ref(error_map);
            return nullptr;
        }

        error_pos = make_pos(error.start, error.end);
        error_traceback =
            nst_array_new(state->traceback.positions->size / 2, nullptr);

        Nst_LList *positions = state->traceback.positions;
        Nst_Int skipped = 0;
        Nst_LLNode *n1 = positions->head;
        Nst_LLNode *n2 = n1 == nullptr ? n1 : n1->next;
        for ( usize i = 0; n1 != nullptr; i++ )
        {
            Nst_Obj *pos = make_pos(*(Nst_Pos *)n1->value,
                                    *(Nst_Pos *)n2->value);

            n1 = n2->next;
            n2 = n1 == nullptr ? n1 : n1->next;

            if ( pos == nullptr )
            {
                SEQ(error_traceback)->len--;
                skipped++;
                continue;
            }

            SEQ(error_traceback)->objs[i - skipped] = pos;
        }

        nst_llist_empty(positions, nst_free);
        state->traceback.error.occurred = false;
    }
    else
    {
        error_name_str = OBJ(err->name);
        error_message_str = OBJ(err->message);

        if ( OBJ(error_name_str) == nst_null() && !catch_exit )
        {
            nst_dec_ref(map);
            nst_dec_ref(error_map);
            return nullptr;
        }

        error_pos = nst_inc_ref(nst_null());
        error_traceback = nst_inc_ref(nst_null());
    }

    nst_map_set_str(error_map, "name", error_name_str, nullptr);
    nst_map_set_str(error_map, "message", error_message_str, nullptr);
    nst_map_set_str(error_map, "pos", error_pos, nullptr);
    nst_map_set_str(map, "error", error_map, nullptr);
    nst_map_set_str(map, "traceback", error_traceback, nullptr);

    nst_dec_ref(error_name_str);
    nst_dec_ref(error_message_str);
    nst_dec_ref(error_pos);
    nst_dec_ref(error_map);
    nst_dec_ref(error_traceback);

    return map;
}

NST_FUNC_SIGN(try_)
{
    Nst_FuncObj *func;
    Nst_SeqObj *func_args;
    Nst_Obj *catch_exit_obj;

    NST_DEF_EXTRACT("fA?b", &func, &func_args, &catch_exit_obj);
    bool catch_exit = NST_DEF_VAL(catch_exit_obj, AS_BOOL(catch_exit_obj), false);

    if ( func_args->len != func->arg_num )
    {
        NST_SET_CALL_ERROR(nst_sprintf(
            "the function expected %zi arguments but the %s had length %zi",
            func->arg_num, TYPE_NAME(func_args), func_args->len));

        return nullptr;
    }

    Nst_Obj *result = nst_call_func(func, func_args->objs, err);

    if ( result != nullptr )
    {
        return success(result);
    }
    else
    {
        return failure(err, catch_exit);
    }
}

NST_FUNC_SIGN(_get_err_names_)
{
    Nst_Obj *names = nst_array_create_c(
        "OOOOOOO", err,
        nst_str()->e_SyntaxError,
        nst_str()->e_ValueError,
        nst_str()->e_TypeError,
        nst_str()->e_CallError,
        nst_str()->e_MemoryError,
        nst_str()->e_MathError,
        nst_str()->e_ImportError);

    return names;
}
