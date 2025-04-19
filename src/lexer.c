#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "nest.h"

#define START_CH_SIZE 8 * sizeof(u8)
#define CH_TO_HEX(c) (u8)((u8)(c) > '9' ? (u8)(c) - 'a' + 10 : (u8)(c) - '0')

#define CUR_AT_END (cursor.idx >= (i32)cursor.len)

#define CH_IS_DEC(ch) (ch >= '0' && ch <= '9')
#define CH_IS_BIN(ch) (ch == '0' || ch == '1')
#define CH_IS_OCT(ch) (ch >= '0' && ch <= '7')
#define CH_IS_HEX(ch) ((ch >= '0' && ch <= '9')                               \
                    || (ch >= 'a' && ch <= 'f')                               \
                    || (ch >= 'A' && ch <= 'F'))

#define CH_IS_ALPHA(ch) ((ch >= 'a' && ch <= 'z')                             \
                      || (ch >= 'A' && ch <= 'Z')                             \
                      ||  ch == '_')

#define CH_IS_SYMBOL(ch) ((ch >= ':' && ch <= '@')                            \
                       || (ch >= '{' && ch <= '~')                            \
                       || (ch >= '(' && ch <= '/')                            \
                       || (ch >= '#' && ch <= '&')                            \
                       ||  ch == '!' || ch == '['                             \
                       ||  ch == ']' || ch == '^')

#define ADD_ERR_POS                                                           \
    Nst_error_add_span(Nst_span_from_pos(cursor.pos))

typedef struct LexerCursor {
    char *text;
    usize len;
    Nst_Pos pos;
    i32 prev_line_len;
    i32 idx;
    char ch;
    Nst_DynArray *tokens;
} LexerCursor;

static LexerCursor cursor;

static inline void advance(void);
static inline void go_back(void);
static Nst_Tok make_symbol(void);
static Nst_Tok make_num_literal(void);
static Nst_Tok make_ident(void);
static Nst_Tok make_str_literal(void);
static Nst_Tok make_raw_str_literal(void);
static void invalid_escape_error(Nst_Pos escape_start);
static i32 find_fmt_str_inline_end(void);
bool tokenize_internal(i32 max_idx);

static void cursor_init(Nst_SourceText *text, Nst_DynArray *tokens)
{
    cursor.idx = -1;
    cursor.ch = ' ';
    cursor.len = text->text_len;
    cursor.text = text->text;
    cursor.pos.col = -1;
    cursor.pos.line = 0;
    cursor.pos.text = text;
    cursor.tokens = tokens;
    advance();
}

Nst_DynArray Nst_tokenize(Nst_SourceText *text)
{
    Nst_DynArray tokens;

    if (!Nst_da_init(&tokens, sizeof(Nst_Tok), 16)) {
        ADD_ERR_POS;
        Nst_da_init(&tokens, sizeof(Nst_Tok), 0);
        return tokens;
    }
    cursor_init(text, &tokens);

    if (!tokenize_internal((i32)cursor.len)) {
        Nst_da_clear(&tokens, (Nst_Destructor)Nst_tok_destroy);
        return tokens;
    }

    Nst_Tok tok = Nst_tok_new(
        Nst_span_from_pos(cursor.pos),
        Nst_TT_EOFILE,
        NULL);

    if (!Nst_da_append(&tokens, &tok)) {
        ADD_ERR_POS;
        Nst_da_clear(&tokens, (Nst_Destructor)Nst_tok_destroy);
    }

    return tokens;
}

