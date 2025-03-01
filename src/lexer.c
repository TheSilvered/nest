#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "str_builder.h"
#include "hash.h"
#include "lexer.h"
#include "global_consts.h"
#include "tokens.h"
#include "interpreter.h" // Nst_get_full_path
#include "encoding.h"
#include "format.h"

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
    Nst_error_add_pos(cursor.pos, cursor.pos)

typedef struct LexerCursor {
    char *text;
    usize len;
    Nst_Pos pos;
    i32 prev_line_len;
    i32 idx;
    char ch;
    Nst_LList *tokens;
} LexerCursor;

static LexerCursor cursor;

static inline void advance(void);
static inline void go_back(void);
static Nst_Tok *make_symbol(void);
static Nst_Tok *make_num_literal(void);
static Nst_Tok *make_ident(void);
static Nst_Tok *make_str_literal(void);
static Nst_Tok *make_raw_str_literal(void);
static void invalid_escape_error(Nst_Pos escape_start);
static i32 find_fmt_str_inline_end(void);
static void parse_first_line(char *text, usize len, i32 *opt_level,
                             Nst_EncodingID *encoding, bool *no_default);
bool tokenize_internal(i32 max_idx);

static void cursor_init(Nst_SourceText *text, Nst_LList *tokens)
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

Nst_LList *Nst_tokenizef(const char *filename, Nst_EncodingID encoding,
                         i32 *opt_level, bool *no_default,
                         Nst_SourceText *src_text)
{
    FILE *file = NULL;
    char *text = NULL;
    char *full_path = NULL;

    file = Nst_fopen_unicode(filename, "rb");

    if (file == NULL) {
        if (!Nst_error_occurred())
            Nst_error_setf_value("File \"%.100s\" not found.", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    text = Nst_calloc(size + 1, sizeof(char), NULL);
    if (text == NULL)
        goto cleanup;

    usize str_len = fread(text, sizeof(char), size + 1, file);
    text[str_len] = '\0';
    fclose(file);
    file = NULL;

    Nst_get_full_path(filename, &full_path, NULL);
    if (full_path == NULL)
        goto cleanup;

    src_text->text = text;
    src_text->text_len = str_len;
    src_text->path = full_path;

    parse_first_line(text, str_len, opt_level, &encoding, no_default);

    if (!Nst_normalize_encoding(src_text, encoding))
        goto cleanup;

    if (!Nst_add_lines(src_text))
        goto cleanup;

    Nst_LList *tokens = Nst_tokenize(src_text);
    return tokens;
cleanup:
    if (file != NULL)
        fclose(file);
    return NULL;
}

Nst_LList *Nst_tokenize(Nst_SourceText *text)
{
    Nst_LList *tokens = Nst_llist_new();

    if (tokens == NULL) {
        ADD_ERR_POS;
        return NULL;
    }
    cursor_init(text, tokens);

    if (!tokenize_internal((i32)cursor.len)) {
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
        return NULL;
    }

    Nst_Tok *tok = Nst_tok_new_noend(cursor.pos, Nst_TT_EOFILE);
    if (tok == NULL) {
        ADD_ERR_POS;
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
        return NULL;
    }

    if (!Nst_llist_append(tokens, tok, true)) {
        ADD_ERR_POS;
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
        return NULL;
    }

    return tokens;
}

bool tokenize_internal(i32 max_idx)
{
    Nst_Tok *tok = NULL;

    while (cursor.idx < max_idx) {
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
            tok = Nst_tok_new_noend(Nst_pos_copy(cursor.pos), Nst_TT_ENDL);
            if (tok == NULL)
                ADD_ERR_POS;
        } else if (cursor.ch == '\\') {
            advance();
            i32 res = Nst_check_utf8_bytes(
                (u8 *)cursor.text + cursor.idx,
                (usize)(cursor.text - cursor.idx));
            for (i32 i = 0; i < res - 1; i++)
                advance();
        } else {
            Nst_error_setc_syntax("invalid character");
            Nst_error_add_pos(cursor.pos, cursor.pos);
        }

        if (Nst_error_occurred()) {
            if (tok != NULL)
                Nst_tok_destroy(tok);
            return false;
        }

        if (tok != NULL)
            Nst_llist_append(cursor.tokens, tok, true);
        tok = NULL;
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

static Nst_Tok *make_symbol()
{
    Nst_Pos start = Nst_pos_copy(cursor.pos);
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
        return NULL;
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
            Nst_error_add_pos(start, cursor.pos);
        }

        return NULL;
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

    Nst_TokType token_type = Nst_tok_from_str(symbol);

    while (token_type == Nst_TT_INVALID) {
        go_back();

        if (symbol[2] != '\0')
            symbol[2] = '\0';
        else if (symbol[1] != '\0')
            symbol[1] = '\0';

        token_type = Nst_tok_from_str(symbol);
    }

    Nst_Tok *tok = Nst_tok_new_noval(
        start,
        Nst_pos_copy(cursor.pos),
        token_type);
    if (tok == NULL)
        ADD_ERR_POS;
    return tok;
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
        Nst_error_add_pos(start, cursor.pos);
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
        Nst_error_add_pos(start, cursor.pos);
        return -1;
    }
    i32 ltrl_size = 2;
    do {
        ltrl_size++;
        advance();
    } while (CH_IS_OCT(cursor.ch) || cursor.ch == '_');
    if (CH_IS_DEC(cursor.ch)) {
        Nst_error_setc_syntax("invalid Int literal");
        Nst_error_add_pos(start, cursor.pos);
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
        Nst_error_add_pos(start, cursor.pos);
        return -1;
    }
    i32 ltrl_size = 2;
    do {
        ltrl_size++;
        advance();
    } while (CH_IS_HEX(cursor.ch) || cursor.ch == '_');
    if (CH_IS_ALPHA(cursor.ch)) {
        Nst_error_setc_syntax(err_msg);
        Nst_error_add_pos(start, cursor.pos);
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
        Nst_error_add_pos(start, cursor.pos);
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
        Nst_error_add_pos(start, cursor.pos);
        return -1;
    }
    do {
        advance();
        ltrl_size++;
    } while (CH_IS_DEC(cursor.ch) || cursor.ch == '_');
    go_back();
    return ltrl_size;
}

static Nst_Tok *make_num_literal(void)
{
    char *start_p = cursor.text + cursor.idx;
    Nst_Pos start = Nst_pos_copy(cursor.pos);
    Nst_Tok *tok = NULL;

    i32 ltrl_size = 0;
    u8 *ltrl = NULL;
    bool neg = false;
    bool is_real = false;
    bool is_byte = false;
    Nst_Obj *res;

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
            return NULL;
        goto end;
    case 'o':
    case 'O':
        ltrl_size = oct_ltrl_size(start, &is_byte);
        if (ltrl_size < 0)
            return NULL;
        goto end;
    case 'x':
    case 'X':
    case 'h':
    case 'H':
        ltrl_size = hex_ltrl_size(start, &is_byte);
        if (ltrl_size < 0)
            return NULL;
        goto end;
    default:
        go_back();
    }

dec_num:
    ltrl_size = dec_ltrl_size(start, &is_byte, &is_real);
    if (ltrl_size < 0)
        return NULL;

end:
    ltrl = Nst_malloc_c(ltrl_size + 2, u8);
    if (ltrl == NULL) {
        ADD_ERR_POS;
        return NULL;
    }
    ltrl[0] = neg ? '-' : '+';
    memcpy(ltrl + 1, start_p, ltrl_size);
    ltrl[ltrl_size + 1] = '\0';

    if (is_real)
        res = Nst_sv_parse_real(Nst_sv_new(ltrl, ltrl_size + 1));
    else if (is_byte)
        res = Nst_sv_parse_byte(Nst_sv_new(ltrl, ltrl_size + 1));
    else
        res = Nst_sv_parse_int(Nst_sv_new(ltrl, ltrl_size + 1), 0);
    Nst_free(ltrl);

    if (res == NULL) {
        ADD_ERR_POS;
        return NULL;
    }
    tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, res);
    if (tok == NULL)
        ADD_ERR_POS;
    return tok;
}

