#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "error_internal.h"
#include "hash.h"
#include "lexer.h"
#include "global_consts.h"
#include "tokens.h"
#include "interpreter.h" // Nst_get_full_path
#include "encoding.h"
#include "format.h"

#define START_CH_SIZE 8 * sizeof(i8)

#define SET_INVALID_ESCAPE_ERROR do { \
    Nst_buffer_destroy(&buf); \
    _Nst_SET_RAW_SYNTAX_ERROR( \
        error, \
        escape_start, \
        cursor.pos, \
        _Nst_EM_INVALID_ESCAPE); \
    return; } while (0)

#define CUR_AT_END (cursor.idx >= (i32)cursor.len)

#define CH_IS_DEC(ch) (ch >= '0' && ch <= '9')
#define CH_IS_BIN(ch) (ch == '0' || ch == '1')
#define CH_IS_OCT(ch) (ch >= '0' && ch <= '7')
#define CH_IS_HEX(ch) ((ch >= '0' && ch <= '9') || \
                       (ch >= 'a' && ch <= 'f') || \
                       (ch >= 'A' && ch <= 'F') )

#define CH_IS_ALPHA(ch) ((ch >= 'a' && ch <= 'z') || \
                         (ch >= 'A' && ch <= 'Z') || \
                          ch == '_' )

#define CH_IS_SYMBOL(ch) ((ch >= ':' && ch <= '@') || \
                          (ch >= '{' && ch <= '~') || \
                          (ch >= '(' && ch <= '/') || \
                          (ch >= '#' && ch <= '&') || \
                           ch == '!' || ch == '['  || \
                           ch == ']' || ch == '^' )

#define SET_ERROR_IF_OP_ERR(cond) do { \
    if ( (cond) || Nst_error_occurred() ) \
    { \
        _Nst_SET_ERROR_FROM_OP_ERR(error, cursor.pos, cursor.pos); \
        return; \
    } } while ( 0 )

typedef struct LexerCursor {
    i8 *text;
    usize len;
    Nst_Pos pos;
    i32 prev_line_len;
    i32 idx;
    i8 ch;
} LexerCursor;

static LexerCursor cursor;

static inline void advance();
static inline void go_back();
static void make_symbol(Nst_Tok **tok, Nst_Error *error);
static void make_num_literal(Nst_Tok **tok, Nst_Error *error);
static void make_ident(Nst_Tok **tok, Nst_Error *error);
static void make_str_literal(Nst_Tok **tok, Nst_Error *error);
static void parse_first_line(i8       *text,
                             usize     len,
                             i32      *opt_level,
                             Nst_CPID *encoding,
                             bool     *no_default);

