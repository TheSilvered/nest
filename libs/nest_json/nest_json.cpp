#include "nest_json.h"
#include "json_lexer.h"
#include "json_parser.h"

#define FUNC_COUNT 5

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(parse_s_,     1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(parse_f_,     1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dump_s_,      1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dump_f_,      2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(set_options_, 1);
    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

NST_FUNC_SIGN(parse_s_)
{
    Nst_StrObj *str;
    NST_DEF_EXTRACT("s", &str);

    LList *tokens = json_tokenize(
        (char *)"<Str>",
        str->value, str->len,
        false, err);
    if ( tokens == nullptr )
    {
        return nullptr;
    }

    Nst_Obj *value = json_parse((char *)"<Str>", tokens, err);
    return value;
}

NST_FUNC_SIGN(parse_f_)
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
    size_t buf_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = new char[buf_size + 1];
    size_t len = fread(buf, sizeof(char), buf_size, f);
    buf[len] = 0;
    LList *tokens = json_tokenize(path->value, buf, len, true, err);
    if ( tokens == nullptr )
    {
        return nullptr;
    }

    Nst_Obj *value = json_parse(path->value, tokens, err);
    return value;
}

NST_FUNC_SIGN(dump_s_)
{
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(dump_f_)
{
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
