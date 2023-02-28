#include <cstdlib>
#include "json_lexer.h"
#include "json_parser.h"

bool trailing_commas = false;
static i8 *file_path; // the text of the position cannot be used
static i32 recursion_level;

#define INC_RECURSION_LVL \
    do { \
        recursion_level++; \
        if ( recursion_level > 1500 ) \
        { \
            NST_SET_RAW_MEMORY_ERROR("over 1500 recursive calls, parsing failed"); \
            return nullptr; \
        } \
    } while ( 0 )
#define DEC_RECURSION_LVL recursion_level--

static Nst_Obj *parse_value(Nst_LList *tokens, Nst_OpErr *err);
static Nst_Obj *parse_object(Nst_LList *tokens, Nst_OpErr *err);
static Nst_Obj *parse_array(Nst_LList *tokens, Nst_OpErr *err);

Nst_Obj *json_parse(i8 *path, Nst_LList *tokens, Nst_OpErr *err)
{
    file_path = path;
    recursion_level = 0;
    Nst_Obj *res = parse_value(tokens, err);
    if ( res == nullptr )
    {
        nst_llist_destroy(tokens, (nst_llist_destructor)nst_token_destroy);
        return nullptr;
    }

    if ( tokens->size != 1 )
    {
        nst_dec_ref(res);
        Nst_Pos err_pos = NST_TOK(tokens->head)->start;
        JSON_SYNTAX_ERROR("unexpected token", file_path, err_pos);
        nst_llist_destroy(tokens, (nst_llist_destructor)nst_token_destroy);
        return nullptr;
    }
    nst_llist_destroy(tokens, (nst_llist_destructor)nst_token_destroy);
    return res;
}

static Nst_Obj *parse_value(Nst_LList *tokens, Nst_OpErr *err)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));

    switch ( tok->type )
    {
    case JSON_VALUE:
    {
        Nst_Obj *res = nst_inc_ref(tok->value);
        nst_token_destroy(tok);
        return res;
    }
    case JSON_LBRACKET:
        nst_token_destroy(tok);
        return parse_array(tokens, err);
    case JSON_LBRACE:
        nst_token_destroy(tok);
        return parse_object(tokens, err);
    default:
        JSON_SYNTAX_ERROR("expected value", file_path, tok->start);
        nst_token_destroy(tok);
        return nullptr;
    }
    DEC_RECURSION_LVL;
}

static Nst_Obj *parse_object(Nst_LList *tokens, Nst_OpErr *err)
{
    INC_RECURSION_LVL;
    Nst_MapObj *map = MAP(nst_map_new());
    Nst_Tok *tok = NST_TOK(nst_llist_pop(tokens));

    if ( (JSONTokenType)tok->type == JSON_RBRACE )
    {
        nst_token_destroy(tok);
        return OBJ(map);
    }

    while ( true )
    {
        if ( (JSONTokenType)tok->type != JSON_VALUE ||
             tok->value->type != nst_type()->Str )
        {
            JSON_SYNTAX_ERROR("expected string", file_path, tok->start);
            nst_token_destroy(tok);
            nst_dec_ref(map);
            return nullptr;
        }

        Nst_Obj *key = nst_inc_ref(tok->value);
        nst_token_destroy(tok);
        tok = NST_TOK(nst_llist_pop(tokens));

        if ( (JSONTokenType)tok->type != JSON_COLON )
        {
            JSON_SYNTAX_ERROR("expected colon", file_path, tok->start);
            nst_token_destroy(tok);
            nst_dec_ref(map);
            nst_dec_ref(key);
            return nullptr;
        }

        nst_token_destroy(tok);
        Nst_Obj *val = parse_value(tokens, err);

        if ( val == nullptr )
        {
            nst_dec_ref(map);
            nst_dec_ref(key);
            return nullptr;
        }

        nst_map_set(map, key, val);
        nst_dec_ref(key);
        nst_dec_ref(val);

        tok = NST_TOK(nst_llist_pop(tokens));
        if ( (JSONTokenType)tok->type == JSON_RBRACE )
        {
            nst_token_destroy(tok);
            return OBJ(map);
        }
        else if ( (JSONTokenType)tok->type == JSON_COMMA )
        {
            nst_token_destroy(tok);
            tok = NST_TOK(nst_llist_pop(tokens));
        }
        else
        {
            JSON_SYNTAX_ERROR("expected ',' or '}'", file_path, tok->start);
            nst_token_destroy(tok);
            nst_dec_ref(map);
            return nullptr;
        }

        if ( (JSONTokenType)tok->type == JSON_RBRACE && trailing_commas )
        {
            nst_token_destroy(tok);
            return OBJ(map);
        }
    }
    DEC_RECURSION_LVL;
}

static Nst_Obj *parse_array(Nst_LList *tokens, Nst_OpErr *err)
{
    INC_RECURSION_LVL;
    Nst_VectorObj *vec = SEQ(nst_vector_new(0));
    Nst_Tok *tok = NST_TOK(nst_llist_peek_front(tokens));

    if ( (JSONTokenType)tok->type == JSON_RBRACKET )
    {
        nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
        goto end;
    }

    while ( true )
    {
        Nst_Obj *val = parse_value(tokens, err);

        if ( val == nullptr )
        {
            nst_dec_ref(vec);
            return nullptr;
        }

        nst_vector_append(vec, val);
        nst_dec_ref(val);

        tok = NST_TOK(nst_llist_pop(tokens));
        if ( (JSONTokenType)tok->type == JSON_RBRACKET )
        {
            nst_token_destroy(tok);
            goto end;
        }
        else if ( (JSONTokenType)tok->type == JSON_COMMA )
        {
            nst_token_destroy(tok);
        }
        else
        {
            JSON_SYNTAX_ERROR("expected ',' or ']'", file_path, tok->start);
            nst_token_destroy(tok);
            nst_dec_ref(vec);
            return nullptr;
        }

        tok = NST_TOK(nst_llist_peek_front(tokens));
        if ( (JSONTokenType)tok->type == JSON_RBRACKET && trailing_commas )
        {
            nst_token_destroy(NST_TOK(nst_llist_pop(tokens)));
            goto end;
        }
    }

end:
    vec->type = TYPE(nst_inc_ref(nst_type()->Array));
    nst_dec_ref(nst_type()->Vector);
    if ( vec->len < vec->size && vec->len != 0 )
    {
        Nst_Obj **new_objs = (Nst_Obj **)realloc(
            vec->objs,
            vec->len * sizeof(Nst_Obj *));
        if ( new_objs != nullptr )
        {
            vec->objs = new_objs;
            vec->size = vec->len;
        }
    }
    return OBJ(vec);
    DEC_RECURSION_LVL;
}