Nst_LList *Nst_tokenizef(i8             *filename,
                         Nst_CPID        encoding,
                         i32            *opt_level,
                         bool           *no_default,
                         Nst_SourceText *src_text,
                         Nst_Error      *error)
{
    *opt_level = 3;
    *no_default = false;

    FILE *file = fopen(filename, "rb");
    if ( file == NULL )
    {
        Nst_fprint(Nst_io.err, "File \"");
        Nst_fprint(Nst_io.err, (const i8 *)filename);
        Nst_fprintln(Nst_io.err, "\" not found");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    i8 *text = (i8 *)Nst_raw_calloc(size + 1, sizeof(i8));
    if ( text == NULL )
    {
        Nst_fprint(Nst_io.err, "Memory allocation failed\n");
        return NULL;
    }

    usize str_len = fread(text, sizeof(i8), size + 1, file);
    fclose(file);
    text[str_len] = '\0';

    i8 *full_path;
    Nst_get_full_path(filename, &full_path, NULL);
    if ( full_path == NULL )
    {
        Nst_error_clear();
        Nst_fprint(Nst_io.err, "Memory allocation failed\n");
        return NULL;
    }

    src_text->text = text;
    src_text->len = str_len;
    src_text->path = full_path;

    parse_first_line(text, str_len, opt_level, &encoding, no_default);

    bool result = Nst_normalize_encoding(src_text, encoding, error);
    Nst_add_lines(src_text);
    if ( !result || error->occurred )
    {
        return NULL;
    }
    Nst_LList *tokens = Nst_tokenize(src_text, error);
    return tokens;
}

Nst_LList *Nst_tokenize(Nst_SourceText *text, Nst_Error *error)
{
    Nst_Tok *tok = NULL;
    Nst_LList *tokens = Nst_llist_new();

    cursor.idx = -1;
    cursor.ch = ' ';
    cursor.len = text->len;
    cursor.text = text->text;
    cursor.pos.col = -1;
    cursor.pos.line = 0;
    cursor.pos.text = text;

    if (tokens == NULL)
    {
        _Nst_SET_ERROR_FROM_OP_ERR(error, cursor.pos, cursor.pos);
        return NULL;
    }

    advance();

    while ( !CUR_AT_END )
    {
        if ( cursor.ch == ' ' || cursor.ch == '\t' )
        {
            advance();
            continue;
        }
        else if ( CH_IS_DEC(cursor.ch) ||
                  cursor.ch == '+'     ||
                  cursor.ch == '-' )
        {
            make_num_literal(&tok, error);
        }
        else if ( CH_IS_SYMBOL(cursor.ch) )
        {
            make_symbol(&tok, error);
        }
        else if ( CH_IS_ALPHA(cursor.ch) || (u8)cursor.ch >= 0b10000000 )
        {
            make_ident(&tok, error);
        }
        else if ( cursor.ch == '"' || cursor.ch == '\'' )
        {
            make_str_literal(&tok, error);
        }
        else if ( cursor.ch == '\n' )
        {
            tok = Nst_tok_new_noend(Nst_copy_pos(cursor.pos), Nst_TT_ENDL);
            if (tokens == NULL)
            {
                _Nst_SET_ERROR_FROM_OP_ERR(error, cursor.pos, cursor.pos);
            }
        }
        else if ( cursor.ch == '\\' )
        {
            advance();
        }
        else
        {
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
                cursor.pos,
                cursor.pos,
                _Nst_EM_INVALID_CHAR);
        }

        if ( error->occurred )
        {
            if ( tok != NULL )
            {
                Nst_token_destroy(tok);
            }
            Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
            return NULL;
        }

        if ( tok != NULL )
        {
            Nst_llist_append(tokens, tok, true);
        }
        tok = NULL;
        advance();
    }

    tok = Nst_tok_new_noend(cursor.pos, Nst_TT_EOFILE);
    if ( tok == NULL )
    {
        _Nst_SET_ERROR_FROM_OP_ERR(error, cursor.pos, cursor.pos);
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
        return NULL;
    }

    if ( !Nst_llist_append(tokens, tok, true) )
    {
        _Nst_SET_ERROR_FROM_OP_ERR(error, cursor.pos, cursor.pos);
        Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
        return NULL;
    }

    return tokens;
}

inline static void advance()
{
    cursor.idx++;
    cursor.pos.col++;

    if ( cursor.idx > (i32)cursor.len )
    {
        cursor.ch = '\0';
        return;
    }

    if ( cursor.ch == '\n' )
    {
        cursor.prev_line_len = cursor.pos.col;
        cursor.pos.col = 0;
        cursor.pos.line++;
    }

    cursor.ch = cursor.text[cursor.idx];
}

inline static void go_back()
{
    cursor.idx--;
    cursor.pos.col--;

    if (cursor.idx < 0)
        return;

    cursor.ch = cursor.text[cursor.idx];
    if ( cursor.ch == '\n' )
    {
        cursor.pos.line--;
        cursor.pos.col = cursor.prev_line_len;
    }
}