static Nst_Tok *make_ident(void)
{
    Nst_Pos start = Nst_pos_copy(cursor.pos);
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
        return NULL;
    }

    memcpy(str, str_start, str_len);
    str[str_len] = '\0';

    Nst_Pos end = Nst_pos_copy(cursor.pos);
    Nst_Obj *val_obj = Nst_str_new_c_raw(str, true);
    if (val_obj == NULL) {
        Nst_free(str);
        ADD_ERR_POS;
        return NULL;
    }
    Nst_obj_hash(val_obj);

    Nst_Tok *tok = Nst_tok_new_value(start, end, Nst_TT_IDENT, val_obj);
    if (tok == NULL)
        ADD_ERR_POS;
    return tok;
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

static bool add_token(Nst_Tok *tok)
{
    if (tok == NULL)
        return false;
    if (Nst_llist_append(cursor.tokens, tok, true))
        return true;
    Nst_free(tok);
    return false;
}

static bool format_escape(Nst_StrBuilder *sb, bool is_format_string,
                          Nst_Pos *start)
{
    i32 max_idx = find_fmt_str_inline_end();
    if (max_idx == -1) {
        Nst_error_setc_syntax("invalid format string");
        Nst_error_add_pos(cursor.pos, cursor.pos);
        return false;
    }
    advance();

    Nst_Tok *tok = NULL;

    if (!is_format_string) {
        tok = Nst_tok_new_noend(cursor.pos, Nst_TT_L_PAREN);
        if (tok == NULL)
            return false;
        else if (!Nst_llist_append(cursor.tokens, tok, true))
            return false;
    }

    Nst_Obj *val_obj = Nst_str_from_sb(sb);
    if (val_obj == NULL) {
        ADD_ERR_POS;
        return false;
    }
    Nst_obj_hash(val_obj);

    tok = Nst_tok_new_value(
        *start, cursor.pos,
        Nst_TT_VALUE,
        val_obj);
    if (!add_token(tok))
        return false;

    tok = Nst_tok_new_noend(cursor.pos, Nst_TT_L_PAREN);
    if (!add_token(tok))
        return false;

    if (!tokenize_internal(max_idx))
        return false;

    tok = Nst_tok_new_noend(cursor.pos, Nst_TT_R_PAREN);
    if (!add_token(tok))
        return false;

    if (!Nst_sb_init(sb, START_CH_SIZE)) {
        ADD_ERR_POS;
        return false;
    }
    *start = Nst_pos_copy(cursor.pos);
    return true;
}

