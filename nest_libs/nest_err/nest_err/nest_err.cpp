#include <cstdlib>
#include "nest_err.h"

#define FUNC_COUNT 2 // Set this to the number of functions in your module

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(try_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(raise_, 2);

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

Nst_Obj *make_pos(Nst_Pos pos1, Nst_Pos pos2)
{
    if ( pos1.filename == nullptr )
        return nullptr;

    Nst_Obj *map = nst_new_map();

    Nst_Obj *arr1 = nst_new_array(2);
    Nst_Obj *arr2 = nst_new_array(2);
    Nst_Obj *file_str = nst_new_string_raw(pos1.filename, false);

    AS_SEQ(arr1)->objs[0] = nst_new_int(pos1.line);
    AS_SEQ(arr1)->objs[1] = nst_new_int(pos1.col);
    AS_SEQ(arr2)->objs[0] = nst_new_int(pos2.line);
    AS_SEQ(arr2)->objs[1] = nst_new_int(pos2.col);

    nst_map_set_str(map, "file", file_str);
    nst_map_set_str(map, "start", arr1);
    nst_map_set_str(map, "end", arr2);

    nst_dec_ref(file_str);
    nst_dec_ref(arr1);
    nst_dec_ref(arr2);

    return map;
}

Nst_Obj *success(Nst_Obj *val)
{
    Nst_Obj *map = nst_new_map();
    nst_map_set_str(map, "value", val);
    nst_map_set_str(map, "error", nst_null);
    nst_map_set_str(map, "traceback", nst_null);
    nst_dec_ref(val);

    return map;
}

Nst_Obj *failure(Nst_OpErr *err)
{
    Nst_Obj *map = nst_new_map();
    Nst_Obj *error_map = nst_new_map();
    Nst_Obj *error_name_str;
    Nst_Obj *error_message_str;
    Nst_Obj *error_pos;
    Nst_Obj *error_traceback;
    nst_map_set_str(map, "value", nst_null);

    if ( nst_state.traceback->error.occurred )
    {
        Nst_Error error = nst_state.traceback->error;
        error_name_str = nst_new_string_raw(error.name, false);
        error_message_str = nst_new_string_raw(error.message, false);
        error_pos = make_pos(error.start, error.end);

        error_traceback = nst_new_array(nst_state.traceback->positions->size / 2);

        LList *positions = nst_state.traceback->positions;
        Nst_Int skipped = 0;
        LLNode *n1 = positions->head;
        LLNode *n2 = n1 == nullptr ? n1 : n1->next;
        for ( size_t i = 0; n1 != NULL; i++ )
        {
            Nst_Obj *pos = make_pos(*(Nst_Pos *)n1->value,
                                    *(Nst_Pos *)n2->value);

            n1 = n2->next;
            n2 = n1 == nullptr ? n1 : n1->next;

            if ( pos == nullptr )
            {
                AS_SEQ(error_traceback)->len--;
                skipped++;
                continue;
            }

            AS_SEQ(error_traceback)->objs[i - skipped] = pos;
        }

        LList_empty(positions, free);
        nst_state.traceback->error.occurred = false;
    }
    else
    {
        Nst_Obj *error_map = nst_new_map();
        error_name_str = nst_new_string_raw(err->name,    false);
        error_message_str = nst_new_string_raw(err->message, false);
        error_pos = nst_inc_ref(nst_null);
        error_traceback = nst_inc_ref(nst_null);
    }

    nst_map_set_str(error_map, "name", error_name_str);
    nst_map_set_str(error_map, "message", error_message_str);
    nst_map_set_str(error_map, "pos", error_pos);
    nst_map_set_str(map, "error", error_map);
    nst_map_set_str(map, "traceback", error_traceback);

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

    if ( !nst_extract_arg_values("fA", arg_num, args, err, &func, &func_args) )
        return nullptr;

    if ( func_args->len != func->arg_num )
    {
        NST_SET_CALL_ERROR(_nst_format_error(
            "the function expected %zi arguments but the %s had length %zi",
            "usu",
            func->arg_num, TYPE_NAME(func_args), func_args->len));

        return nullptr;
    }

    Nst_Obj *result = nst_call_func(func, func_args->objs, err);

    if ( result != NULL )
        return success(result);
    else
        return failure(err);
}


NST_FUNC_SIGN(raise_)
{
    Nst_StrObj *name;
    Nst_StrObj *message;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &name, &message) )
        return nullptr;

    char *err_name = new char[name->len + 1];
    char *err_message = new char[message->len + 1];

    memcpy(err_name, name->value, name->len + 1);
    memcpy(err_message, message->value, message->len + 1);

    NST_SET_ERROR(
        err_name,
        err_message
    );

    return nullptr;
}
