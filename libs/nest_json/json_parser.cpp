#include <cstdlib>
#include "json_lexer.h"
#include "json_parser.h"

bool trailing_commas = false;
static i8 *file_path; // the text of the position cannot be used
static i32 recursion_level;

#define INC_RECURSION_LVL do {                                                \
        recursion_level++;                                                    \
        if (recursion_level > 1500) {                                         \
            Nst_set_memory_error_c(                                           \
                "over 1500 recursive calls, parsing failed");                 \
            return nullptr;                                                   \
        }                                                                     \
    } while (0)
#define DEC_RECURSION_LVL recursion_level--

static Nst_Obj *parse_value(Nst_LList *tokens);
static Nst_Obj *parse_object(Nst_LList *tokens);
static Nst_Obj *parse_array(Nst_LList *tokens);

Nst_Obj *json_parse(i8 *path, Nst_LList *tokens)
{
    file_path = path;
    recursion_level = 0;
    Nst_Obj *res = parse_value(tokens);
    if (res == nullptr) {
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
        return nullptr;
    }

    if (tokens->len != 1) {
        Nst_dec_ref(res);
        Nst_Pos err_pos = Nst_TOK(tokens->head)->start;
        JSON_SYNTAX_ERROR("unexpected token", file_path, err_pos);
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
        return nullptr;
    }
    Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
    return res;
}

static Nst_Obj *parse_value(Nst_LList *tokens)
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));

    switch (tok->type) {
    case JSON_VALUE: {
        Nst_Obj *res = Nst_inc_ref(tok->value);
        Nst_token_destroy(tok);
        return res;
    }
    case JSON_LBRACKET:
        Nst_token_destroy(tok);
        return parse_array(tokens);
    case JSON_LBRACE:
        Nst_token_destroy(tok);
        return parse_object(tokens);
    default:
        JSON_SYNTAX_ERROR("expected value", file_path, tok->start);
        Nst_token_destroy(tok);
        return nullptr;
    }
    DEC_RECURSION_LVL;
}

static Nst_Obj *parse_object(Nst_LList *tokens)
{
    INC_RECURSION_LVL;
    Nst_MapObj *map = MAP(Nst_map_new());
    Nst_Tok *tok = Nst_TOK(Nst_llist_pop(tokens));

    if ((JSONTokenType)tok->type == JSON_RBRACE) {
        Nst_token_destroy(tok);
        return OBJ(map);
    }

    while (true) {
        if ((JSONTokenType)tok->type != JSON_VALUE || !Nst_T(tok->value, Str)){
            JSON_SYNTAX_ERROR("expected string", file_path, tok->start);
            Nst_token_destroy(tok);
            Nst_dec_ref(map);
            return nullptr;
        }

        Nst_Obj *key = Nst_inc_ref(tok->value);
        Nst_token_destroy(tok);
        tok = Nst_TOK(Nst_llist_pop(tokens));

        if ((JSONTokenType)tok->type != JSON_COLON) {
            JSON_SYNTAX_ERROR("expected colon", file_path, tok->start);
            Nst_token_destroy(tok);
            Nst_dec_ref(map);
            Nst_dec_ref(key);
            return nullptr;
        }

        Nst_token_destroy(tok);
        Nst_Obj *val = parse_value(tokens);

        if (val == nullptr) {
            Nst_dec_ref(map);
            Nst_dec_ref(key);
            return nullptr;
        }

        Nst_map_set(map, key, val);
        Nst_dec_ref(key);
        Nst_dec_ref(val);

        tok = Nst_TOK(Nst_llist_pop(tokens));
        if ((JSONTokenType)tok->type == JSON_RBRACE) {
            Nst_token_destroy(tok);
            return OBJ(map);
        } else if ((JSONTokenType)tok->type == JSON_COMMA) {
            Nst_token_destroy(tok);
            tok = Nst_TOK(Nst_llist_pop(tokens));
        } else {
            JSON_SYNTAX_ERROR("expected ',' or '}'", file_path, tok->start);
            Nst_token_destroy(tok);
            Nst_dec_ref(map);
            return nullptr;
        }

        if ((JSONTokenType)tok->type == JSON_RBRACE && trailing_commas) {
            Nst_token_destroy(tok);
            return OBJ(map);
        }
    }
    DEC_RECURSION_LVL;
}

static Nst_Obj *parse_array(Nst_LList *tokens)
{
    INC_RECURSION_LVL;
    Nst_VectorObj *vec = SEQ(Nst_vector_new(0));
    Nst_Tok *tok = Nst_TOK(Nst_llist_peek_front(tokens));

    if ((JSONTokenType)tok->type == JSON_RBRACKET) {
        Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
        goto end;
    }

    while (true) {
        Nst_Obj *val = parse_value(tokens);

        if (val == nullptr) {
            Nst_dec_ref(vec);
            return nullptr;
        }

        Nst_vector_append(vec, val);
        Nst_dec_ref(val);

        tok = Nst_TOK(Nst_llist_pop(tokens));
        if ((JSONTokenType)tok->type == JSON_RBRACKET) {
            Nst_token_destroy(tok);
            goto end;
        } else if ((JSONTokenType)tok->type == JSON_COMMA)
            Nst_token_destroy(tok);
        else {
            JSON_SYNTAX_ERROR("expected ',' or ']'", file_path, tok->start);
            Nst_token_destroy(tok);
            Nst_dec_ref(vec);
            return nullptr;
        }

        tok = Nst_TOK(Nst_llist_peek_front(tokens));
        if ((JSONTokenType)tok->type == JSON_RBRACKET && trailing_commas) {
            Nst_token_destroy(Nst_TOK(Nst_llist_pop(tokens)));
            goto end;
        }
    }

end:
    vec->type = TYPE(Nst_inc_ref(Nst_type()->Array));
    Nst_dec_ref(Nst_type()->Vector);
    if (vec->len < vec->cap && vec->len != 0) {
        Nst_Obj **new_objs = Nst_realloc_c(
            vec->objs,
            vec->len,
            Nst_Obj *,
            0);
        if (new_objs != nullptr) {
            vec->objs = new_objs;
            vec->cap = vec->len;
        }
    }
    return OBJ(vec);
    DEC_RECURSION_LVL;
}