static Nst_Tok *make_str_literal(void)
{
    Nst_Pos start = Nst_pos_copy(cursor.pos);
    Nst_Pos escape_start = start;
    u8 closing_ch = cursor.ch;
    bool allow_multiline = cursor.ch == '"';
    bool is_format_string = false;
    Nst_Tok *tok;

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, START_CH_SIZE)) {
        ADD_ERR_POS;
        return NULL;
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
            Nst_error_add_pos(cursor.pos, cursor.pos);
            goto failure;
        } else if (cursor.ch == '\\') {
            escape_start = Nst_pos_copy(cursor.pos);
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
        Nst_error_add_pos(cursor.pos, cursor.pos);
        goto failure;
    }

    Nst_Obj *val_obj = Nst_str_from_sb(&sb);
    if (val_obj == NULL) {
        ADD_ERR_POS;
        return NULL;
    }
    Nst_obj_hash(val_obj);

    if (!is_format_string) {
        tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, val_obj);
        if (tok == NULL)
            ADD_ERR_POS;
        return tok;
    }
    if (Nst_str_len(val_obj) == 0)
        Nst_dec_ref(val_obj);
    else {
        tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, val_obj);
        if (!add_token(tok))
            return NULL;
    }
    tok = Nst_tok_new_noend(cursor.pos, Nst_TT_CONCAT);
    if (!add_token(tok))
        return NULL;
    tok = Nst_tok_new_noend(cursor.pos, Nst_TT_R_PAREN);
    if (tok == NULL)
        ADD_ERR_POS;
    return tok;