bool tokenize_internal(i32 max_idx)
{
    while (cursor.idx < max_idx) {
        Nst_Tok tok = Nst_tok_invalid();
        if (cursor.ch == ' ' || cursor.ch == '\t') {
            advance();
            continue;
        } else if (CH_IS_DEC(cursor.ch)
                   || cursor.ch == '+'
                   || cursor.ch == '-')
        {
            tok = make_num_literal();
        } else if (CH_IS_SYMBOL(cursor.ch))
            tok = make_symbol();
        else if (CH_IS_ALPHA(cursor.ch) || (u8)cursor.ch >= 0x80)
            tok = make_ident();
        else if (cursor.ch == '"' || cursor.ch == '\'')
            tok = make_str_literal();
        else if (cursor.ch == '`')
            tok = make_raw_str_literal();
        else if (cursor.ch == '\n') {
            tok = Nst_tok_new(Nst_span_from_pos(cursor.pos), Nst_TT_ENDL, NULL);
        } else if (cursor.ch == '\\') {
            advance();
            i32 res = Nst_check_utf8_bytes(
                (u8 *)cursor.text + cursor.idx,
                (usize)(cursor.text - cursor.idx));
            for (i32 i = 0; i < res - 1; i++)
                advance();
        } else {
            Nst_error_setc_syntax("invalid character");
            Nst_error_add_span(Nst_span_from_pos(cursor.pos));
        }

        if (Nst_error_occurred())
            return false;
        if (tok.type != Nst_TT_INVALID && !Nst_da_append(cursor.tokens, &tok))
            return false;
        advance();
    }

    return true;
}

inline static void advance(void)
{
    cursor.idx++;
    cursor.pos.col++;

    if (cursor.idx > (i32)cursor.len) {
        cursor.ch = '\0';
        return;
    }

    if (cursor.ch == '\n') {
        cursor.prev_line_len = cursor.pos.col;
        cursor.pos.col = 0;
        cursor.pos.line++;
    }

    cursor.ch = cursor.text[cursor.idx];
}

inline static void go_back(void)
{
    cursor.idx--;
    cursor.pos.col--;

    if (cursor.idx < 0)
        return;

    cursor.ch = cursor.text[cursor.idx];
    if (cursor.ch == '\n') {
        cursor.pos.line--;
        cursor.pos.col = cursor.prev_line_len;
    }
}

static Nst_Tok make_symbol(void)
{
    Nst_Pos start = cursor.pos;
    u8 symbol[4] = { cursor.ch, 0, 0, 0 };
    advance();
    if (!CUR_AT_END && CH_IS_SYMBOL(cursor.ch)) {
        symbol[1] = cursor.ch;
        advance();

        if (!CUR_AT_END && CH_IS_SYMBOL(cursor.ch))
            symbol[2] = cursor.ch;
        else
            go_back();
    } else
        go_back();

    if (symbol[0] == '-' && symbol[1] == '-') {
        while (!CUR_AT_END && cursor.ch != '\n') {
            if (cursor.ch == '\\') {
                advance();

                // Allows an even number of \ to escape the new line
                if (cursor.ch == '\\')
                    go_back();
            }
            advance();
        }
        go_back();
        return Nst_tok_invalid();
    } else if (symbol[0] == '-' && symbol[1] == '/') {
        bool can_close = false;
        bool was_closed = false;

        while (!CUR_AT_END) {
            if (can_close && cursor.ch == '-') {
                advance();
                was_closed = true;
                break;
            }

            can_close = cursor.ch == '/';
            advance();
        }

        go_back();

        if (!was_closed) {
            Nst_error_setc_syntax("multi-line comment was never closed");
            Nst_error_add_span(Nst_span_new(start, cursor.pos));
        }

        return Nst_tok_invalid();
    }

    if (symbol[1] == '-' && symbol[2] == '-') {
        symbol[1] = '\0';
        symbol[2] = '\0';
        go_back();
        go_back();
    } else if (symbol[1] == '-' && symbol[2] == '/') {
        symbol[1] = '\0';
        symbol[2] = '\0';
        go_back();
        go_back();
    } else if (symbol[2] == '-') {
        // A '-' at the end might start a comment, il can be checked only when
        // it is at index 0 or 1
        symbol[2] = '\0';
        go_back();
    }

    Nst_TokType token_type = Nst_tok_type_from_str(symbol);

    while (token_type == Nst_TT_INVALID) {
        go_back();

        if (symbol[2] != '\0')
            symbol[2] = '\0';
        else if (symbol[1] != '\0')
            symbol[1] = '\0';

        token_type = Nst_tok_type_from_str(symbol);
    }

    return Nst_tok_new(Nst_span_new(start, cursor.pos), token_type, NULL);
}

