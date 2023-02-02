#include "json_lexer.h"
#include "json_parser.h"

bool trailing_commas = false;
static char *file_path; // the text of the position cannot be used

static Nst_Obj *parse_value(LList *tokens, Nst_OpErr *err);
static Nst_Obj *parse_object(LList *tokens, Nst_OpErr *err);
static Nst_Obj *parse_array(LList *tokens, Nst_OpErr *err);

Nst_Obj *json_parse(char *path, LList *tokens, Nst_OpErr *err)
{
    file_path = path;

    Nst_Obj *res = parse_value(tokens, err);
    if ( res == nullptr )
    {
        return nullptr;
    }

    if ( tokens->size != 1 )
    {
        nst_dec_ref(res);
        Nst_Pos err_pos = TOK(tokens->head)->start;
        JSON_SYNTAX_ERROR("unexpected token", file_path, err_pos);
        return nullptr;
    }
    return res;
}

static Nst_Obj *parse_value(LList *tokens, Nst_OpErr *err)
{
    Nst_LexerToken *tok = TOK(LList_pop(tokens));

    switch ( tok->type )
    {
    case JSON_VALUE:
    {
        Nst_Obj *res = nst_inc_ref(tok->value);
        nst_destroy_token(tok);
        return res;
    }
    case JSON_LBRACKET:
        nst_destroy_token(tok);
        return parse_array(tokens, err);
    case JSON_LBRACE:
        nst_destroy_token(tok);
        return parse_object(tokens, err);
    default:
        JSON_SYNTAX_ERROR("unexpected token", file_path, tok->start);
        nst_destroy_token(tok);
        return nullptr;
    }
}

static Nst_Obj *parse_object(LList *tokens, Nst_OpErr *err)
{
    Nst_MapObj *map = MAP(nst_new_map());
    Nst_LexerToken *tok = TOK(LList_pop(tokens));

    if ( tok->type == JSON_RBRACE )
    {
        nst_destroy_token(tok);
        return OBJ(map);
    }

    while ( true )
    {
        if ( tok->type != JSON_VALUE || tok->value->type != nst_t.Str )
        {
            JSON_SYNTAX_ERROR("expected string", file_path, tok->start);
            nst_destroy_token(tok);
            nst_dec_ref(map);
            return nullptr;
        }

        Nst_Obj *key = nst_inc_ref(tok->value);
        nst_destroy_token(tok);
        tok = TOK(LList_pop(tokens));

        if ( tok->type != JSON_COLON )
        {
            JSON_SYNTAX_ERROR("expected colon", file_path, tok->start);
            nst_destroy_token(tok);
            nst_dec_ref(map);
            nst_dec_ref(key);
            return nullptr;
        }

        nst_destroy_token(tok);
        Nst_Obj *val = parse_value(tokens, err);

        if ( val == nullptr )
        {
            nst_dec_ref(map);
            nst_dec_ref(key);
            return nullptr;
        }

        nst_map_set(map, key, val);
        nst_dec_ref(key);

        tok = TOK(LList_pop(tokens));
        if ( tok->type == JSON_RBRACE )
        {
            nst_destroy_token(tok);
            return OBJ(map);
        }
        else if ( tok->type == JSON_COMMA )
        {
            nst_destroy_token(tok);
            tok = TOK(LList_pop(tokens));
        }
        else
        {
            JSON_SYNTAX_ERROR("expected ',' or '}'", file_path, tok->start);
            nst_destroy_token(tok);
            nst_dec_ref(map);
            return nullptr;
        }

        if ( tok->type == JSON_RBRACE && trailing_commas )
        {
            nst_destroy_token(tok);
            return OBJ(map);
        }
    }
}

static Nst_Obj *parse_array(LList *tokens, Nst_OpErr *err)
{
    Nst_VectorObj *vec = SEQ(nst_new_vector(0));
    Nst_LexerToken *tok;

    if ( TOK(LList_peek_front(tokens))->type == JSON_RBRACKET )
    {
        nst_destroy_token(TOK(LList_pop(tokens)));
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

        nst_append_value_vector(vec, val);
        nst_dec_ref(val);

        tok = TOK(LList_pop(tokens));
        if ( tok->type == JSON_RBRACKET )
        {
            nst_destroy_token(tok);
            goto end;
        }
        else if ( tok->type == JSON_COMMA )
        {
            nst_destroy_token(tok);
        }
        else
        {
            JSON_SYNTAX_ERROR("expected ',' or ']'", file_path, tok->start);
            nst_destroy_token(tok);
            nst_dec_ref(vec);
            return nullptr;
        }

        if ( TOK(LList_peek_front(tokens))->type == JSON_RBRACKET )
        {
            nst_destroy_token(TOK(LList_pop(tokens)));
            goto end;
        }
    }

end:
    vec->type = TYPE(nst_inc_ref(nst_t.Array));
    nst_dec_ref(nst_t.Vector);
    if ( vec->len < vec->size )
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
}
