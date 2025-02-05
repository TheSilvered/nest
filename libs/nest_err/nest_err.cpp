#include <cstdlib>
#include "nest_err.h"

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(try_, 4),
    Nst_CONSTDECLR(SYNTAX_ERROR_),
    Nst_CONSTDECLR(VALUE_ERROR_),
    Nst_CONSTDECLR(TYPE_ERROR_),
    Nst_CONSTDECLR(CALL_ERROR_),
    Nst_CONSTDECLR(MEMORY_ERROR_),
    Nst_CONSTDECLR(MATH_ERROR_),
    Nst_CONSTDECLR(IMPORT_ERROR_),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

Nst_Obj *make_pos(Nst_Pos start, Nst_Pos end)
{
    Nst_Obj *map = Nst_map_new();

    Nst_Obj *file_str = Nst_str_new_c_raw(start.text->path, false);
    Nst_Obj *arr_start = Nst_array_create_c("ii", start.line, start.col);
    Nst_Obj *arr_end = Nst_array_create_c("ii", end.line, end.col);

    Nst_map_set_str(map, "file", file_str);
    Nst_map_set_str(map, "start", arr_start);
    Nst_map_set_str(map, "end", arr_end);

    Nst_dec_ref(file_str);
    Nst_dec_ref(arr_start);
    Nst_dec_ref(arr_end);

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

Nst_Obj *failure(bool catch_exit, bool catch_interrupt)
{
    Nst_Obj *map = Nst_map_new();
    Nst_Obj *error_map = Nst_map_new();
    Nst_Obj *error_name_str = nullptr;
    Nst_Obj *error_message_str = nullptr;
    Nst_Obj *error_traceback = nullptr;
    Nst_Traceback *error = Nst_error_get();
    usize i = 0;
    usize pos_count = 0;
    Nst_Obj **tb_objs = nullptr;

    if (!error->error_occurred) {
        Nst_set_value_error_c("invalid error state");
        return nullptr;
    }

    Nst_map_set_str(map, "value", Nst_null());
    error_name_str = Nst_inc_ref(error->error_name);
    error_message_str = Nst_inc_ref(error->error_msg);
    if (((OBJ(error_name_str) == Nst_null() && !catch_exit))
        || (OBJ(error_message_str) == Nst_null() && !catch_interrupt))
    {
        Nst_ndec_ref(map);
        map = nullptr;
        goto cleanup;
    }

    pos_count = error->positions->len / 2;
    tb_objs = Nst_malloc_c(pos_count, Nst_Obj *);
    for (Nst_LLNode *n_start = error->positions->head, *n_end = nullptr;
         n_start != nullptr && n_start->next != nullptr;
         n_start = n_end->next)
    {
        n_end = n_start->next;
        if (((Nst_Pos *)n_start->value)->text == nullptr) {
            pos_count--;
            continue;
        }

        Nst_Obj *pos = make_pos(
            *(Nst_Pos *)n_start->value,
            *(Nst_Pos *)n_end->value);

        tb_objs[i++] = pos;
    }

    error_traceback = Nst_array_from_objsn(pos_count, tb_objs);
    Nst_free(tb_objs);

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

Nst_Obj *NstC try_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *func;
    Nst_Obj *func_args;
    bool catch_exit;
    bool catch_interrupt;

    if (!Nst_extract_args(
            "f ?A y y",
            arg_num, args,
            &func, &func_args, &catch_exit, &catch_interrupt))
    {
        return nullptr;
    }

    i64 func_args_len;
    Nst_Obj **objs;
    if (OBJ(func_args) == Nst_null()) {
        func_args_len = 0;
        objs = nullptr;
    } else {
        func_args_len = Nst_seq_len(func_args);
        objs = _Nst_seq_objs(func_args);
    }
    usize func_arg_num = Nst_func_arg_num(func);

    if (func_args_len > (i64)func_arg_num) {
        Nst_set_call_error(
            _Nst_EM_WRONG_ARG_NUM_FMT(func_arg_num, func_args_len));
        return nullptr;
    }

    Nst_Obj *result = Nst_func_call(
        func,
        func_args_len,
        objs);

    if (result != nullptr)
        return success(result);
    else
        return failure(catch_exit, catch_interrupt);
}

Nst_Obj *NstC _get_err_names_()
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

Nst_Obj *NstC SYNTAX_ERROR_()
{
    return Nst_inc_ref(Nst_str()->e_SyntaxError);
}

Nst_Obj *NstC VALUE_ERROR_()
{
    return Nst_inc_ref(Nst_str()->e_ValueError);
}

Nst_Obj *NstC TYPE_ERROR_()
{
    return Nst_inc_ref(Nst_str()->e_TypeError);
}

Nst_Obj *NstC CALL_ERROR_()
{
    return Nst_inc_ref(Nst_str()->e_CallError);
}

Nst_Obj *NstC MEMORY_ERROR_()
{
    return Nst_inc_ref(Nst_str()->e_MemoryError);
}

Nst_Obj *NstC MATH_ERROR_()
{
    return Nst_inc_ref(Nst_str()->e_MathError);
}

Nst_Obj *NstC IMPORT_ERROR_()
{
    return Nst_inc_ref(Nst_str()->e_ImportError);
}
