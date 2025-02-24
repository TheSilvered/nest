#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include "json_lexer.h"

#define FILE_INFO ", file \"%s\", line %lli, column %lli"

#define SET_INVALID_ESCAPE_ERROR                                              \
    Nst_error_setf_syntax(                                                    \
        "JSON: invalid string escape" FILE_INFO,                              \
        state.pos.text->path,                                                 \
        (i64)state.pos.line,                                                  \
        (i64)state.pos.col)

#define SET_INVALID_VALUE_ERROR                                               \
    Nst_error_setf_syntax(                                                    \
        "JSON: invalid value" FILE_INFO,                                      \
        state.pos.text->path,                                                 \
        (i64)state.pos.line,                                                  \
        (i64)state.pos.col)

#define IS_HEX(ch) ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))
#define HEX_TO_INT(ch) (ch <= '9' ? ch - '0' : ch - 'a' + 10)

#define tok_new_noval(start, end, type) \
    Nst_tok_new_noval(start, end, (Nst_TokType)type)
#define tok_new_noend(start, type) Nst_tok_new_noend(start, (Nst_TokType)type)
#define tok_new_value(start, end, type, value) \
    Nst_tok_new_value(start, end, (Nst_TokType)type, value)

bool comments = false;
bool nan_and_inf = false;

typedef struct _LexerState {
    Nst_Pos pos;
    usize idx;
    usize len;
    i8 *path;
    i8 ch;
} LexerState;

LexerState state;

static void advance()
{
    state.idx++;
    state.pos.col++;
    if (state.idx >= state.len) {
        state.ch = '\0';
        return;
    }

    state.ch = state.pos.text->text[state.idx];
    if (state.ch == '\n') {
        state.pos.line++;
        state.pos.col = 0;
    }
}

static void go_back()
{
    state.idx--;
    state.pos.col--;
    state.ch = state.pos.text->text[state.idx];
}

static Nst_Tok *parse_json_str();
static Nst_Tok *parse_json_num();
static Nst_Tok *parse_json_val();
static bool ignore_comment();
static bool check_ident(const i8 *name);

Nst_LList *json_tokenize(i8 *path, i8 *text, usize text_len,
                         bool readonly_text, Nst_EncodingID encoding)
{
    Nst_SourceText src_text = {
        .allocated = false,
        .text = text,
        .path = path,
        .lines = nullptr,
        .text_len = text_len,
        .lines_len = 0
    };

    if (readonly_text) {
        i8 *text_copy = Nst_malloc_c(text_len, i8);
        if (text_copy == nullptr)
            return nullptr;
        memcpy(text_copy, text, text_len);
        src_text.text = text_copy;
    }

    bool result = Nst_normalize_encoding(&src_text, encoding);
    Nst_add_lines(&src_text);
    if (!result || Nst_error_occurred()) {
        Nst_free(src_text.text);
        Nst_free(src_text.lines);
        return nullptr;
    }

    Nst_LList *tokens = Nst_llist_new();
    Nst_Tok *tok = nullptr;

    if (text_len == 0)
        goto end;

    state.pos.text = &src_text;
    state.pos.col = 1;
    state.pos.line = 1;
    state.idx = 0;
    state.len = src_text.text_len;
    state.path = path;
    state.ch = *src_text.text;

    while (state.idx < state.len) {
        if (state.ch == ' ' || state.ch == '\n' ||
            state.ch == '\r' || state.ch == '\t')
        {
            advance();
            continue;
        } else if (state.ch == '[')
            tok = tok_new_noval(state.pos, state.pos, JSON_LBRACKET);
        else if (state.ch == ']')
            tok = tok_new_noval(state.pos, state.pos, JSON_RBRACKET);
        else if (state.ch == '{')
            tok = tok_new_noval(state.pos, state.pos, JSON_LBRACE);
        else if (state.ch == '}')
            tok = tok_new_noval(state.pos, state.pos, JSON_RBRACE);
        else if (state.ch == ',')
            tok = tok_new_noval(state.pos, state.pos, JSON_COMMA);
        else if (state.ch == ':')
            tok = tok_new_noval(state.pos, state.pos, JSON_COLON);
        else if (state.ch == '"')
            tok = parse_json_str();
        else if ((state.ch >= '0' && state.ch <= '9') || state.ch == '-'
                 || state.ch == 'I' || state.ch == 'N')
        {
            tok = parse_json_num();
        } else if (state.ch == 't' || state.ch == 'f' || state.ch == 'n')
            tok = parse_json_val();
        else if (state.ch == '/') {
            if (ignore_comment())
                continue;
            else
                tok = nullptr;
        } else {
            Nst_error_setf_syntax(
                "JSON: invalid character" FILE_INFO,
                path, (i64)state.pos.line, (i64)state.pos.col);
            tok = nullptr;
        }

        if (tok == nullptr) {
            Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
            Nst_free(src_text.text);
            Nst_free(src_text.lines);
            return nullptr;
        }
        Nst_llist_append(tokens, tok, true);
        advance();
    }
end:
    Nst_llist_append(tokens, tok_new_noend(state.pos, JSON_EOF), true);
    Nst_free(src_text.text);
    Nst_free(src_text.lines);
    return tokens;
}

