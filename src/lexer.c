#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "hash.h"
#include "lexer.h"
#include "global_consts.h"
#include "tokens.h"
#include "interpreter.h" // Nst_get_full_path
#include "encoding.h"
#include "format.h"

#define START_CH_SIZE 8 * sizeof(i8)
#define CH_TO_HEX(c) (u8)((u8)(c) > '9' ? (u8)(c) - 'a' + 10 : (u8)(c) - '0')

#define SET_INVALID_ESCAPE_ERROR do {                                         \
    Nst_buffer_destroy(&buf);                                                 \
    Nst_set_internal_syntax_error_c(                                          \
        Nst_error_get(),                                                      \
        escape_start,                                                         \
        cursor.pos,                                                           \
        _Nst_EM_INVALID_ESCAPE);                                              \
    return;                                                                   \
    } while (0)

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

#define RETURN_IF_OP_ERR(cond) do {                                           \
    if ((cond) || Nst_error_occurred()) {                                     \
        Nst_error_add_positions(Nst_error_get(), cursor.pos, cursor.pos);     \
        return;                                                               \
    }                                                                         \
    } while (0)

#define STR_APPEND_TOK(tok, free_buf) do {                                    \
    if (tok == NULL) {                                                        \
        if (free_buf) Nst_buffer_destroy(&buf);                               \
        Nst_error_add_positions(Nst_error_get(), cursor.pos, cursor.pos);     \
        return;                                                               \
    }                                                                         \
    if (!Nst_llist_append(cursor.tokens, tok, true)) {                        \
        Nst_tok_destroy(tok);                                                 \
        if (free_buf) Nst_buffer_destroy(&buf);                               \
        Nst_error_add_positions(Nst_error_get(), cursor.pos, cursor.pos);     \
        return;                                                               \
    }                                                                         \
    } while (0)

typedef struct LexerCursor {
    i8 *text;
    usize len;
    Nst_Pos pos;
    i32 prev_line_len;
    i32 idx;
    i8 ch;
    Nst_LList *tokens;
} LexerCursor;

static LexerCursor cursor;

static inline void advance(void);
static inline void go_back(void);
static void make_symbol(Nst_Tok **tok);
static void make_num_literal(Nst_Tok **tok);
static void make_ident(Nst_Tok **tok);
static void make_str_literal(Nst_Tok **tok);
static void parse_first_line(i8 *text, usize len, i32 *opt_level,
                             Nst_CPID *encoding, bool *no_default);
static i32 find_fmt_str_inline_end(void);