static i32 bin_ltrl_size(Nst_Pos start, bool *is_byte)
{
    advance(); // advance from the initial 'b' or 'B'

    // treat 0b and 0B as a byte with value 0
    if (!CH_IS_BIN(cursor.ch)) {
        *is_byte = true;
        go_back();
        return 2;
    }
    i32 ltrl_size = 2;
    do {
        ltrl_size++;
        advance();
    } while (CH_IS_BIN(cursor.ch) || cursor.ch == '_');
    if (CH_IS_DEC(cursor.ch)) {
        Nst_error_setc_syntax("invalid Int literal");
        Nst_error_add_span(Nst_span_new(start, cursor.pos));
        return -1;
    }
    if (cursor.ch == 'b' || cursor.ch == 'B') {
        ltrl_size++;
        *is_byte = true;
    } else
        go_back();
    return ltrl_size;
}

static i32 oct_ltrl_size(Nst_Pos start, bool *is_byte)
{
    advance(); // advance from the initial 'o' or 'O'

    if (!CH_IS_OCT(cursor.ch)) {
        go_back();
        Nst_error_setc_syntax("invalid Int literal");
        Nst_error_add_span(Nst_span_new(start, cursor.pos));
        return -1;
    }
    i32 ltrl_size = 2;
    do {
        ltrl_size++;
        advance();
    } while (CH_IS_OCT(cursor.ch) || cursor.ch == '_');
    if (CH_IS_DEC(cursor.ch)) {
        Nst_error_setc_syntax("invalid Int literal");
        Nst_error_add_span(Nst_span_new(start, cursor.pos));
        return -1;
    }
    if (cursor.ch == 'b' || cursor.ch == 'B') {
        ltrl_size++;
        *is_byte = true;
    } else
        go_back();
    return ltrl_size;
}

static i32 hex_ltrl_size(Nst_Pos start, bool *is_byte)
{
    const char *err_msg = "invalid Int literal";
    if (cursor.ch == 'h' || cursor.ch == 'H') {
        *is_byte = true;
        err_msg = "invalid Byte literal";
    }
    advance();
    if (!CH_IS_HEX(cursor.ch)) {
        go_back();
        Nst_error_setc_syntax(err_msg);
        Nst_error_add_span(Nst_span_new(start, cursor.pos));
        return -1;
    }
    i32 ltrl_size = 2;
    do {
        ltrl_size++;
        advance();
    } while (CH_IS_HEX(cursor.ch) || cursor.ch == '_');
    if (CH_IS_ALPHA(cursor.ch)) {
        Nst_error_setc_syntax(err_msg);
        Nst_error_add_span(Nst_span_new(start, cursor.pos));
        return -1;
    }
    go_back();
    return ltrl_size;
}

static i32 dec_ltrl_size(Nst_Pos start, bool *is_byte, bool *is_real)
{
    i32 ltrl_size = 0;
    do {
        advance();
        ltrl_size++;
    } while (CH_IS_DEC(cursor.ch) || cursor.ch == '_');

    if (cursor.ch == 'b' || cursor.ch == 'B') {
        ltrl_size++;
        *is_byte = true;
        return ltrl_size;
    } else if (cursor.ch != '.') {
        go_back();
        return ltrl_size;
    }

    *is_real = true;
    advance();
    ltrl_size++;

    if (!CH_IS_DEC(cursor.ch)) {
        Nst_error_setc_syntax("invalid Real literal");
        Nst_error_add_span(Nst_span_new(start, cursor.pos));
        return -1;
    }
    do {
        advance();
        ltrl_size++;
    } while (CH_IS_DEC(cursor.ch) || cursor.ch == '_');

    if (cursor.ch != 'e' && cursor.ch != 'E') {
        go_back();
        return ltrl_size;
    }
    advance();
    ltrl_size++;
    if (cursor.ch == '+' || cursor.ch == '-') {
        advance();
        ltrl_size++;
    }
    if (!CH_IS_DEC(cursor.ch)) {
        Nst_error_setc_syntax("invalid Real literal");
        Nst_error_add_span(Nst_span_new(start, cursor.pos));
        return -1;
    }
    do {
        advance();
        ltrl_size++;
    } while (CH_IS_DEC(cursor.ch) || cursor.ch == '_');
    go_back();
    return ltrl_size;
}

