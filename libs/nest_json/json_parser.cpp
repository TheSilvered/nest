#include <cstdlib>
#include "json_lexer.h"
#include "json_parser.h"

bool trailing_commas = false;
static char *file_path; // the text of the position cannot be used
static i32 recursion_level;
static Nst_DynArray *tokens;
static usize idx;

// needed because when debugging on Windows it runs out of stack space quickly
// does not cause any issues when running on Release mode
#if defined(Nst_MSVC) && defined(_DEBUG)
#define MAX_RECURSION_LVL 500
#else
#define MAX_RECURSION_LVL 1500
#endif

#define INC_RECURSION_LVL do {                                                \
        recursion_level++;                                                    \
        if (recursion_level > MAX_RECURSION_LVL) {                            \
            Nst_error_setc_memory(                                            \
                "over 1500 recursive calls, parsing failed");                 \
            return nullptr;                                                   \
        }                                                                     \
    } while (0)
#define DEC_RECURSION_LVL recursion_level--

static Nst_Obj *parse_value();
static Nst_Obj *parse_object();
static Nst_Obj *parse_array();

static Nst_Tok *pop_tok()
{
    return (Nst_Tok *)Nst_da_get(tokens, idx++);
}

static Nst_Tok *peek_tok()
{
    return (Nst_Tok *)Nst_da_get(tokens, idx);
}

Nst_Obj *json_parse(char *path, Nst_DynArray *tok_arr)
{
    tokens = tok_arr;
    file_path = path;
    recursion_level = 0;
    idx = 0;
    Nst_Obj *res = parse_value();
    if (res == nullptr) {
        Nst_da_clear(tokens, (Nst_Destructor)Nst_tok_destroy);
        return nullptr;
    }

    if (idx + 1 != tokens->len) {
        Nst_dec_ref(res);
        Nst_Pos err_pos = Nst_span_end(
            ((Nst_Tok *)Nst_da_get(tokens, idx))->span);
        JSON_SYNTAX_ERROR("unexpected token", file_path, err_pos);
        Nst_da_clear(tokens, (Nst_Destructor)Nst_tok_destroy);
        return nullptr;
    }
    Nst_da_clear(tokens, (Nst_Destructor)Nst_tok_destroy);
    return res;
}

static Nst_Obj *parse_value()
{
    INC_RECURSION_LVL;
    Nst_Tok *tok = pop_tok();

    switch ((JSONTokenType)tok->type) {
    case JSON_VALUE: {
        Nst_Obj *res = Nst_inc_ref(tok->value);
        DEC_RECURSION_LVL;
        return res;
    }
    case JSON_LBRACKET:
        DEC_RECURSION_LVL;
        return parse_array();
    case JSON_LBRACE:
        DEC_RECURSION_LVL;
        return parse_object();
    default:
        JSON_SYNTAX_ERROR(
            "expected value",
            file_path,
            Nst_span_start(tok->span));
        return nullptr;
    }
}

static Nst_Obj *parse_object()
{
    INC_RECURSION_LVL;
    Nst_Obj *map = Nst_map_new();
    Nst_Tok *tok = pop_tok();

    if ((JSONTokenType)tok->type == JSON_RBRACE) {
        DEC_RECURSION_LVL;
        return map;
    }

    while (true) {
        if ((JSONTokenType)tok->type != JSON_VALUE || !Nst_T(tok->value, Str)){
            JSON_SYNTAX_ERROR(
                "expected string",
                file_path,
                Nst_span_start(tok->span));
            Nst_dec_ref(map);
            return nullptr;
        }

        Nst_Obj *key = Nst_inc_ref(tok->value);
        tok = pop_tok();

        if ((JSONTokenType)tok->type != JSON_COLON) {
            JSON_SYNTAX_ERROR(
                "expected colon",
                file_path,
                Nst_span_start(tok->span));
            Nst_dec_ref(map);
            Nst_dec_ref(key);
            return nullptr;
        }

        Nst_Obj *val = parse_value();

        if (val == nullptr) {
            Nst_dec_ref(map);
            Nst_dec_ref(key);
            return nullptr;
        }

        Nst_map_set(map, key, val);
        Nst_dec_ref(key);
        Nst_dec_ref(val);

        tok = pop_tok();
        if ((JSONTokenType)tok->type == JSON_RBRACE) {
            DEC_RECURSION_LVL;
            return map;
        } else if ((JSONTokenType)tok->type == JSON_COMMA) {
            tok = pop_tok();
        } else {
            JSON_SYNTAX_ERROR(
                "expected ',' or '}'",
                file_path,
                Nst_span_start(tok->span));
            Nst_dec_ref(map);
            return nullptr;
        }

        if ((JSONTokenType)tok->type == JSON_RBRACE && trailing_commas) {
            DEC_RECURSION_LVL;
            return map;
        }
    }
}

static Nst_Obj *parse_array()
{
    INC_RECURSION_LVL;
    Nst_Obj *vec = Nst_vector_new(0);
    Nst_Tok *tok = peek_tok();

    if ((JSONTokenType)tok->type == JSON_RBRACKET) {
        pop_tok();
        goto end;
    }

    while (true) {
        Nst_Obj *val = parse_value();

        if (val == nullptr) {
            Nst_dec_ref(vec);
            return nullptr;
        }

        Nst_vector_append(vec, val);
        Nst_dec_ref(val);

        tok = pop_tok();
        if ((JSONTokenType)tok->type == JSON_RBRACKET) {
            goto end;
        } else if ((JSONTokenType)tok->type != JSON_COMMA) {
            JSON_SYNTAX_ERROR(
                "expected ',' or ']'",
                file_path,
                Nst_span_start(tok->span));
            Nst_dec_ref(vec);
            return nullptr;
        }

        tok = peek_tok();
        if ((JSONTokenType)tok->type == JSON_RBRACKET && trailing_commas) {
            pop_tok();
            goto end;
        }
    }

end:
    vec->type = Nst_inc_ref(Nst_type()->Array);
    Nst_dec_ref(Nst_type()->Vector);
    DEC_RECURSION_LVL;
    return vec;
}
