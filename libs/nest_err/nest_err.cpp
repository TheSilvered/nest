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
    if (start.text == nullptr)
        return nullptr;

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
    Nst_Obj *error_name_str = nullptr;
    Nst_Obj *error_message_str = nullptr;
    Nst_Obj *error_traceback = nullptr;
    Nst_Traceback *error = Nst_error_get();
    usize i = 0;

    if (!error->error_occurred) {
        Nst_set_value_error_c("invalid error state");
        return nullptr;
    }

    Nst_map_set_str(map, "value", Nst_null());
    error_name_str = OBJ(error->error_name);
    error_message_str = OBJ(error->error_msg);
    if (OBJ(error_name_str) == Nst_null() && !catch_exit) {
        Nst_ndec_ref(map);
        map = nullptr;
        goto cleanup;
    }

    error_traceback = Nst_array_new(error->positions->len / 2);
    for (Nst_LLNode *n_start = error->positions->head, *n_end = nullptr;
         n_start != nullptr && n_start->next != nullptr;
         n_start = n_end->next)
    {
        n_end = n_start->next;
        Nst_Obj *pos = make_pos(
            *(Nst_Pos *)n_start->value,
            *(Nst_Pos *)n_end->value);

        if (pos == nullptr) {
            SEQ(error_traceback)->len--;
            continue;
        }
        SEQ(error_traceback)->objs[i++] = pos;
    }

    Nst_error_clear();

    Nst_map_set_str(error_map, "name", error_name_str);
    Nst_map_set_str(error_map, "message", error_message_str);
    Nst_map_set_str(map, "error", error_map);
    Nst_map_set_str(map, "traceback", error_traceback);

cleanup:
    Nst_ndec_ref(error_map);
    Nst_ndec_ref(error_name_str);
    Nst_ndec_ref(error_message_str);
    Nst_ndec_ref(error_traceback);

    return map;
}

Nst_FUNC_SIGN(try_)
{
    Nst_FuncObj *func;
    Nst_SeqObj *func_args;
    bool catch_exit;

    Nst_DEF_EXTRACT("f A y", &func, &func_args, &catch_exit);

    if (func_args->len != func->arg_num) {
        Nst_set_call_error(Nst_sprintf(
            "the function expected %zi arguments but the %s had length %zi",
            func->arg_num, TYPE_NAME(func_args), func_args->len));

        return nullptr;
    }

    Nst_Obj *result = Nst_call_func(
        func,
        (i32)func_args->len,
        func_args->objs);

    if (result != nullptr)
        return success(result);
    else
        return failure(catch_exit);
}

Nst_FUNC_SIGN(_get_err_names_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);

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