static Nst_Tok make_num_literal(void)
{
    char *start_p = cursor.text + cursor.idx;
    Nst_Pos start = cursor.pos;

    i32 ltrl_size = 0;
    u8 *ltrl = NULL;
    bool neg = false;
    bool is_real = false;
    bool is_byte = false;
    bool parse_res = false;
    Nst_Obj *res = NULL;

    if (cursor.ch == '-' || cursor.ch == '+') {
        neg = cursor.ch == '-';
        advance();
        if (!CH_IS_DEC(cursor.ch)) {
            go_back();
            return make_symbol();
        }
        start_p++;
    }

    if (cursor.ch != '0')
        goto dec_num;

    advance();
    switch (cursor.ch) {
    case 'b':
    case 'B':
        ltrl_size = bin_ltrl_size(start, &is_byte);
        if (ltrl_size < 0)
            return Nst_tok_invalid();
        goto end;
    case 'o':
    case 'O':
        ltrl_size = oct_ltrl_size(start, &is_byte);
        if (ltrl_size < 0)
            return Nst_tok_invalid();
        goto end;
    case 'x':
    case 'X':
    case 'h':
    case 'H':
        ltrl_size = hex_ltrl_size(start, &is_byte);
        if (ltrl_size < 0)
            return Nst_tok_invalid();
        goto end;
    default:
        go_back();
    }

dec_num:
    ltrl_size = dec_ltrl_size(start, &is_byte, &is_real);
    if (ltrl_size < 0)
        return Nst_tok_invalid();

end:
    ltrl = Nst_malloc_c(ltrl_size + 2, u8);
    if (ltrl == NULL) {
        ADD_ERR_POS;
        return Nst_tok_invalid();
    }
    ltrl[0] = neg ? '-' : '+';
    memcpy(ltrl + 1, start_p, ltrl_size);
    ltrl[ltrl_size + 1] = '\0';

    if (is_real) {
        f64 value;
        parse_res = Nst_sv_parse_real(
            Nst_sv_new(ltrl, ltrl_size + 1),
            Nst_SVFLAG_STRICT_REAL | Nst_SVFLAG_FULL_MATCH,
            &value, NULL);
        if (parse_res)
            res = Nst_real_new(value);
    } else if (is_byte) {
        u8 value;
        parse_res = Nst_sv_parse_byte(
            Nst_sv_new(ltrl, ltrl_size + 1), 0,
            Nst_SVFLAG_CHAR_BYTE
            | Nst_SVFLAG_FULL_MATCH
            | Nst_SVFLAG_CAN_OVERFLOW,
            &value, NULL);
        if (parse_res)
            res = Nst_byte_new(value);
    } else {
        i64 value;
        parse_res = Nst_sv_parse_int(
            Nst_sv_new(ltrl, ltrl_size + 1),
            0, Nst_SVFLAG_FULL_MATCH,
            &value, NULL);
        if (parse_res)
            res = Nst_int_new(value);
    }
    Nst_free(ltrl);

    if (!parse_res || res == NULL) {
        ADD_ERR_POS;
        return Nst_tok_invalid();
    }
    return Nst_tok_new(Nst_span_new(start, cursor.pos), Nst_TT_VALUE, res);
}