failure:
    Nst_sb_destroy(&sb);
    return NULL;
}

static void invalid_escape_error(Nst_Pos escape_start)
{
    Nst_error_setc_syntax("invalid escape sequence");
    Nst_error_add_pos(escape_start, cursor.pos);
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

static Nst_Tok *make_raw_str_literal(void)
{
    Nst_Pos start = Nst_pos_copy(cursor.pos);
    Nst_Pos end = start;
    Nst_Tok *tok;

    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, START_CH_SIZE)) {
        ADD_ERR_POS;
        return NULL;
    }
    advance(); // still on the opening character

    while (!CUR_AT_END) {
        if (cursor.ch == '`') {
            end = Nst_pos_copy(cursor.pos);
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
        Nst_error_add_pos(cursor.pos, cursor.pos);
        goto failure;
    }

    Nst_Obj *val_obj = Nst_str_from_sb(&sb);
    if (val_obj == NULL) {
        ADD_ERR_POS;
        return NULL;
    }
    Nst_obj_hash(val_obj);

    tok = Nst_tok_new_value(start, end, Nst_TT_VALUE, val_obj);
    if (tok == NULL)
        ADD_ERR_POS;
    return tok;
failure:
    Nst_sb_destroy(&sb);
    return NULL;
}

bool Nst_add_lines(Nst_SourceText *text)
{
    char *text_p = text->text;
    char **starts = (char **)Nst_calloc(100, sizeof(char *), NULL);
    if (starts == NULL) {
        text->lines = NULL;
        text->lines_len = 0;
        return false;
    }

    starts[0] = text_p;
    usize line_count = 1;

    // normalize line endings

    // if the file contains \n, then \n doesn't change and \r\n becomes just \n
    // if the file only contains \r, it is replaced with \n

    bool remove_r = false;
    for (usize i = 0, n = text->text_len; i < n; i++) {
        if (text_p[i] == '\n') {
            remove_r = true;
            break;
        }
    }

    usize offset = 0;
    for (usize i = 0, n = text->text_len; i < n; i++) {
        if (text_p[i] != '\r')
            text_p[i - offset] = text_p[i];
        else if (remove_r)
            offset++;
        else
            text_p[i] = '\n';
    }

    text->text_len = text->text_len - offset;
    text->text[text->text_len] = '\0';

    // now all lines end with \n
    for (usize i = 0, n = text->text_len; i < n; i++) {
        if (text_p[i] != '\n')
            continue;

        if (i + 1 == n) {
            text->lines = starts;
            text->lines_len = line_count;
        }

        line_count++;

        if (line_count % 100 == 0) {
            char **temp = (char **)Nst_realloc(
                starts,
                i + 100,
                sizeof(char *),
                0);
            if (temp == NULL) {
                Nst_free(starts);
                text->lines = NULL;
                text->lines_len = 0;
                return false;
            }
            starts = temp;
        }

        starts[line_count - 1] = text_p + i + 1;
    }

    text->lines = starts;
    text->lines_len = line_count;
    return true;
}