static Nst_Tok *parse_json_str()
{
    Nst_Pos start = Nst_pos_copy(state.pos);
    Nst_Pos escape_start = Nst_pos_copy(state.pos);
    bool escape = false;

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, 8))
        return nullptr;

    advance();

    while (state.idx < state.len && (state.ch != '"' || escape)) {
        if (!Nst_sb_reserve(&sb, 3)) {
            Nst_sb_destroy(&sb);
            return nullptr;
        }

        if (!escape) {
            if ((u8)state.ch < ' ') {
                Nst_sb_destroy(&sb);
                Nst_error_setf_syntax(
                    "JSON: invalid character" FILE_INFO,
                    state.pos.text->path,
                    (i64)state.pos.line,
                    (i64)state.pos.col);
                return nullptr;
            } else if (state.ch == '\\') {
                escape = true;
                escape_start = Nst_pos_copy(state.pos);
            } else
                Nst_sb_push_char(&sb, state.ch);
            advance();
            continue;
        }

        // If there is an escape sequence
        switch (state.ch) {
        case '"': Nst_sb_push_char(&sb, '"' ); break;
        case '\\':Nst_sb_push_char(&sb, '\\'); break;
        case '/': Nst_sb_push_char(&sb, '/' ); break;
        case 'b': Nst_sb_push_char(&sb, '\b'); break;
        case 'f': Nst_sb_push_char(&sb, '\f'); break;
        case 'n': Nst_sb_push_char(&sb, '\n'); break;
        case 'r': Nst_sb_push_char(&sb, '\r'); break;
        case 't': Nst_sb_push_char(&sb, '\t'); break;
        case 'u': {
            advance();
            if (state.idx + 3 >= state.len) {
                Nst_sb_destroy(&sb);
                SET_INVALID_ESCAPE_ERROR;
                return nullptr;
            }

            u8 ch1 = (u8)tolower(state.ch);
            advance();
            u8 ch2 = (u8)tolower(state.ch);
            advance();
            u8 ch3 = (u8)tolower(state.ch);
            advance();
            u8 ch4 = (u8)tolower(state.ch);

            if (!IS_HEX(ch1) || !IS_HEX(ch2) || !IS_HEX(ch3) || !IS_HEX(ch4)) {
                Nst_sb_destroy(&sb);
                SET_INVALID_ESCAPE_ERROR;
                return nullptr;
            }

            ch1 = HEX_TO_INT(ch1);
            ch2 = HEX_TO_INT(ch2);
            ch3 = HEX_TO_INT(ch3);
            ch4 = HEX_TO_INT(ch4);

            u32 num = (ch1 << 12) + (ch2 << 8) + (ch3 << 4) + ch4;
            Nst_sb_push_cps(&sb, &num, 1);
            break;
        }
        default:
            Nst_sb_destroy(&sb);
            SET_INVALID_ESCAPE_ERROR;
            return nullptr;
        }

        escape = false;
        advance();
    }

    if (state.ch != '"') {
        JSON_SYNTAX_ERROR("open string", state.path, state.pos);
        Nst_sb_destroy(&sb);
        return nullptr;
    }

    Nst_Obj *val_obj = Nst_str_from_sb(&sb);
    return tok_new_value(start, state.pos, JSON_VALUE, val_obj);
}

static bool check_ident(const i8 *name)
{
    usize l = strlen(name);
    for (usize i = 0; i < l; i++) {
        if (state.ch != name[i]) {
            SET_INVALID_VALUE_ERROR;
            return false;
        }
        advance();
    }
    go_back();
    return true;
}