static void make_symbol(Nst_Tok **tok, Nst_Error *error)
{
    Nst_Pos start = Nst_copy_pos(cursor.pos);
    i8 symbol[4] = { cursor.ch, 0, 0, 0 };
    advance();
    if ( !CUR_AT_END && CH_IS_SYMBOL(cursor.ch) )
    {
        symbol[1] = cursor.ch;
        advance();

        if ( !CUR_AT_END && CH_IS_SYMBOL(cursor.ch) )
        {
            symbol[2] = cursor.ch;
        }
        else
        {
            go_back();
        }
    }
    else
    {
        go_back();
    }

    if ( symbol[0] == '-' && symbol[1] == '-' )
    {
        while ( !CUR_AT_END && cursor.ch != '\n' )
        {
            if ( cursor.ch == '\\' )
            {
                advance();

                // Allows an even number of \ to escape the new line
                if ( cursor.ch == '\\' )
                {
                    go_back();
                }
            }
            advance();
        }
        go_back();
        return;
    }
    else if ( symbol[0] == '-' && symbol[1] == '/' )
    {
        bool can_close = false;
        bool was_closed = false;

        while ( !CUR_AT_END )
        {
            if ( can_close && cursor.ch == '-' )
            {
                advance();
                was_closed = true;
                break;
            }

            can_close = cursor.ch == '/';
            advance();
        }

        go_back();

        if ( !was_closed )
        {
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _Nst_EM_OPEN_COMMENT);
        }

        return;
    }

    if ( symbol[1] == '-' && symbol[2] == '-' )
    {
        symbol[1] = '\0';
        symbol[2] = '\0';
        go_back();
        go_back();
    }
    else if ( symbol[1] == '-' && symbol[2] == '/' )
    {
        symbol[1] = '\0';
        symbol[2] = '\0';
        go_back();
        go_back();
    }
    // A '-' at the end might start a comment, il can be checked only when it
    // is at index 0 or 1
    else if ( symbol[2] == '-' )
    {
        symbol[2] = '\0';
        go_back();
    }

    Nst_TokType token_type = Nst_tok_from_str(symbol);

    while ( token_type == -1 )
    {
        go_back();

        if ( symbol[2] != '\0' )
        {
            symbol[2] = '\0';
        }
        else if ( symbol[1] != '\0' )
        {
            symbol[1] = '\0';
        }

        token_type = Nst_tok_from_str(symbol);
    }

    *tok = Nst_tok_new_noval(start, Nst_copy_pos(cursor.pos), token_type);
    SET_ERROR_IF_OP_ERR(*tok == NULL);
}