Nst_LList *Nst_tokenizef(i8 *filename, Nst_CPID encoding, i32 *opt_level,
                         bool *no_default, Nst_SourceText *src_text)
{
    FILE *file = NULL;
    i8 *text = NULL;
    i8 *full_path = NULL;

    file = Nst_fopen_unicode(filename, "rb");

    if (file == NULL) {
        if (!Nst_error_occurred())
            Nst_set_value_errorf("File \"%.100s\" not found.", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    text = (i8 *)Nst_calloc(size + 1, sizeof(i8), NULL);
    if (text == NULL)
        goto cleanup;

    usize str_len = fread(text, sizeof(i8), size + 1, file);
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
            make_num_literal(&tok);
        } else if (CH_IS_SYMBOL(cursor.ch))
            make_symbol(&tok);
        else if (CH_IS_ALPHA(cursor.ch) || (u8)cursor.ch >= 0b10000000)
            make_ident(&tok);
        else if (cursor.ch == '"' || cursor.ch == '\'')
            make_str_literal(&tok);
        else if (cursor.ch == '\n') {
            tok = Nst_tok_new_noend(Nst_copy_pos(cursor.pos), Nst_TT_ENDL);
            if (tok == NULL) {
                Nst_error_add_positions(
                    Nst_error_get(),
                    cursor.pos,
                    cursor.pos);
            }
        } else if (cursor.ch == '\\')
            advance();
        else {
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                cursor.pos,
                cursor.pos,
                _Nst_EM_INVALID_CHAR);
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

Nst_LList *Nst_tokenize(Nst_SourceText *text)
{
    Nst_LList *tokens = Nst_llist_new();

    cursor.idx = -1;
    cursor.ch = ' ';
    cursor.len = text->text_len;
    cursor.text = text->text;
    cursor.pos.col = -1;
    cursor.pos.line = 0;
    cursor.pos.text = text;
    cursor.tokens = tokens;

    if (tokens == NULL) {
        Nst_error_add_positions(Nst_error_get(), cursor.pos, cursor.pos);
        return NULL;
    }

    advance();

    if (!tokenize_internal((i32)cursor.len)) {
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
        return NULL;
    }

    Nst_Tok *tok = Nst_tok_new_noend(cursor.pos, Nst_TT_EOFILE);
    if (tok == NULL) {
        Nst_error_add_positions(Nst_error_get(), cursor.pos, cursor.pos);
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
        return NULL;
    }

    if (!Nst_llist_append(tokens, tok, true)) {
        Nst_error_add_positions(Nst_error_get(), cursor.pos, cursor.pos);
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
        return NULL;
    }

    return tokens;
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

static void make_symbol(Nst_Tok **tok)
{
    Nst_Pos start = Nst_copy_pos(cursor.pos);
    i8 symbol[4] = { cursor.ch, 0, 0, 0 };
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
        return;
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
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                start,
                cursor.pos,
                _Nst_EM_OPEN_COMMENT);
        }

        return;
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

    *tok = Nst_tok_new_noval(start, Nst_copy_pos(cursor.pos), token_type);
    RETURN_IF_OP_ERR(*tok == NULL);
}

static void make_num_literal(Nst_Tok **tok)
{
    i8 *start_p = cursor.text + cursor.idx;
    Nst_Pos start = Nst_copy_pos(cursor.pos);
    Nst_Pos end = start;

    usize ltrl_size = 0;
    i8 *ltrl = NULL;
    bool neg = false;
    bool is_real = false;
    Nst_StrObj s;
    Nst_Obj *res;

    if (cursor.ch == '-' || cursor.ch == '+') {
        neg = cursor.ch == '-';
        advance();
        if (!CH_IS_DEC(cursor.ch)) {
            go_back();
            make_symbol(tok);
            return;
        }
        start_p++;
    }

    if (cursor.ch != '0')
        goto dec_num;

    advance();
    switch (cursor.ch) {
    case 'b':
    case 'B':
        advance();
        if (!CH_IS_BIN(cursor.ch)) {
            go_back();
            Nst_Obj *val = Nst_byte_new(0);
            RETURN_IF_OP_ERR(val == NULL);
            *tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, val);
            RETURN_IF_OP_ERR(*tok == NULL);
            return;
        }
        ltrl_size += 2;
        do {
            ltrl_size++;
            advance();
        } while (CH_IS_BIN(cursor.ch) || cursor.ch == '_');
        if (CH_IS_DEC(cursor.ch)) {
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        end = Nst_copy_pos(cursor.pos);
        goto end;
    case 'o':
    case 'O':
        advance();
        if (!CH_IS_OCT(cursor.ch)) {
            go_back();
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        ltrl_size += 2;
        do {
            ltrl_size++;
            advance();
        } while (CH_IS_OCT(cursor.ch) || cursor.ch == '_');
        if (CH_IS_DEC(cursor.ch)) {
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        end = Nst_copy_pos(cursor.pos);
        goto end;
    case 'x':
    case 'X':
        advance();
        if (!CH_IS_HEX(cursor.ch)) {
            go_back();
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        ltrl_size += 2;
        do {
            ltrl_size++;
            advance();
        } while (CH_IS_HEX(cursor.ch) || cursor.ch == '_');
        if (CH_IS_ALPHA(cursor.ch)) {
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        end = Nst_copy_pos(cursor.pos);
        goto end;
    case 'h':
    case 'H': {
        advance();
        if (!CH_IS_HEX(cursor.ch)) {
            go_back();
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                start,
                cursor.pos,
                _Nst_EM_BAD_BYTE_LITERAL);
            return;
        }
        i8 ch1 = 0;
        i8 ch2 = 0;
        do {
            if (cursor.ch == '_') {
                advance();
                continue;
            }
            ch1 = ch2;
            if (cursor.ch >= '0' && cursor.ch <= '9')
                ch2 = cursor.ch - '0';
            else if (cursor.ch >= 'a' && cursor.ch <= 'f')
                ch2 = cursor.ch - 'a' + 10;
            else
                ch2 = cursor.ch - 'A' + 10;
            advance();
        } while (CH_IS_HEX(cursor.ch) || cursor.ch == '_');
        if (CH_IS_ALPHA(cursor.ch)) {
            Nst_set_internal_syntax_error_c(
                Nst_error_get(),
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        i32 val = (i32)ch2 + (i32)ch1 * 16;
        if (neg)
            val = -val;
        Nst_Obj *obj = Nst_byte_new((u8)val);
        RETURN_IF_OP_ERR(obj == NULL);
        *tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, obj);
        RETURN_IF_OP_ERR(*tok == NULL);
        return;
    }
    default:
        go_back();
    }

dec_num:
    do {
        advance();
        ltrl_size++;
    } while (CH_IS_DEC(cursor.ch) || cursor.ch == '_');

    if (cursor.ch != '.') {
        go_back();
        goto end;
    }
    is_real = true;
    advance();
    ltrl_size++;

    if (!CH_IS_DEC(cursor.ch)) {
        Nst_set_internal_syntax_error_c(
            Nst_error_get(),
            start,
            cursor.pos,
            _Nst_EM_BAD_REAL_LITERAL);
        return;
    }
    do {
        advance();
        ltrl_size++;
    } while (CH_IS_DEC(cursor.ch) || cursor.ch == '_');

    if (cursor.ch != 'e' && cursor.ch != 'E') {
        go_back();
        goto end;
    }
    advance();
    ltrl_size++;
    if (cursor.ch == '+' || cursor.ch == '-') {
        advance();
        ltrl_size++;
    }
    if (!CH_IS_DEC(cursor.ch)) {
        Nst_set_internal_syntax_error_c(
            Nst_error_get(),
            start,
            cursor.pos,
            _Nst_EM_BAD_REAL_LITERAL);
        return;
    }
    do {
        advance();
        ltrl_size++;
    } while (CH_IS_DEC(cursor.ch) || cursor.ch == '_');
    go_back();

end:
    ltrl = Nst_malloc_c(ltrl_size + 3, i8);
    RETURN_IF_OP_ERR(ltrl == NULL);
    ltrl[0] = neg ? '-' : '+';
    memcpy(ltrl + 1, start_p, ltrl_size);
    ltrl[ltrl_size + 1] = '\0';
    s.value = ltrl;
    s.len = ltrl_size + 1;
    end = cursor.pos;

    if (is_real) {
        res = Nst_string_parse_real(&s);
        Nst_free(ltrl);
    } else {
        res = Nst_string_parse_int(&s, 0);
        advance();
        if (cursor.ch == 'b' || cursor.ch == 'B') {
            if (res == NULL
                && Nst_error_get()->error_name == Nst_s.e_MemoryError)
            {
                Nst_error_clear();
                ltrl[ltrl_size + 1] = 'b';
                ltrl[ltrl_size + 2] = '\0';
                s.len++;
                res = Nst_string_parse_byte(&s);
                Nst_free(ltrl);
            } else {
                Nst_free(ltrl);
                RETURN_IF_OP_ERR(res == NULL);
                Nst_Obj *new_res = Nst_byte_new(AS_INT(res) & 0xff);
                Nst_dec_ref(res);
                res = new_res;
            }
            end = cursor.pos;
        } else {
            Nst_free(ltrl);
            go_back();
        }
    }

    RETURN_IF_OP_ERR(res == NULL);
    *tok = Nst_tok_new_value(start, end, Nst_TT_VALUE, res);
    RETURN_IF_OP_ERR(*tok == NULL);
}

static void make_ident(Nst_Tok **tok)
{
    Nst_Pos start = Nst_copy_pos(cursor.pos);
    i8 *str;
    i8 *str_start = cursor.text + cursor.idx;
    usize str_len = 0;

    while (cursor.idx < (i32)cursor.len
           && (CH_IS_ALPHA(cursor.ch)
               || CH_IS_DEC(cursor.ch)
               || (u8)cursor.ch >= 0b10000000))
    {
        i32 res = Nst_check_utf8_bytes(
            (u8 *)cursor.text + cursor.idx,
            (usize)(cursor.text - cursor.idx));
        str_len += res;
        for (i32 i = 0; i < res; i++)
            advance();
    }
    go_back();

    str = Nst_malloc_c(str_len + 1, i8);
    RETURN_IF_OP_ERR(str == NULL);

    memcpy(str, str_start, str_len);
    str[str_len] = '\0';

    Nst_Pos end = Nst_copy_pos(cursor.pos);
    Nst_StrObj *val_obj = STR(Nst_string_new_c_raw(str, true));
    if (val_obj == NULL) {
        Nst_free(str);
        Nst_error_add_positions(Nst_error_get(), cursor.pos, cursor.pos);
    }
    Nst_obj_hash(OBJ(val_obj));

    *tok = Nst_tok_new_value(start, end, Nst_TT_IDENT, OBJ(val_obj));
    RETURN_IF_OP_ERR(val_obj == NULL);
}

static void make_str_literal(Nst_Tok **tok)
{
    Nst_Pos start = Nst_copy_pos(cursor.pos);
    Nst_Pos escape_start = Nst_copy_pos(cursor.pos);
    i8 closing_ch = cursor.ch;
    bool allow_multiline = cursor.ch == '"';
    bool escape = false;
    bool is_format_string = false;
    Nst_Tok *fmt_tok;

    Nst_Buffer buf;
    RETURN_IF_OP_ERR(!Nst_buffer_init(&buf, START_CH_SIZE));

    advance(); // still on the opening character

    // while there is text to add and (the string has not ended or
    // the end is inside and escape)
    while (cursor.idx < (i32) cursor.len
           && (cursor.ch != closing_ch || escape))
    {
        if (!Nst_buffer_expand_by(&buf, 4)) {
            Nst_buffer_destroy(&buf);
            Nst_error_add_positions(Nst_error_get(), cursor.pos, cursor.pos);
        }

        if (!escape) {
            if (cursor.ch == '\n' && !allow_multiline) {
                Nst_buffer_destroy(&buf);
                Nst_set_internal_syntax_error_c(
                    Nst_error_get(),
                    cursor.pos,
                    cursor.pos,
                    _Nst_EM_UNEXPECTED_NEWLINE);
                return;
            } else if (cursor.ch == '\\') {
                escape = true;
                escape_start = Nst_copy_pos(cursor.pos);
            } else
                Nst_buffer_append_char(&buf, cursor.ch);
            advance();
            continue;
        }

        // If there is an escape sequence
        switch (cursor.ch) {
        case '\'':Nst_buffer_append_char(&buf, '\''); break;
        case '"': Nst_buffer_append_char(&buf, '"' ); break;
        case '\\':Nst_buffer_append_char(&buf, '\\'); break;
        case 'a': Nst_buffer_append_char(&buf, '\a'); break;
        case 'b': Nst_buffer_append_char(&buf, '\b'); break;
        case 'e': Nst_buffer_append_char(&buf,'\x1b');break;
        case 'f': Nst_buffer_append_char(&buf, '\f'); break;
        case 'n': Nst_buffer_append_char(&buf, '\n'); break;
        case 'r': Nst_buffer_append_char(&buf, '\r'); break;
        case 't': Nst_buffer_append_char(&buf, '\t'); break;
        case 'v': Nst_buffer_append_char(&buf, '\v'); break;
        case 'x': {
            if ((usize)cursor.idx + 2 >= cursor.len)
                SET_INVALID_ESCAPE_ERROR;

            advance();
            i8 ch1 = (i8)tolower(cursor.ch);
            advance();
            i8 ch2 = (i8)tolower(cursor.ch);

            if (!CH_IS_HEX(ch1) || !CH_IS_HEX(ch2))
                SET_INVALID_ESCAPE_ERROR;

            u8 result = (CH_TO_HEX(ch1) << 4) + CH_TO_HEX(ch2);
            if (result >= 0x80) {
                i8 utf8_b1 = 0b11000000 | (i8)(result >> 6);
                i8 utf8_b2 = 0b10000000 | (i8)(result & 0x3f);
                Nst_buffer_append_char(&buf, utf8_b1);
                Nst_buffer_append_char(&buf, utf8_b2);
            } else
                Nst_buffer_append_char(&buf, (i8)result);
            break;
        }
        case 'u':
        case 'U': {
            i32 size = cursor.ch == 'U' ? 6 : 4;
            if ((usize)cursor.idx + size >= cursor.len)
                SET_INVALID_ESCAPE_ERROR;

            i32 num = 0;
            for (i32 i = 0; i < size; i++) {
                advance();
                i8 ch = (i8)tolower(cursor.ch);
                if (!CH_IS_HEX(ch))
                    SET_INVALID_ESCAPE_ERROR;

                ch = (i8)CH_TO_HEX(ch);
                num += ch << (4 * (size - i - 1));
            }

            i8 unicode_char[5] = { 0 };

            if (num > 0x10ffff)
                SET_INVALID_ESCAPE_ERROR;

            Nst_ext_utf8_from_utf32(num, (u8 *)unicode_char);
            Nst_buffer_append_c_str(&buf, (const i8 *)unicode_char);
            break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            i8 ch1 = cursor.ch - '0';

            advance();
            if (!CH_IS_OCT(cursor.ch)) {
                Nst_buffer_append_char(&buf, ch1);
                go_back();
                break;
            }
            i8 ch2 = cursor.ch - '0';

            advance();
            if (!CH_IS_OCT(cursor.ch)) {
                Nst_buffer_append_char(&buf, (ch1 << 3) + ch2);
                go_back();
                break;
            }
            i8 ch3 = cursor.ch - '0';
            u16 result = (ch1 << 6) + (ch2 << 3) + ch3;

            if (result >= 0x80) {
                i8 utf8_b1 = 0b11000000 | (i8)(result >> 6);
                i8 utf8_b2 = 0b10000000 | (i8)(result & 0x3f);
                Nst_buffer_append_char(&buf, utf8_b1);
                Nst_buffer_append_char(&buf, utf8_b2);
            } else
                Nst_buffer_append_char(&buf, (i8)result);
            break;
        }
        case '(': {
            i32 max_idx = find_fmt_str_inline_end();
            advance();

            if (max_idx == -1) {
                Nst_buffer_destroy(&buf);
                Nst_set_internal_syntax_error_c(
                    Nst_error_get(),
                    cursor.pos,
                    cursor.pos,
                    "invalid format string");
                return;
            }

            if (!is_format_string) {
                fmt_tok = Nst_tok_new_noend(cursor.pos, Nst_TT_L_PAREN);
                STR_APPEND_TOK(fmt_tok, true);
                is_format_string = true;
            }

            Nst_Obj *val_obj = OBJ(Nst_buffer_to_string(&buf));
            RETURN_IF_OP_ERR(val_obj == NULL);
            Nst_obj_hash(val_obj);

            fmt_tok = Nst_tok_new_value(
                start, cursor.pos,
                Nst_TT_VALUE,
                val_obj);
            STR_APPEND_TOK(fmt_tok, false);

            fmt_tok = Nst_tok_new_noend(cursor.pos, Nst_TT_L_PAREN);
            STR_APPEND_TOK(fmt_tok, false);

            if (!tokenize_internal(max_idx))
                return;

            fmt_tok = Nst_tok_new_noend(cursor.pos, Nst_TT_R_PAREN);
            STR_APPEND_TOK(fmt_tok, false);

            if (!Nst_buffer_init(&buf, START_CH_SIZE)) {
                Nst_error_add_positions(
                    Nst_error_get(),
                    cursor.pos, cursor.pos);
                return;
            }
            break;
        }
        default:
            SET_INVALID_ESCAPE_ERROR;
        }

        escape = false;
        advance();
    }

    go_back();
    Nst_Pos error_end = Nst_copy_pos(cursor.pos);
    advance();

    if (cursor.ch != closing_ch) {
        Nst_buffer_destroy(&buf);
        Nst_set_internal_syntax_error_c(
            Nst_error_get(),
            start,
            error_end,
            _Nst_EM_OPEN_STR_LITERAL);
        return;
    }

    Nst_Obj *val_obj = OBJ(Nst_buffer_to_string(&buf));
    RETURN_IF_OP_ERR(val_obj == NULL);
    Nst_obj_hash(val_obj);

    if (!is_format_string) {
        *tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, val_obj);
        RETURN_IF_OP_ERR(*tok == NULL);
        return;
    }
    if (STR(val_obj)->len == 0)
        Nst_dec_ref(val_obj);
    else {
        fmt_tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, val_obj);
        STR_APPEND_TOK(fmt_tok, false);
    }
    fmt_tok = Nst_tok_new_noend(cursor.pos, Nst_TT_CONCAT);
    STR_APPEND_TOK(fmt_tok, false);
    *tok = Nst_tok_new_noend(cursor.pos, Nst_TT_R_PAREN);
}

bool Nst_add_lines(Nst_SourceText *text)
{
    i8 *text_p = text->text;
    i8 **starts = (i8 **)Nst_calloc(100, sizeof(i8 *), NULL);
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
            void *temp = Nst_realloc(starts, i + 100, sizeof(i8*), 0);
            if (temp == NULL) {
                Nst_free(starts);
                text->lines = NULL;
                text->lines_len = 0;
                return false;
            }
            starts = (i8 **)temp;
        }

        starts[line_count - 1] = text_p + i + 1;
    }

    text->lines = starts;
    text->lines_len = line_count;
    return true;
}

bool Nst_normalize_encoding(Nst_SourceText *text, Nst_CPID encoding)
{
    i32 bom_size = 0;
    if (encoding == Nst_CP_UNKNOWN)
        encoding = Nst_detect_encoding(text->text, text->text_len, &bom_size);
    else
        Nst_check_bom(text->text, text->text_len, &bom_size);

    encoding = Nst_single_byte_cp(encoding);
    Nst_CP *from = Nst_cp(encoding);

    Nst_Pos pos = { 0, 0, text };
    Nst_Buffer buf;
    if (!Nst_buffer_init(&buf, text->text_len + 40)) {
        Nst_error_add_positions(Nst_error_get(), pos, pos);
        return false;
    }

    isize n = (isize)text->text_len - bom_size;
    u8 *text_p = (u8 *)text->text + bom_size;

    bool skip_line_feed = false;

    while (n > 0) {
        // Decode character
        i32 ch_len = from->check_bytes(text_p, n);
        if (ch_len < 0) {
            Nst_buffer_destroy(&buf);
            Nst_set_internal_syntax_error(
                Nst_error_get(),
                pos, pos,
                STR(Nst_sprintf(
                    _Nst_EM_INVALID_ENCODING,
                    *text_p, from->name)));
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
        if (!Nst_buffer_expand_by(&buf, 5)) {
            Nst_buffer_destroy(&buf);
            Nst_error_add_positions(Nst_error_get(), pos, pos);
            return false;
        }
        ch_len = Nst_cp_ext_utf8.from_utf32(utf32_ch, buf.data + buf.len);
        buf.len += ch_len;
        pos.col++;
    }
    buf.data[buf.len] = 0;

    Nst_free(text->text);
    text->text = buf.data;
    text->text_len = buf.len;
    return true;
}

static void parse_first_line(i8 *text, usize len, i32 *opt_level,
                             Nst_CPID *encoding, bool *no_default)
{
    i32 bom_size;
    Nst_check_bom(text, len, &bom_size);
    text += bom_size;
    len -= bom_size;

    // the first line must start with --$
    if (len < 3 || text[0] != '-' || text[1] != '-' || text[2] != '$')
        return;

    // max length: --encoding + '=' + 15 characters for the value
    i8 curr_opt[27];
    usize i = 0;
    text += 3;
    len -= 3;

    while (*text != '\n' && *text != '\r' && len != 0) {
        if (*text != ' ') {
            if (i < 26)
                curr_opt[i] = *text;
            i++;
            text++;
            len--;
            continue;
        }
        text++;
        len--;

        if (i > 26 || i == 0) {
            i = 0;
            continue;
        }
        curr_opt[i] = '\0';

        if (strcmp(curr_opt, "-O0") == 0)
            *opt_level = 0;
        else if (strcmp(curr_opt, "-O1") == 0)
            *opt_level = 1;
        else if (strcmp(curr_opt, "-O2") == 0)
            *opt_level = 2;
        else if (strcmp(curr_opt, "-O3") == 0)
            *opt_level = 3;
        else if (strcmp(curr_opt, "--no-default") == 0)
            *no_default = true;
        else if (strncmp(curr_opt, "--encoding", 10) == 0) {
            if (curr_opt[11] != '=') {
                i = 0;
                continue;
            }
            Nst_CPID new_encoding = Nst_encoding_from_name(curr_opt + 11);
            new_encoding = Nst_single_byte_cp(new_encoding);
            if (new_encoding != Nst_CP_UNKNOWN)
                *encoding = new_encoding;
        }
        i = 0;
    }

    if (i > 12 || i == 0) {
        i = 0;
        return;
    }
    curr_opt[i] = '\0';

    if (strcmp(curr_opt, "-O0") == 0)
        *opt_level = 0;
    else if (strcmp(curr_opt, "-O1") == 0)
        *opt_level = 1;
    else if (strcmp(curr_opt, "-O2") == 0)
        *opt_level = 2;
    else if (strcmp(curr_opt, "-O3") == 0)
        *opt_level = 3;
    else if (strcmp(curr_opt, "--no-default") == 0)
        *no_default = true;
    else if (strncmp(curr_opt, "--encoding", 10) == 0) {
        if (curr_opt[11] != '=')
            return;
        Nst_CPID new_encoding = Nst_encoding_from_name(curr_opt + 11);
        new_encoding = Nst_single_byte_cp(new_encoding);
        if (new_encoding != Nst_CP_UNKNOWN)
            *encoding = new_encoding;
    }
}

static bool skip_inline_str(void)
{
    i8 closing_char = cursor.ch;
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
