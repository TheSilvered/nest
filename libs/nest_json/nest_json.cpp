#include <cstdlib>
#include "nest_json.h"
#include "json_lexer.h"
#include "json_parser.h"
#include "json_dumper.h"

#define FUNC_COUNT 6

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(load_s_,      1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(load_f_,      1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dump_s_,      2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dump_f_,      3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(set_options_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_options_, 0);

#if __LINE__ - FUNC_COUNT != 18
#error
#endif

    lib_init_ = true;
    return true;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

NST_FUNC_SIGN(load_s_)
{
    Nst_StrObj *str;
    NST_DEF_EXTRACT("s", &str);

    Nst_LList *tokens = json_tokenize(
        (i8 *)"<Str>",
        str->value, str->len,
        false, err);
    if ( tokens == nullptr )
    {
        return nullptr;
    }

    Nst_Obj *value = json_parse((i8 *)"<Str>", tokens, err);
    return value;
}

NST_FUNC_SIGN(load_f_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);

    Nst_IOFile f = fopen(path->value, "rb");
    if ( f == nullptr )
    {
        NST_SET_RAW_VALUE_ERROR("file not found");
        return nullptr;
    }

    fseek(f, 0, SEEK_END);
    usize buf_size = (usize)ftell(f);
    fseek(f, 0, SEEK_SET);

    i8 *buf = (i8 *)nst_malloc((buf_size + 1), sizeof(i8));
    if ( buf == nullptr )
    {
        fclose(f);
        NST_FAILED_ALLOCATION;
        return nullptr;
    }

    usize len = fread(buf, sizeof(i8), buf_size, f);
    fclose(f);
    buf[len] = 0;
    Nst_LList *tokens = json_tokenize(path->value, buf, len, true, err);
    if ( tokens == nullptr )
    {
        return nullptr;
    }

    Nst_Obj *value = json_parse(path->value, tokens, err);
    return value;
}

NST_FUNC_SIGN(dump_s_)
{
    Nst_Obj *obj;
    Nst_Obj *indent_obj;

    NST_DEF_EXTRACT("o?i", &obj, &indent_obj);
    Nst_Int indent = NST_DEF_VAL(indent_obj, AS_INT(indent_obj), 0);

    return json_dump(obj, (i32)indent, err);
}

NST_FUNC_SIGN(dump_f_)
{
    Nst_StrObj *path;
    Nst_Obj *obj;
    Nst_Obj *indent_obj;

    NST_DEF_EXTRACT("so?i", &path, &obj, &indent_obj);
    Nst_Int indent = NST_DEF_VAL(indent_obj, AS_INT(indent_obj), 0);

    Nst_IOFile f = fopen(path->value, "wb");
    if ( f == nullptr )
    {
        NST_SET_RAW_VALUE_ERROR("file not found");
        return nullptr;
    }

    Nst_Obj *res = json_dump(obj, (i32)indent, err);
    if ( res == nullptr )
    {
        fclose(f);
        return nullptr;
    }
    fwrite(STR(res)->value, sizeof(i8), STR(res)->len, f);
    fclose(f);
    nst_dec_ref(res);
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(set_options_)
{
    Nst_Int options;
    NST_DEF_EXTRACT("i", &options);

    comments        = bool(options & 0b01);
    trailing_commas = bool(options & 0b10);

    NST_RETURN_NULL;
}

NST_FUNC_SIGN(get_options_)
{
    Nst_Int val = 0;
    val |= comments        ? 0b01 : 0;
    val |= trailing_commas ? 0b10 : 0;
    return nst_int_new(val);
}