static void make_num_literal(Nst_Tok **tok, Nst_Error *error)
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

    if ( cursor.ch == '-' || cursor.ch == '+' )
    {
        neg = cursor.ch == '-';
        advance();
        if ( !CH_IS_DEC(cursor.ch) )
        {
            go_back();
            make_symbol(tok, error);
            return;
        }
        start_p++;
    }

    if ( cursor.ch != '0' )
    {
        goto dec_num;
    }

    advance();
    switch ( cursor.ch )
    {
    case 'b':
    case 'B':
        advance();
        if ( !CH_IS_BIN(cursor.ch) )
        {
            go_back();
            Nst_Obj *val = Nst_byte_new(0);
            SET_ERROR_IF_OP_ERR(val == NULL);
            *tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, val);
            SET_ERROR_IF_OP_ERR(*tok == NULL);
            return;
        }
        ltrl_size += 2;
        do
        {
            ltrl_size++;
            advance();
        }
        while ( CH_IS_BIN(cursor.ch) || cursor.ch == '_' );
        if ( CH_IS_DEC(cursor.ch) )
        {
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
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
        if ( !CH_IS_OCT(cursor.ch) )
        {
            go_back();
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        ltrl_size += 2;
        do
        {
            ltrl_size++;
            advance();
        }
        while ( CH_IS_OCT(cursor.ch) || cursor.ch == '_' );
        if ( CH_IS_DEC(cursor.ch) )
        {
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
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
        if ( !CH_IS_HEX(cursor.ch) )
        {
            go_back();
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        ltrl_size += 2;
        do
        {
            ltrl_size++;
            advance();
        }
        while ( CH_IS_HEX(cursor.ch) || cursor.ch == '_' );
        if ( CH_IS_ALPHA(cursor.ch) )
        {
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        end = Nst_copy_pos(cursor.pos);
        goto end;
    case 'h':
    case 'H':
    {
        advance();
        if ( !CH_IS_HEX(cursor.ch) )
        {
            go_back();
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _Nst_EM_BAD_BYTE_LITERAL);
            return;
        }
        i8 ch1 = 0;
        i8 ch2 = 0;
        do
        {
            if ( cursor.ch == '_' )
            {
                advance();
                continue;
            }
            ch1 = ch2;
            if ( cursor.ch >= '0' && cursor.ch <= '9' )
            {
                ch2 = cursor.ch - '0';
            }
            else if ( cursor.ch >= 'a' && cursor.ch <= 'f' )
            {
                ch2 = cursor.ch - 'a' + 10;
            }
            else
            {
                ch2 = cursor.ch - 'A' + 10;
            }
            advance();
        }
        while ( CH_IS_HEX(cursor.ch) || cursor.ch == '_' );
        if ( CH_IS_ALPHA(cursor.ch) )
        {
            _Nst_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _Nst_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        i32 val = (i32)ch2 + (i32)ch1 * 16;
        if ( neg )
        {
            val = -val;
        }
        Nst_Obj *obj = Nst_byte_new(0);
        SET_ERROR_IF_OP_ERR(obj == NULL);
        *tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, obj);
        SET_ERROR_IF_OP_ERR(*tok == NULL);
        return;
    }
    default:
        go_back();
    }

dec_num:
    do
    {
        advance();
        ltrl_size++;
    }
    while ( CH_IS_DEC(cursor.ch) || cursor.ch == '_' );

    if ( cursor.ch != '.' )
    {
        go_back();
        goto end;
    }
    is_real = true;
    advance();
    ltrl_size++;

    if ( !CH_IS_DEC(cursor.ch) )
    {
        _Nst_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            cursor.pos,
            _Nst_EM_BAD_REAL_LITERAL);
        return;
    }
    do
    {
        advance();
        ltrl_size++;
    }
    while ( CH_IS_DEC(cursor.ch) || cursor.ch == '_' );

    if ( cursor.ch != 'e' && cursor.ch != 'E' )
    {
        go_back();
        goto end;
    }
    advance();
    ltrl_size++;
    if ( cursor.ch == '+' || cursor.ch == '-' )
    {
        advance();
        ltrl_size++;
    }
    if ( !CH_IS_DEC(cursor.ch) )
    {
        _Nst_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            cursor.pos,
            _Nst_EM_BAD_REAL_LITERAL);
        return;
    }
    do
    {
        advance();
        ltrl_size++;
    }
    while ( CH_IS_DEC(cursor.ch) || cursor.ch == '_' );
    go_back();

end:
    ltrl = Nst_malloc_c(ltrl_size + 3, i8);
    SET_ERROR_IF_OP_ERR(ltrl == NULL);
    ltrl[0] = neg ? '-' : '+';
    memcpy(ltrl + 1, start_p, ltrl_size);
    ltrl[ltrl_size + 1] = '\0';
    s.value = ltrl;
    s.len = ltrl_size + 1;
    end = cursor.pos;

    if ( is_real )
    {
        res = Nst_string_parse_real(&s);
        Nst_free(ltrl);
    }
    else
    {
        res = Nst_string_parse_int(&s, 0);
        advance();
        if ( cursor.ch == 'b' || cursor.ch == 'B' )
        {
            if ( res == NULL && Nst_error_get()->name == Nst_s.e_MemoryError )
            {
                Nst_error_clear();
                ltrl[ltrl_size + 1] = 'b';
                ltrl[ltrl_size + 2] = '\0';
                s.len++;
                res = Nst_string_parse_byte(&s);
                Nst_free(ltrl);
            }
            else
            {
                Nst_free(ltrl);
                SET_ERROR_IF_OP_ERR(res == NULL);
                Nst_Obj *new_res = Nst_byte_new(AS_INT(res) & 0xff);
                Nst_dec_ref(res);
                res = new_res;
            }
            end = cursor.pos;
        }
        else
        {
            Nst_free(ltrl);
            go_back();
        }
    }

    SET_ERROR_IF_OP_ERR(res == NULL);
    *tok = Nst_tok_new_value(start, end, Nst_TT_VALUE, res);
    SET_ERROR_IF_OP_ERR(*tok == NULL);
}

