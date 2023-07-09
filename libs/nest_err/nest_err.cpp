#include <cstdlib>
#include "nest_err.h"

#define FUNC_COUNT 2

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(try_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_err_names_, 0);

#if __LINE__ - FUNC_COUNT != 15
#error
#endif

    lib_init_ = !Nst_error_occurred();
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

    Nst_Obj *map = Nst_map_new();

    Nst_Obj *arr1 = Nst_array_new(2);
    Nst_Obj *arr2 = Nst_array_new(2);
    Nst_Obj *file_str = Nst_string_new_c_raw(start.text->path, false);

    SEQ(arr1)->objs[0] = Nst_int_new(start.line);
    SEQ(arr1)->objs[1] = Nst_int_new(start.col);
    SEQ(arr2)->objs[0] = Nst_int_new(end.line);
    SEQ(arr2)->objs[1] = Nst_int_new(end.col);

    Nst_map_set_str(map, "file", file_str);
    Nst_map_set_str(map, "start", arr1);
    Nst_map_set_str(map, "end", arr2);

    Nst_dec_ref(file_str);
    Nst_dec_ref(arr1);
    Nst_dec_ref(arr2);

    return map;
}

Nst_Obj *success(Nst_Obj *val)
{
    Nst_Obj *map = Nst_map_new();
    Nst_map_set_str(map, "value", val);
    Nst_map_set_str(map, "error", Nst_null());
    Nst_map_set_str(map, "traceback", Nst_null());
    Nst_dec_ref(val);

    return map;
}

Nst_Obj *failure(bool catch_exit)
{
    Nst_Obj *map = Nst_map_new();
    Nst_Obj *error_map = Nst_map_new();
    Nst_Obj *error_name_str;
    Nst_Obj *error_message_str;
    Nst_Obj *error_pos;
    Nst_Obj *error_traceback;
    Nst_ExecutionState *state = Nst_get_state();
    Nst_map_set_str(map, "value", Nst_null());

    if ( state->traceback.error.occurred )
    {
        Nst_Error error = state->traceback.error;
        error_name_str = OBJ(error.name);
        error_message_str = OBJ(error.message);

        if ( OBJ(error_name_str) == Nst_null() && !catch_exit )
        {
            Nst_dec_ref(map);
            Nst_dec_ref(error_map);
            return nullptr;
        }

        error_pos = make_pos(error.start, error.end);
        error_traceback =
            Nst_array_new(state->traceback.positions->size / 2);

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

        Nst_llist_empty(positions, Nst_free);
        state->traceback.error.occurred = false;
    }
    else
    {
        Nst_OpErr *err = Nst_error_get();
        error_name_str = OBJ(err->name);
        error_message_str = OBJ(err->message);

        if ( OBJ(error_name_str) == Nst_null() && !catch_exit )
        {
            Nst_dec_ref(map);
            Nst_dec_ref(error_map);
            return nullptr;
        }

        error_pos = Nst_inc_ref(Nst_null());
        error_traceback = Nst_inc_ref(Nst_null());
    }

    Nst_map_set_str(error_map, "name", error_name_str);
    Nst_map_set_str(error_map, "message", error_message_str);
    Nst_map_set_str(error_map, "pos", error_pos);
    Nst_map_set_str(map, "error", error_map);
    Nst_map_set_str(map, "traceback", error_traceback);

    Nst_dec_ref(error_name_str);
    Nst_dec_ref(error_message_str);
    Nst_dec_ref(error_pos);
    Nst_dec_ref(error_map);
    Nst_dec_ref(error_traceback);

    return map;
}

Nst_FUNC_SIGN(try_)
{
    Nst_FuncObj *func;
    Nst_SeqObj *func_args;
    Nst_Obj *catch_exit_obj;

    Nst_DEF_EXTRACT("fA?b", &func, &func_args, &catch_exit_obj);
    bool catch_exit = Nst_DEF_VAL(catch_exit_obj, AS_BOOL(catch_exit_obj), false);

    if ( func_args->len != func->arg_num )
    {
        Nst_set_call_error(Nst_sprintf(
            "the function expected %zi arguments but the %s had length %zi",
            func->arg_num, TYPE_NAME(func_args), func_args->len));

        return nullptr;
    }

    Nst_Obj *result = Nst_call_func(func, func_args->objs);

    if ( result != nullptr )
    {
        return success(result);
    }
    else
    {
        return failure(catch_exit);
    }
}

Nst_FUNC_SIGN(_get_err_names_)
{
    Nst_Obj *names = Nst_array_create_c(
        "OOOOOOO",
        Nst_str()->e_SyntaxError,
        Nst_str()->e_ValueError,
        Nst_str()->e_TypeError,
        Nst_str()->e_CallError,
        Nst_str()->e_MemoryError,
        Nst_str()->e_MathError,
        Nst_str()->e_ImportError);

    return names;
}