static Nst_Tok make_ident(void)
{
    Nst_Pos start = cursor.pos;
    char *str;
    char *str_start = cursor.text + cursor.idx;
    usize str_len = 0;

    while (!CUR_AT_END
           && (CH_IS_ALPHA(cursor.ch)
               || CH_IS_DEC(cursor.ch)
               || (u8)cursor.ch >= 0x80))
    {
        i32 res = Nst_check_utf8_bytes(
            (u8 *)cursor.text + cursor.idx,
            (usize)(cursor.text - cursor.idx));
        str_len += res;
        for (i32 i = 0; i < res; i++)
            advance();
    }
    go_back();

    str = Nst_malloc_c(str_len + 1, char);
    if (str == NULL) {
        ADD_ERR_POS;
        return Nst_tok_invalid();
    }

    memcpy(str, str_start, str_len);
    str[str_len] = '\0';

    Nst_Pos end = cursor.pos;
    Nst_Obj *val_obj = Nst_str_new_c_raw(str, true);
    if (val_obj == NULL) {
        Nst_free(str);
        ADD_ERR_POS;
        return Nst_tok_invalid();
    }
    Nst_obj_hash(val_obj);

    return Nst_tok_new(Nst_span_new(start, end), Nst_TT_IDENT, val_obj);
}

static bool x_escape(Nst_StrBuilder *sb, Nst_Pos escape_start)
{
    if ((usize)cursor.idx + 2 >= cursor.len) {
        invalid_escape_error(escape_start);
        return false;
    }

    advance();
    u8 ch1 = (u8)tolower(cursor.ch);
    advance();
    u8 ch2 = (u8)tolower(cursor.ch);

    if (!CH_IS_HEX(ch1) || !CH_IS_HEX(ch2)) {
        invalid_escape_error(escape_start);
        return false;
    }

    u8 result = (CH_TO_HEX(ch1) << 4) + CH_TO_HEX(ch2);
    if (result >= 0x80) {
        u8 utf8_b1 = 0b11000000 | (u8)(result >> 6);
        u8 utf8_b2 = 0b10000000 | (u8)(result & 0x3f);
        Nst_sb_push_char(sb, utf8_b1);
        Nst_sb_push_char(sb, utf8_b2);
    } else
        Nst_sb_push_char(sb, (u8)result);
    return true;
}

static bool u_escape(Nst_StrBuilder *sb, Nst_Pos escape_start)
{
    i32 size = cursor.ch == 'U' ? 6 : 4;
    if ((usize)cursor.idx + size >= cursor.len) {
        invalid_escape_error(escape_start);
        return false;
    }

    i32 num = 0;
    for (i32 i = 0; i < size; i++) {
        advance();
        u8 ch = (u8)tolower(cursor.ch);
        if (!CH_IS_HEX(ch)) {
            invalid_escape_error(escape_start);
            return false;
        }

        ch = (u8)CH_TO_HEX(ch);
        num += ch << (4 * (size - i - 1));
    }

    if (num > 0x10ffff) {
        invalid_escape_error(escape_start);
        return false;
    }

    Nst_sb_push_cps(sb, (u32 *)&num, 1);
    return true;
}

static void o_escape(Nst_StrBuilder *sb)
{
    u8 ch1 = cursor.ch - '0';

    advance();
    if (!CH_IS_OCT(cursor.ch)) {
        Nst_sb_push_char(sb, ch1);
        go_back();
        return;
    }
    u8 ch2 = cursor.ch - '0';

    advance();
    if (!CH_IS_OCT(cursor.ch)) {
        Nst_sb_push_char(sb, (ch1 << 3) + ch2);
        go_back();
        return;
    }
    u8 ch3 = cursor.ch - '0';
    u32 result = (ch1 << 6) + (ch2 << 3) + ch3;

    Nst_sb_push_cps(sb, &result, 1);
}