static void make_ident(Nst_Tok **tok, Nst_Error *error)
{
    Nst_Pos start = Nst_copy_pos(cursor.pos);

    i8 *str;
    i8 *str_start = cursor.text + cursor.idx;
    usize str_len = 0;

    while ( cursor.idx < (i32)cursor.len &&
            (CH_IS_ALPHA(cursor.ch) ||
             CH_IS_DEC(cursor.ch)   ||
             (u8)cursor.ch >= 0b10000000) )
    {
        i32 res = Nst_check_utf8_bytes(
            (u8 *)cursor.text + cursor.idx,
            (usize)(cursor.text - cursor.idx));
        str_len += res;
        for ( i32 i = 0; i < res; i++ )
        {
            advance();
        }
    }
    go_back();

    str = Nst_malloc_c(str_len + 1, i8);
    SET_ERROR_IF_OP_ERR(str == NULL);

    memcpy(str, str_start, str_len);
    str[str_len] = '\0';

    Nst_Pos end = Nst_copy_pos(cursor.pos);
    Nst_StrObj *val_obj = STR(Nst_string_new_c_raw(str, true));
    if ( val_obj == NULL )
    {
        Nst_free(str);
        _Nst_SET_ERROR_FROM_OP_ERR(error, cursor.pos, cursor.pos);
    }
    Nst_obj_hash(OBJ(val_obj));

    *tok = Nst_tok_new_value(start, end, Nst_TT_IDENT, OBJ(val_obj));
    SET_ERROR_IF_OP_ERR(val_obj == NULL);
}