bool Nst_normalize_encoding(Nst_SourceText *text, Nst_EncodingID encoding)
{
    i32 bom_size = 0;
    if (encoding == Nst_EID_UNKNOWN)
        encoding = Nst_encoding_detect(text->text, text->text_len, &bom_size);
    else
        Nst_check_bom(text->text, text->text_len, &bom_size);

    encoding = Nst_encoding_to_single_byte(encoding);
    Nst_Encoding *from = Nst_encoding(encoding);

    Nst_Pos pos = { 0, 0, text };
    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, text->text_len + 40)) {
        Nst_error_add_pos(pos, pos);
        return false;
    }

    isize n = (isize)text->text_len - bom_size;
    u8 *text_p = (u8 *)text->text + bom_size;

    bool skip_line_feed = false;

    while (n > 0) {
        // Decode character
        i32 ch_len = from->check_bytes(text_p, n);
        if (ch_len < 0) {
            Nst_sb_destroy(&sb);
            Nst_error_setf_syntax(
                "could not encode byte %ib for %s encoding",
                *text_p, from->name);
            Nst_error_add_pos(pos, pos);
            return false;
        }
        usize ch_size = ch_len * from->ch_size;
        u32 utf32_ch = from->to_utf32(text_p);
        text_p += ch_size;
        n -= ch_len;

        if (utf32_ch == '\n' && !skip_line_feed) {
            pos.line++;
            pos.col = 0;
        }
        if (skip_line_feed)
            skip_line_feed = false;

        if (utf32_ch == '\r') {
            pos.line++;
            pos.col = 0;
            skip_line_feed = true;
        }

        // Re-encode character
        if (!Nst_sb_reserve(&sb, 5)) {
            Nst_sb_destroy(&sb);
            Nst_error_add_pos(pos, pos);
            return false;
        }
        ch_len = Nst_encoding_ext_utf8.from_utf32(utf32_ch, sb.value + sb.len);
        sb.len += ch_len;
        pos.col++;
    }
    sb.value[sb.len] = 0;

    Nst_free(text->text);
    text->text = (char *)sb.value;
    text->text_len = sb.len;
    return true;
}

static void parse_option(char *opt, i32 *opt_level, Nst_EncodingID *encoding,
                         bool *no_default)
{
    if (strcmp(opt, "-O0") == 0)
        *opt_level = 0;
    else if (strcmp(opt, "-O1") == 0)
        *opt_level = 1;
    else if (strcmp(opt, "-O2") == 0)
        *opt_level = 2;
    else if (strcmp(opt, "-O3") == 0)
        *opt_level = 3;
    else if (strcmp(opt, "--no-default") == 0)
        *no_default = true;
    else if (strncmp(opt, "--encoding=", 11) == 0) {
        Nst_EncodingID new_encoding = Nst_encoding_from_name(opt + 11);
        new_encoding = Nst_encoding_to_single_byte(new_encoding);
        if (new_encoding != Nst_EID_UNKNOWN)
            *encoding = new_encoding;
    }
}

static void parse_first_line(char *text, usize len, i32 *opt_level,
                             Nst_EncodingID *encoding, bool *no_default)
{
    i32 bom_size;
    Nst_check_bom(text, len, &bom_size);
    text += bom_size;
    len -= bom_size;

    // the first line must start with --$
    if (len < 3 || strncmp(text, "--$", 3) != 0)
        return;

    // max length: '--encoding=' + 15 characters for the value
    char curr_opt[27];
    usize i = 0;
    char ch = 0;
    text += 3;
    len -= 3;

    while ((ch = *text) != '\n' && *text != '\r' && len-- != 0) {
        if (ch != ' ') {
            if (i < 26)
                curr_opt[i] = ch;
            i++;
            continue;
        }
        if (i > 26 || i == 0) {
            i = 0;
            continue;
        }
        curr_opt[i] = '\0';
        parse_option(curr_opt, opt_level, encoding, no_default);
        i = 0;
    }

    if (i > 12 || i == 0)
        return;

    curr_opt[i] = '\0';
    parse_option(curr_opt, opt_level, encoding, no_default);
}