static Nst_Tok *parse_json_num()
{
    i8 *start_idx = state.pos.text->text + state.idx;
    Nst_Pos start = Nst_pos_copy(state.pos);
    if (state.ch == '-')
        advance();

    if (state.idx >= state.len) {
        JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
        return nullptr;
    } else if (state.ch == '0') {
        advance();
        if (state.ch >= '0' && state.ch <= '9') {
            JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
            return nullptr;
        }

        goto float_ltrl;
    } else if (state.ch > '0' && state.ch <= '9') {
        while (state.ch >= '0' && state.ch <= '9')
            advance();

        if (state.ch == '.' || state.ch == 'e' || state.ch == 'E')
            goto float_ltrl;
        go_back();
        errno = 0;
        i64 value = strtoll(start_idx, nullptr, 10);
        if (errno == ERANGE) {
            Nst_error_setf_memory(
                "JSON: number too big" FILE_INFO,
                state.path,
                (i64)state.pos.line,
                (i64)state.pos.col);
            return nullptr;
        }
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            Nst_int_new(value));
    } else if (state.ch == 'I' && nan_and_inf) {
        if (!check_ident("Infinity"))
            return nullptr;

        if (*start_idx == '-') {
            return tok_new_value(
                start,
                state.pos,
                JSON_VALUE,
                Nst_inc_ref(Nst_const()->Real_neginf));
        } else {
            return tok_new_value(
                start,
                state.pos,
                JSON_VALUE,
                Nst_inc_ref(Nst_const()->Real_inf));
        }
    } else if (state.ch == 'N' && nan_and_inf) {
        if (!check_ident("NaN"))
            return nullptr;

        if (*start_idx == '-') {
            return tok_new_value(
                start,
                state.pos,
                JSON_VALUE,
                Nst_inc_ref(Nst_const()->Real_negnan));
        } else {
            return tok_new_value(
                start,
                state.pos,
                JSON_VALUE,
                Nst_inc_ref(Nst_const()->Real_nan));
        }
    } else {
        JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
        return nullptr;
    }

float_ltrl:
    if (state.ch != '.' && state.ch != 'e' && state.ch != 'E') {
        go_back();
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            Nst_inc_ref(Nst_const()->Int_0));
    }

    if (state.ch == '.') {
        advance();
        if (state.ch < '0' || state.ch > '9') {
            JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
            return nullptr;
        }
        while (state.ch >= '0' && state.ch <= '9')
            advance();
    }

    if (state.ch == 'e' || state.ch == 'E') {
        advance();
        if (state.ch == '+' || state.ch == '-')
            advance();

        if (state.ch < '0' || state.ch > '9') {
            JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
            return nullptr;
        }
        while (state.ch >= '0' && state.ch <= '9')
            advance();
        go_back();
    } else
        go_back();

    {
        f64 value = Nst_strtod(start_idx, nullptr);
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            Nst_real_new(value));
    }
}

static Nst_Tok *parse_json_val()
{
    Nst_Pos start = Nst_pos_copy(state.pos);
    switch (state.ch) {
    case 't':
        if (!check_ident("true"))
            return nullptr;
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            Nst_inc_ref(Nst_true()));
    case 'f':
        if (!check_ident("false"))
            return nullptr;
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            Nst_inc_ref(Nst_false()));
    default:
        if (!check_ident("null"))
            return nullptr;
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            Nst_inc_ref(Nst_null()));
    }
}

static bool ignore_comment()
{
    if (!comments) {
        JSON_SYNTAX_ERROR("invalid character", state.path, state.pos);
        return false;
    }

    advance();
    if (state.idx >= state.len || (state.ch != '/' && state.ch != '*')) {
        JSON_SYNTAX_ERROR("invalid character", state.path, state.pos);
        return false;
    }

    bool is_multiline = state.ch == '*';
    bool can_close = false;

    while (state.idx < state.len) {
        advance();
        if (state.ch == '\n' && !is_multiline) {
            advance();
            return true;
        } else if (state.ch == '/' && can_close) {
            advance();
            return true;
        } else if (state.ch == '*')
            can_close = true;
        else
            can_close = false;
    }

    if (is_multiline) {
        JSON_SYNTAX_ERROR("open multiline comment", state.path, state.pos);
        return false;
    }
    return true;
}