static bool format_escape(Nst_StrBuilder *sb, bool is_format_string,
                          Nst_Pos *start)
{
    i32 max_idx = find_fmt_str_inline_end();
    if (max_idx == -1) {
        Nst_error_setc_syntax("invalid format string");
        Nst_error_add_span(Nst_span_from_pos(cursor.pos));
        return false;
    }
    advance();

    Nst_Tok tok;

    if (!is_format_string) {
        tok = Nst_tok_new(Nst_span_from_pos(cursor.pos), Nst_TT_L_PAREN, NULL);
        if (!Nst_da_append(cursor.tokens, &tok))
            return false;
    }

    Nst_Obj *val_obj = Nst_str_from_sb(sb);
    if (val_obj == NULL) {
        ADD_ERR_POS;
        return false;
    }
    Nst_obj_hash(val_obj);

    tok = Nst_tok_new(
        Nst_span_new(*start, cursor.pos),
        Nst_TT_VALUE,
        val_obj);
    if (!Nst_da_append(cursor.tokens, &tok))
        return false;

    tok = Nst_tok_new(Nst_span_from_pos(cursor.pos), Nst_TT_L_PAREN, NULL);
    if (!Nst_da_append(cursor.tokens, &tok))
        return false;

    if (!tokenize_internal(max_idx))
        return false;

    tok = Nst_tok_new(Nst_span_from_pos(cursor.pos), Nst_TT_R_PAREN, NULL);
    if (!Nst_da_append(cursor.tokens, &tok))
        return false;

    if (!Nst_sb_init(sb, START_CH_SIZE)) {
        ADD_ERR_POS;
        return false;
    }
    *start = cursor.pos;
    return true;
}

static Nst_Tok make_str_literal(void)
{
    Nst_Pos start = cursor.pos;
    Nst_Pos escape_start = start;
    u8 closing_ch = cursor.ch;
    bool allow_multiline = cursor.ch == '"';
    bool is_format_string = false;

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, START_CH_SIZE)) {
        ADD_ERR_POS;
        return Nst_tok_invalid();
    }

    advance(); // still on the opening character

    // while there is text to add and (the string has not ended or
    // the end is inside and escape)
    while (!CUR_AT_END && cursor.ch != closing_ch) {
        if (!Nst_sb_reserve(&sb, 4)) {
            ADD_ERR_POS;
            goto failure;
        }

        if (cursor.ch == '\n' && !allow_multiline) {
            Nst_error_setc_syntax(
                "single-quote strings cannot span multiple lines");
            Nst_error_add_span(Nst_span_from_pos(cursor.pos));
            goto failure;
        } else if (cursor.ch == '\\') {
            escape_start = cursor.pos;
            advance();
        } else {
            Nst_sb_push_char(&sb, cursor.ch);
            advance();
            continue;
        }

        switch (cursor.ch) {
        case '\'':Nst_sb_push_char(&sb, '\''); break;
        case '"': Nst_sb_push_char(&sb, '"');  break;
        case '\\':Nst_sb_push_char(&sb, '\\'); break;
        case 'a': Nst_sb_push_char(&sb, '\a'); break;
        case 'b': Nst_sb_push_char(&sb, '\b'); break;
        case 'e': Nst_sb_push_char(&sb,'\x1b');break;
        case 'f': Nst_sb_push_char(&sb, '\f'); break;
        case 'n': Nst_sb_push_char(&sb, '\n'); break;
        case 'r': Nst_sb_push_char(&sb, '\r'); break;
        case 't': Nst_sb_push_char(&sb, '\t'); break;
        case 'v': Nst_sb_push_char(&sb, '\v'); break;
        case 'x':
            if (!x_escape(&sb, escape_start))
                goto failure;
            break;
        case 'u':
        case 'U':
            if (!u_escape(&sb, escape_start))
                goto failure;
            break;
        case '(':
            if (!format_escape(&sb, is_format_string, &start))
                goto failure;
            is_format_string = true;
            break;
        default:
            if (cursor.ch >= '0' && cursor.ch <= '7')
                o_escape(&sb);
            else {
                invalid_escape_error(escape_start);
                goto failure;
            }
        }

        advance();
    }

    if (cursor.ch != closing_ch) {
        Nst_error_setc_syntax("string literal was never closed");
        Nst_error_add_span(Nst_span_from_pos(cursor.pos));
        goto failure;
    }

    Nst_Obj *val_obj = Nst_str_from_sb(&sb);
    if (val_obj == NULL) {
        ADD_ERR_POS;
        return Nst_tok_invalid();
    }
    Nst_obj_hash(val_obj);

    if (!is_format_string) {
        return Nst_tok_new(
            Nst_span_new(start, cursor.pos),
            Nst_TT_VALUE,
            val_obj);
    }

    Nst_Tok tok;
    if (Nst_str_len(val_obj) == 0)
        Nst_dec_ref(val_obj);
    else {
        tok = Nst_tok_new(
            Nst_span_new(start, cursor.pos),
            Nst_TT_VALUE,
            val_obj);
        if (!Nst_da_append(cursor.tokens, &tok))
            return Nst_tok_invalid();
    }
    tok = Nst_tok_new(Nst_span_from_pos(cursor.pos), Nst_TT_CONCAT, NULL);
    if (!Nst_da_append(cursor.tokens, &tok))
        return Nst_tok_invalid();
    return Nst_tok_new(Nst_span_from_pos(cursor.pos), Nst_TT_R_PAREN, NULL);
