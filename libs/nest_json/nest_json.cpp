#include <cstdlib>
#include "nest_json.h"
#include "json_lexer.h"
#include "json_parser.h"
#include "json_dumper.h"

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(load_s_,        1),
    Nst_FUNCDECLR(load_f_,        2),
    Nst_FUNCDECLR(dump_s_,        2),
    Nst_FUNCDECLR(dump_f_,        4),
    Nst_FUNCDECLR(set_option_,    2),
    Nst_FUNCDECLR(get_option_,    1),
    Nst_FUNCDECLR(clear_options_, 0),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

Nst_Obj *NstC load_s_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *str;
    if (!Nst_extract_args("s", arg_num, args, &str))
        return nullptr;

    Nst_LList *tokens = json_tokenize(
        (i8 *)"<Str>",
        str->value, str->len,
        true, Nst_CP_EXT_UTF8);
    if (tokens == nullptr)
        return nullptr;

    Nst_Obj *value = json_parse((i8 *)"<Str>", tokens);
    return value;
}

Nst_Obj *NstC load_f_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    Nst_StrObj *encoding_obj;
    if (!Nst_extract_args("s ?s", arg_num, args, &path, &encoding_obj))
        return nullptr;

    Nst_CPID encoding = Nst_DEF_VAL(
        encoding_obj,
        Nst_encoding_from_name(encoding_obj->value),
        Nst_CP_UNKNOWN);
    encoding = Nst_single_byte_cp(encoding);

    FILE *f = Nst_fopen_unicode(path->value, "rb");

    if (f == nullptr) {
        if (!Nst_error_occurred())
            Nst_set_value_errorf("file '%.4096s' not found", path->value);
        return nullptr;
    }

    fseek(f, 0, SEEK_END);
    usize buf_size = (usize)ftell(f);
    fseek(f, 0, SEEK_SET);

    i8 *buf = Nst_malloc_c(buf_size + 1, i8);
    if (buf == nullptr) {
        fclose(f);
        return nullptr;
    }

    usize len = fread(buf, sizeof(i8), buf_size, f);
    fclose(f);
    buf[len] = 0;
    Nst_LList *tokens = json_tokenize(path->value, buf, len, false, encoding);
    if (tokens == nullptr)
        return nullptr;

    Nst_Obj *value = json_parse(path->value, tokens);
    return value;
}

Nst_Obj *NstC dump_s_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *obj;
    Nst_Obj *indent_obj;

    if (!Nst_extract_args("o ?i", arg_num, args, &obj, &indent_obj))
        return nullptr;

    i64 indent = Nst_DEF_VAL(indent_obj, AS_INT(indent_obj), 0);
    return json_dump(obj, (i32)indent);
}

Nst_Obj *NstC dump_f_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    Nst_Obj *obj;
    Nst_Obj *indent_obj;
    Nst_StrObj *encoding_obj;

    if (!Nst_extract_args(
            "s o ?i ?s",
            arg_num, args,
            &path, &obj, &indent_obj, &encoding_obj))
    {
        return nullptr;
    }
    i64 indent = Nst_DEF_VAL(indent_obj, AS_INT(indent_obj), 0);

    Nst_CPID encoding = Nst_DEF_VAL(
        encoding_obj,
        Nst_encoding_from_name(encoding_obj->value),
        Nst_CP_EXT_UTF8);
    encoding = Nst_single_byte_cp(encoding);

    FILE *f = Nst_fopen_unicode(path->value, "wb");

    if (f == nullptr) {
        if (!Nst_error_occurred()) {
            Nst_set_value_errorf(
                "could not open the file '%.4096s'",
                path->value);
        }
        return nullptr;
    }

    Nst_Obj *res = json_dump(obj, (i32)indent);
    if (res == nullptr) {
        fclose(f);
        return nullptr;
    }

    i8 *encoded_str;
    usize encoded_str_len;

    bool result = Nst_translate_cp(
        Nst_cp(Nst_CP_EXT_UTF8),
        Nst_cp(encoding),
        (void *)STR(res)->value,
        STR(res)->len,
        (void **)&encoded_str,
        &encoded_str_len);
    Nst_dec_ref(res);

    if (!result) {
        fclose(f);
        return nullptr;
    }

    fwrite(encoded_str, sizeof(i8), encoded_str_len, f);
    fclose(f);
    Nst_free(encoded_str);
    Nst_RETURN_NULL;
}

Nst_Obj *NstC set_option_(usize arg_num, Nst_Obj **args)
{
    i64 option;
    Nst_Obj *value_obj;

    if (!Nst_extract_args("i o", arg_num, args, &option, &value_obj))
        return nullptr;

    bool value;
    if (value_obj == Nst_null()) {
        switch (JSONOptions(option)) {
        case JSON_OPT_COMMENTS:
            comments = comments_default;
            break;
        case JSON_OPT_TRAILING_COMMAS:
            trailing_commas = trailing_commas_default;
            break;
        case JSON_OPT_NAN_AND_INF:
            nan_and_inf = nan_and_inf_default;
            break;
        default:
            Nst_set_value_errorf("option %lli does not exist", option);
            return nullptr;
        }
        Nst_RETURN_NULL;
    }
    value = Nst_obj_to_bool(value_obj);

    switch (JSONOptions(option)) {
    case JSON_OPT_COMMENTS:
        comments = value;
        break;
    case JSON_OPT_TRAILING_COMMAS:
        trailing_commas = value;
        break;
    case JSON_OPT_NAN_AND_INF:
        nan_and_inf = value;
        break;
    default:
        Nst_set_value_errorf("option %lli does not exist", option);
        return nullptr;
    }

    Nst_RETURN_NULL;
}

Nst_Obj *NstC get_option_(usize arg_num, Nst_Obj **args)
{
    i64 option;
    if (!Nst_extract_args("i", arg_num, args, &option))
        return nullptr;

    switch (JSONOptions(option)) {
    case JSON_OPT_COMMENTS:
        Nst_RETURN_BOOL(comments);
    case JSON_OPT_TRAILING_COMMAS:
        Nst_RETURN_BOOL(trailing_commas);
    case JSON_OPT_NAN_AND_INF:
        Nst_RETURN_BOOL(nan_and_inf);
    default:
        Nst_set_value_errorf("option %lli does not exist", option);
        return nullptr;
    }
}

Nst_Obj *NstC clear_options_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);

    comments = comments_default;
    trailing_commas = trailing_commas_default;
    nan_and_inf = nan_and_inf_default;
    Nst_RETURN_NULL;
}
