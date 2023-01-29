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

    LList *tokens = tokenize((char *)"<Str>", str->value, str->len, false, err);
    if ( tokens == nullptr )
    {
        return nullptr;
    }

    for ( ITER_LLIST(n, tokens) )
    {
        Nst_LexerToken *tok = TOK(n->value);
        if ( tok == nullptr )
        {
            printf("- end -\n");
            continue;
        }
        switch ( tok->type )
        {
        case JSON_LBRACKET: printf("JSON_LBRACKET\n"); break;
        case JSON_RBRACKET: printf("JSON_RBRACKET\n"); break;
        case JSON_LBRACE:   printf("JSON_LBRACE\n");   break;
        case JSON_RBRACE:   printf("JSON_RBRACE\n");   break;
        case JSON_COMMA:    printf("JSON_COMMA\n");    break;
        case JSON_COLON:    printf("JSON_COLON\n");    break;
        case JSON_VALUE:
            printf("JSON_VALUE - ");
            if ( tok->value == nst_c.b_true )
            {
                printf("true\n");
            }
            else if ( tok->value == nst_c.b_false )
            {
                printf("false\n");
            }
            else if ( tok->value == nst_c.null )
            {
                printf("null\n");
            }
            else if ( tok->value->type == nst_t.Str )
            {
                printf("%s\n", STR(tok->value)->value);
            }
            else if ( tok->value->type == nst_t.Int )
            {
                printf("%lli\n", AS_INT(tok->value));
            }
            else
            {
                printf("%.14lg\n", AS_REAL(tok->value));
            }
            break;
        }
    }

    NST_RETURN_NULL;
}

NST_FUNC_SIGN(parse_f_)
{
    NST_RETURN_NULL;
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

    comments        = bool(options & 0b001);
    trailing_commas = bool(options & 0b010);
    split_strings   = bool(options & 0b100);

    NST_RETURN_NULL;
}