failure:
    Nst_sb_destroy(&sb);
    return Nst_tok_invalid();
}

static void invalid_escape_error(Nst_Pos escape_start)
{
    Nst_error_setc_syntax("invalid escape sequence");
    Nst_error_add_span(Nst_span_new(escape_start, cursor.pos));
}

static bool skip_inline_str(void)
{
    u8 closing_char = cursor.ch;
    advance(); // skip the initial " or '

    while (cursor.ch != closing_char && !CUR_AT_END) {
        if (cursor.ch != '\\') {
            advance();
            continue;
        }
        advance();
        if (CUR_AT_END)
            return false;
        if (cursor.ch != '(') {
            advance();
            continue;
        }
        cursor.idx = find_fmt_str_inline_end();
    }

    return cursor.ch == closing_char;
}

static i32 find_fmt_str_inline_end(void)
{
    i32 initial_idx = cursor.idx;
    Nst_Pos initial_pos = cursor.pos;
    i32 pairs = 0;
    advance();

    while (!CUR_AT_END) {
        if (cursor.ch == '(') {
            pairs++;
        }
        else if (cursor.ch == ')') {
            if (pairs > 0)
                pairs--;
            else
                break;
        } else if (cursor.ch == '\\') {
            advance();
            if (CUR_AT_END)
                break;
        } else if (cursor.ch == '"' || cursor.ch == '\'') {
            if (!skip_inline_str())
                return -1;
        } else if (cursor.ch == '`') {
            advance();
            // Escaped backtics can be thought as "splitting" the raw string
            // in two, so `a``b` will just run `a` first and `b` next
            while (!CUR_AT_END && cursor.ch != '`')
                advance();
        } else if (cursor.ch == '-') {
            advance();
            if (CUR_AT_END)
                break;
            if (cursor.ch == '-' || cursor.ch == '/')
                return -1;
            go_back();
        }
        advance();
    }

    if (cursor.ch == '\0')
        return -1;

    i32 max_idx = cursor.idx;
    cursor.pos = initial_pos;
    cursor.idx = initial_idx;
    return max_idx;
}

static Nst_Tok make_raw_str_literal(void)
{
    Nst_Pos start = cursor.pos;
    Nst_Pos end = start;

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, START_CH_SIZE)) {
        ADD_ERR_POS;
        return Nst_tok_invalid();
    }
    advance(); // still on the opening character

    while (!CUR_AT_END) {
        if (cursor.ch == '`') {
            end = cursor.pos;
            advance();
            if (CUR_AT_END || cursor.ch != '`') {
                go_back();
                break;
            }
        }

        if (!Nst_sb_push_char(&sb, cursor.ch)) {
            ADD_ERR_POS;
            goto failure;
        }
        advance();
    }

    if (cursor.ch != '`') {
        Nst_error_setc_syntax("string literal was never closed");
        Nst_error_add_span(Nst_span_from_pos(cursor.pos));
        goto failure;
    }

    Nst_Obj *val_obj = Nst_str_from_sb(&sb);
    if (val_obj == NULL) {
        ADD_ERR_POS;
        return Nst_tok_invalid();
    }
    Nst_obj_hash(val_obj);

    return Nst_tok_new(Nst_span_new(start, end), Nst_TT_VALUE, val_obj);
failure:
    Nst_sb_destroy(&sb);
    return Nst_tok_invalid();
}