static void make_str_literal(Nst_Tok **tok, Nst_Error *error)
{
    Nst_Pos start = Nst_copy_pos(cursor.pos);
    Nst_Pos escape_start = Nst_copy_pos(cursor.pos);
    i8 closing_ch = cursor.ch;
    bool allow_multiline = cursor.ch == '"';
    bool escape = false;

    Nst_Buffer buf;
    SET_ERROR_IF_OP_ERR(!Nst_buffer_init(&buf, START_CH_SIZE));

    advance(); // still on the opening character

    // while there is text to add and (the string has not ended or
    // the end is inside and escape)
    while ( cursor.idx < (i32) cursor.len &&
            (cursor.ch != closing_ch || escape) )
    {
        if ( !Nst_buffer_expand_by(&buf, 4) )
        {
            Nst_buffer_destroy(&buf);
            _Nst_SET_ERROR_FROM_OP_ERR(error, cursor.pos, cursor.pos);
        }

        if ( !escape )
        {
            if ( cursor.ch == '\n' && !allow_multiline )
            {
                Nst_buffer_destroy(&buf);
                _Nst_SET_RAW_SYNTAX_ERROR(
                    error,
                    cursor.pos,
                    cursor.pos,
                    _Nst_EM_UNEXPECTED_NEWLINE);
                return;
            }
            else if ( cursor.ch == '\\' )
            {
                escape = true;
                escape_start = Nst_copy_pos(cursor.pos);
            }
            else
            {
                Nst_buffer_append_char(&buf, cursor.ch);
            }
            advance();
            continue;
        }

        // If there is an escape sequence
        switch ( cursor.ch )
        {
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
        case 'x':
        {
            if ( (usize)cursor.idx + 2 >= cursor.len )
            {
                SET_INVALID_ESCAPE_ERROR;
            }

            advance();
            i8 ch1 = (i8)tolower(cursor.ch);
            advance();
            i8 ch2 = (i8)tolower(cursor.ch);

            if ( !CH_IS_HEX(ch1) || !CH_IS_HEX(ch2) )
            {
                SET_INVALID_ESCAPE_ERROR;
            }

            i8 result = ((ch1 > '9' ? ch1 - 'a' + 10 : ch1 - '0') << 4) +
                            (ch2 > '9' ? ch2 - 'a' + 10 : ch2 - '0');

            Nst_buffer_append_char(&buf, result);
            break;
        }
        case 'u':
        case 'U':
        {
            i32 size = cursor.ch == 'U' ? 8 : 4;
            if ( (usize)cursor.idx + size >= cursor.len )
            {
                SET_INVALID_ESCAPE_ERROR;
            }

            i32 num = 0;
            for ( i32 i = 0; i < size; i++ )
            {
                advance();
                i8 ch = (i8)tolower(cursor.ch);
                if ( !CH_IS_HEX(ch) )
                {
                    SET_INVALID_ESCAPE_ERROR;
                }

                ch -= ch > '9' ? 'a' - 10 : '0';
                num += ch << (4 * (size - i - 1));
            }

            i8 unicode_char[5] = { 0 };

            if ( num <= 0x10ffff )
            {
                Nst_utf8_from_utf32(num, (u8 *)unicode_char);
            }
            else
            {
                SET_INVALID_ESCAPE_ERROR;
            }
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
        case '7':
        {
            i8 ch1 = cursor.ch - '0';

            advance();
            if ( !CH_IS_OCT(cursor.ch) )
            {
                Nst_buffer_append_char(&buf, ch1);
                go_back();
                break;
            }
            i8 ch2 = cursor.ch - '0';

            advance();
            if ( !CH_IS_OCT(cursor.ch) )
            {
                Nst_buffer_append_char(&buf, (ch1 << 3) + ch2);
                go_back();
                break;
            }
            i8 ch3 = cursor.ch - '0';
            Nst_buffer_append_char(&buf, (ch1 << 6) + (ch2 << 3) + ch3);
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

    if ( cursor.ch != closing_ch )
    {
        Nst_buffer_destroy(&buf);
        _Nst_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            error_end,
            _Nst_EM_OPEN_STR_LITERAL);
        return;
    }

    Nst_Obj *val_obj = OBJ(Nst_buffer_to_string(&buf));
    SET_ERROR_IF_OP_ERR(val_obj == NULL);
    Nst_obj_hash(val_obj);

    *tok = Nst_tok_new_value(start, cursor.pos, Nst_TT_VALUE, val_obj);
    SET_ERROR_IF_OP_ERR(*tok == NULL);
}

void Nst_add_lines(Nst_SourceText* text)
{
    i8 *text_p = text->text;
    i8 **starts = (i8 **)Nst_raw_calloc(100, sizeof(i8 *));
    if ( starts == NULL )
    {
        text->lines = NULL;
        text->line_count = 0;
        return;
    }

    starts[0] = text_p;
    usize line_count = 1;

    // normalize line endings

    // if the file contains \n, then \n doesn't change and \r\n becomes just \n
    // if the file only contains \r, it is replaced with \n

    bool remove_r = false;
    for ( usize i = 0, n = text->len; i < n; i++ )
    {
        if ( text_p[i] == '\n' )
        {
            remove_r = true;
            break;
        }
    }

    usize offset = 0;
    for ( usize i = 0, n = text->len; i < n; i++ )
    {
        if ( text_p[i] != '\r' )
        {
            text_p[i - offset] = text_p[i];
        }
        else if ( remove_r )
        {
            offset++;
        }
        else
        {
            text_p[i] = '\n';
        }
    }

    text->len = text->len - offset;
    text->text[text->len] = '\0';

    // now all lines end with \n
    for ( usize i = 0, n = text->len; i < n; i++ )
    {
        if ( text_p[i] != '\n' )
        {
            continue;
        }

        if ( i + 1 == n )
        {
            text->lines = starts;
            text->line_count = line_count;
        }

        line_count++;

        if ( line_count % 100 == 0 )
        {
            void *temp = Nst_raw_realloc(starts, i + 100);
            if ( temp == NULL )
            {
                Nst_free(starts);
                text->lines = NULL;
                text->line_count = 0;
                return;
            }
            starts = (i8 **)temp;
        }

        starts[line_count - 1] = text_p + i + 1;
    }

    text->lines = starts;
    text->line_count = line_count;
}

bool Nst_normalize_encoding(Nst_SourceText *text,
                            Nst_CPID        encoding,
                            Nst_Error      *error)
{
    i32 bom_size = 0;
    if ( encoding == Nst_CP_UNKNOWN )
    {
        encoding = Nst_detect_encoding(text->text, text->len, &bom_size);
    }
    else
    {
        Nst_check_bom(text->text, text->len, &bom_size);
    }

    Nst_CP *from = Nst_cp(encoding);

    Nst_Pos pos = { 0, 0, text };
    Nst_Buffer buf;
    if ( !Nst_buffer_init(&buf, text->len + 40) )
    {
        _Nst_SET_ERROR_FROM_OP_ERR(error, pos, pos);
        return false;
    }

    isize n = (isize)text->len - bom_size;
    u8 *text_p = (u8 *)text->text + bom_size;

    bool skip_line_feed = false;

    for ( ; n > 0; )
    {
        // Decode character
        i32 ch_len = from->check_bytes(text_p, n);
        if ( ch_len < 0 )
        {
            Nst_buffer_destroy(&buf);
            _Nst_SET_VALUE_ERROR(error, pos, pos, Nst_sprintf(
                _Nst_EM_INVALID_ENCODING,
                *text_p, from->name));
            return false;
        }
        usize ch_size = ch_len * from->ch_size;
        u32 utf32_ch = from->to_utf32(text_p);
        text_p += ch_size;
        n -= ch_len;

        if ( utf32_ch == '\n' && !skip_line_feed )
        {
            pos.line++;
            pos.col = 0;
        }
        if ( skip_line_feed )
        {
            skip_line_feed = false;
        }

        if ( utf32_ch == '\r' )
        {
            pos.line++;
            pos.col = 0;
            skip_line_feed = true;
        }

        // Re-encode character
        if ( !Nst_buffer_expand_by(&buf, 5) )
        {
            Nst_buffer_destroy(&buf);
            _Nst_SET_ERROR_FROM_OP_ERR(error, pos, pos);
            return false;
        }
        ch_len = Nst_cp_utf8.from_utf32(utf32_ch, buf.data + buf.len);
        buf.len += ch_len;
        pos.col++;
    }
    buf.data[buf.len] = 0;

    Nst_free(text->text);
    text->text = buf.data;
    text->len = buf.len;
    return true;
}

static void parse_first_line(i8       *text,
                             usize     len,
                             i32      *opt_level,
                             Nst_CPID *encoding,
                             bool     *no_default)
{
    i32 bom_size;
    Nst_check_bom(text, len, &bom_size);
    text += bom_size;
    len -= bom_size;

    if ( len < 3 || text[0] != '-' || text[1] != '-' || text[2] != '$' )
    {
        return;
    }

    i8 curr_opt[27]; // max length: --encoding + '=' + 15 characters for the value
    usize i = 0;
    text += 3;
    len -= 3;

    while ( *text != '\n' && *text != '\r' && len != 0 )
    {
        if ( *text != ' ' )
        {
            if ( i < 26 )
            {
                curr_opt[i] = *text;
            }
            i++;
            text++;
            len--;
            continue;
        }
        text++;
        len--;

        if ( i > 26 || i == 0 )
        {
            i = 0;
            continue;
        }
        curr_opt[i] = '\0';

        if ( strcmp(curr_opt, "-O0") == 0 )
        {
            *opt_level = 0;
        }
        else if ( strcmp(curr_opt, "-O1") == 0 )
        {
            *opt_level = 1;
        }
        else if ( strcmp(curr_opt, "-O2") == 0 )
        {
            *opt_level = 2;
        }
        else if ( strcmp(curr_opt, "-O3") == 0 )
        {
            *opt_level = 3;
        }
        else if ( strcmp(curr_opt, "--no-default") == 0 )
        {
            *no_default = true;
        }
        else if ( strncmp(curr_opt, "--encoding", 10) == 0 )
        {
            if ( curr_opt[11] != '=' )
            {
                i = 0;
                continue;
            }
            Nst_CPID new_encoding = Nst_encoding_from_name(curr_opt + 11);
            if ( new_encoding != Nst_CP_UNKNOWN )
            {
                *encoding = new_encoding;
            }
        }
        i = 0;
    }

    if ( i > 12 || i == 0 )
    {
        i = 0;
        return;
    }
    curr_opt[i] = '\0';

    if ( strcmp(curr_opt, "-O0") == 0 )
    {
        *opt_level = 0;
    }
    else if ( strcmp(curr_opt, "-O1") == 0 )
    {
        *opt_level = 1;
    }
    else if ( strcmp(curr_opt, "-O2") == 0 )
    {
        *opt_level = 2;
    }
    else if ( strcmp(curr_opt, "-O3") == 0 )
    {
        *opt_level = 3;
    }
    else if ( strcmp(curr_opt, "--no-default") == 0 )
    {
        *no_default = true;
    }
    else if ( strncmp(curr_opt, "--encoding", 10) == 0 )
    {
        if ( curr_opt[11] != '=' )
        {
            return;
        }
        Nst_CPID new_encoding = Nst_encoding_from_name(curr_opt + 11);
        if ( new_encoding != Nst_CP_UNKNOWN )
        {
            *encoding = new_encoding;
        }
    }
}
