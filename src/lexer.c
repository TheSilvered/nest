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
#include "interpreter.h" // nst_get_full_path
#include "encoding.h"
#include "format.h"

#define START_CH_SIZE 8 * sizeof(i8)

#define SET_INVALID_ESCAPE_ERROR do { \
    nst_free(end_str); \
    _NST_SET_RAW_SYNTAX_ERROR( \
        error, \
        escape_start, \
        cursor.pos, \
        _NST_EM_INVALID_ESCAPE); \
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

#define SET_ERROR_IF_OP_ERR(cond, ...) do { \
    if ( (cond) || lexer_err.name != NULL ) \
    { \
        _NST_SET_ERROR_FROM_OP_ERR(error, &lexer_err, cursor.pos, cursor.pos); \
        __VA_ARGS__ \
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
static Nst_OpErr lexer_err = { NULL, NULL };

static inline void advance();
static inline void go_back();
static void make_symbol(Nst_Tok **tok, Nst_Error *error);
static void make_num_literal(Nst_Tok **tok, Nst_Error *error);
static void make_ident(Nst_Tok **tok, Nst_Error *error);
static void make_str_literal(Nst_Tok **tok, Nst_Error *error);
static void parse_first_line(i8   *text,
                             usize len,
                             i32  *opt_level,
                             bool *force_cp1252,
                             bool *no_default);

Nst_LList *nst_tokenizef(i8             *filename,
                         bool            force_cp1252,
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
        nst_fprint(nst_io->err, "File \"");
        nst_fprint(nst_io->err, (const i8 *)filename);
        nst_fprintln(nst_io->err, "\" not found");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    i8 *text = (i8 *)nst_raw_calloc(size + 1, sizeof(i8));
    if ( text == NULL )
    {
        nst_fprint(nst_io->err, "Memory allocation failed\n");
        return NULL;
    }

    usize str_len = fread(text, sizeof(i8), size + 1, file);
    fclose(file);
    text[str_len] = '\0';

    i8 *full_path;
    nst_get_full_path(filename, &full_path, NULL, NULL);
    if ( full_path == NULL )
    {
        nst_fprint(nst_io->err, "Memory allocation failed\n");
        return NULL;
    }

    src_text->text = text;
    src_text->len = str_len;
    src_text->path = full_path;

    parse_first_line(text, str_len, opt_level, &force_cp1252, no_default);

    i32 text_offset = nst_normalize_encoding(src_text, force_cp1252, error);
    nst_add_lines(src_text, text_offset);
    if ( error->occurred || text_offset == -1 )
    {
        return NULL;
    }

    src_text->text += text_offset;
    src_text->len  -= text_offset;
    Nst_LList *tokens = nst_tokenize(src_text, error);
    src_text->text -= text_offset;
    src_text->len  += text_offset;
    return tokens;
}

Nst_LList *nst_tokenize(Nst_SourceText *text, Nst_Error *error)
{
    Nst_Tok *tok = NULL;
    Nst_LList *tokens = nst_llist_new(&lexer_err);

    cursor.idx = -1;
    cursor.ch = ' ';
    cursor.len = text->len;
    cursor.text = text->text;
    cursor.pos.col = -1;
    cursor.pos.line = 0;
    cursor.pos.text = text;

    if (tokens == NULL)
    {
        _NST_SET_ERROR_FROM_OP_ERR(error, &lexer_err, cursor.pos, cursor.pos);
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
            tok = nst_tok_new_noend(nst_copy_pos(cursor.pos), NST_TT_ENDL, &lexer_err);
            if (tokens == NULL)
            {
                _NST_SET_ERROR_FROM_OP_ERR(error, &lexer_err, cursor.pos, cursor.pos);
            }
        }
        else if ( cursor.ch == '\\' )
        {
            advance();
        }
        else
        {
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                cursor.pos,
                cursor.pos,
                _NST_EM_INVALID_CHAR);
        }

        if ( error->occurred )
        {
            if ( tok != NULL )
            {
                nst_token_destroy(tok);
            }
            nst_llist_destroy(tokens, (Nst_LListDestructor)nst_token_destroy);
            return NULL;
        }

        if ( tok != NULL )
        {
            nst_llist_append(tokens, tok, true, &lexer_err);
        }
        tok = NULL;
        advance();
    }

    tok = nst_tok_new_noend(cursor.pos, NST_TT_EOFILE, &lexer_err);
    if ( tok == NULL )
    {
        _NST_SET_ERROR_FROM_OP_ERR(error, &lexer_err, cursor.pos, cursor.pos);
        nst_llist_destroy(tokens, (Nst_LListDestructor)nst_token_destroy);
        return NULL;
    }
    nst_llist_append(tokens, tok, true, &lexer_err);
    if ( lexer_err.name != NULL )
    {
        _NST_SET_ERROR_FROM_OP_ERR(error, &lexer_err, cursor.pos, cursor.pos);
        nst_llist_destroy(tokens, (Nst_LListDestructor)nst_token_destroy);
        return NULL;
    }

    return tokens;
}

inline static void advance()
{
    cursor.idx++;
    cursor.pos.col++;

    if ( cursor.idx > (i32) cursor.len )
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
    Nst_Pos start = nst_copy_pos(cursor.pos);
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
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_OPEN_COMMENT);
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

    Nst_TokType token_type = nst_tok_from_str(symbol);

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

        token_type = nst_tok_from_str(symbol);
    }

    *tok = nst_tok_new_noval(start, nst_copy_pos(cursor.pos), token_type, &lexer_err);
    SET_ERROR_IF_OP_ERR(*tok == NULL);
}

static void make_num_literal(Nst_Tok **tok, Nst_Error *error)
{
    i8 *start_p = cursor.text + cursor.idx;
    Nst_Pos start = nst_copy_pos(cursor.pos);
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
            Nst_Obj *val = nst_byte_new(0, &lexer_err);
            SET_ERROR_IF_OP_ERR(val == NULL);
            *tok = nst_tok_new_value(start, cursor.pos, NST_TT_VALUE, val, &lexer_err);
            SET_ERROR_IF_OP_ERR(*tok == NULL, nst_dec_ref(val););
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
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        end = nst_copy_pos(cursor.pos);
        goto end;
    case 'o':
    case 'O':
        advance();
        if ( !CH_IS_OCT(cursor.ch) )
        {
            go_back();
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_BAD_INT_LITERAL);
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
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        end = nst_copy_pos(cursor.pos);
        goto end;
    case 'x':
    case 'X':
        advance();
        if ( !CH_IS_HEX(cursor.ch) )
        {
            go_back();
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_BAD_INT_LITERAL);
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
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        end = nst_copy_pos(cursor.pos);
        goto end;
    case 'h':
    case 'H':
    {
        advance();
        if ( !CH_IS_HEX(cursor.ch) )
        {
            go_back();
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_BAD_BYTE_LITERAL);
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
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_BAD_INT_LITERAL);
            return;
        }
        go_back();
        i32 val = (i32)ch2 + (i32)ch1 * 16;
        if ( neg )
        {
            val = -val;
        }
        Nst_Obj *obj = nst_byte_new(0, &lexer_err);
        SET_ERROR_IF_OP_ERR(obj == NULL);
        *tok = nst_tok_new_value(start, cursor.pos, NST_TT_VALUE, obj, &lexer_err);
        SET_ERROR_IF_OP_ERR(*tok == NULL, nst_dec_ref(obj););
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
        _NST_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            cursor.pos,
            _NST_EM_BAD_REAL_LITERAL);
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
        _NST_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            cursor.pos,
            _NST_EM_BAD_REAL_LITERAL);
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
    ltrl = (i8 *)nst_malloc(ltrl_size + 3, sizeof(u8), &lexer_err);
    SET_ERROR_IF_OP_ERR(ltrl == NULL);
    ltrl[0] = neg ? '-' : '+';
    memcpy(ltrl + 1, start_p, ltrl_size);
    ltrl[ltrl_size + 1] = '\0';
    s.value = ltrl;
    s.len = ltrl_size + 1;
    end = cursor.pos;

    if ( is_real )
    {
        res = nst_string_parse_real(&s, &lexer_err);
        nst_free(ltrl);
    }
    else
    {
        res = nst_string_parse_int(&s, 0, &lexer_err);
        advance();
        if ( cursor.ch == 'b' || cursor.ch == 'B' )
        {
            if ( res == NULL && lexer_err.name == nst_s.e_MemoryError )
            {
                nst_dec_ref(lexer_err.name);
                nst_dec_ref(lexer_err.message);
                ltrl[ltrl_size + 1] = 'b';
                ltrl[ltrl_size + 2] = '\0';
                s.len++;
                res = nst_string_parse_byte(&s, &lexer_err);
                nst_free(ltrl);
            }
            else
            {
                nst_free(ltrl);
                SET_ERROR_IF_OP_ERR(res == NULL);
                Nst_Obj *new_res = nst_byte_new(AS_INT(res) & 0xff, &lexer_err);
                nst_dec_ref(res);
                res = new_res;
            }
            end = cursor.pos;
        }
        else
        {
            nst_free(ltrl);
            go_back();
        }
    }

    SET_ERROR_IF_OP_ERR(res == NULL);
    *tok = nst_tok_new_value(start, end, NST_TT_VALUE, res, &lexer_err);
    SET_ERROR_IF_OP_ERR(*tok == NULL);
}

static void make_ident(Nst_Tok **tok, Nst_Error *error)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);

    i8 *str;
    i8 *str_start = cursor.text + cursor.idx;
    usize str_len = 0;

    while ( cursor.idx < (i32)cursor.len &&
            (CH_IS_ALPHA(cursor.ch) ||
             CH_IS_DEC(cursor.ch)   ||
             (u8)cursor.ch >= 0b10000000) )
    {
        i32 res = nst_check_utf8_bytes(
            (u8 *)cursor.text + cursor.idx,
            (usize)(cursor.text - cursor.idx));
        str_len += res;
        for ( i32 i = 0; i < res; i++ )
        {
            advance();
        }
    }
    go_back();

    str = (i8 *)nst_malloc(str_len + 1, sizeof(u8), &lexer_err);
    SET_ERROR_IF_OP_ERR(str == NULL);

    if ( str == NULL )
    {
        _NST_FAILED_ALLOCATION(error, start, cursor.pos);
        return;
    }
    memcpy(str, str_start, str_len);
    str[str_len] = '\0';

    Nst_Pos end = nst_copy_pos(cursor.pos);
    Nst_StrObj *val_obj = STR(nst_string_new_c_raw(str, true, &lexer_err));
    SET_ERROR_IF_OP_ERR(val_obj == NULL, nst_free(str););
    nst_obj_hash(OBJ(val_obj));

    *tok = nst_tok_new_value(start, end, NST_TT_IDENT, OBJ(val_obj), &lexer_err);
    SET_ERROR_IF_OP_ERR(val_obj == NULL, nst_dec_ref(val_obj););
}

static void make_str_literal(Nst_Tok **tok, Nst_Error *error)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);
    Nst_Pos escape_start = nst_copy_pos(cursor.pos);
    i8 closing_ch = cursor.ch;
    bool allow_multiline = cursor.ch == '"';
    bool escape = false;

    i8 *end_str = (i8 *)nst_malloc(START_CH_SIZE, sizeof(u8), &lexer_err);
    i8 *end_str_realloc = NULL;
    SET_ERROR_IF_OP_ERR(end_str == NULL);

    usize str_len = 0;
    usize chunk_size = START_CH_SIZE;

    advance(); // still on the opening character

    // while there is text to add and (the string has not ended or
    // the end is inside and escape)
    while ( cursor.idx < (i32) cursor.len &&
            (cursor.ch != closing_ch || escape) )
    {
        if ( str_len + 4 == chunk_size )
        {
            chunk_size = (usize)(chunk_size * 1.5);
            end_str_realloc = (i8 *)nst_realloc(
                end_str,
                chunk_size,
                sizeof(i8),
                0, &lexer_err);
            SET_ERROR_IF_OP_ERR(end_str_realloc == NULL, nst_free(end_str););
            end_str = end_str_realloc;
        }

        if ( !escape )
        {
            if ( cursor.ch == '\n' && !allow_multiline )
            {
                nst_free(end_str);
                _NST_SET_RAW_SYNTAX_ERROR(
                    error,
                    cursor.pos,
                    cursor.pos,
                    _NST_EM_UNEXPECTED_NEWLINE);
                return;
            }
            else if ( cursor.ch == '\\' )
            {
                escape = true;
                escape_start = nst_copy_pos(cursor.pos);
            }
            else
            {
                end_str[str_len++] = cursor.ch;
            }
            advance();
            continue;
        }

        // If there is an escape sequence
        switch ( cursor.ch )
        {
        case '\'':end_str[str_len++] = '\''; break;
        case '"': end_str[str_len++] = '"' ; break;
        case '\\':end_str[str_len++] = '\\'; break;
        case 'a': end_str[str_len++] = '\a'; break;
        case 'b': end_str[str_len++] = '\b'; break;
        case 'e': end_str[str_len++] ='\x1b';break;
        case 'f': end_str[str_len++] = '\f'; break;
        case 'n': end_str[str_len++] = '\n'; break;
        case 'r': end_str[str_len++] = '\r'; break;
        case 't': end_str[str_len++] = '\t'; break;
        case 'v': end_str[str_len++] = '\v'; break;
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

            end_str[str_len++] = result;
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

            if ( num <= 0x7f )
            {
                end_str[str_len++] = (i8)num;
            }
            else if ( num <= 0x7ff )
            {
                end_str[str_len++] = 0b11000000 | (i8)(num >> 6);
                end_str[str_len++] = 0b10000000 | (i8)(num & 0x3f);
            }
            else if ( num <= 0xffff )
            {
                end_str[str_len++] = 0b11100000 | (i8)(num >> 12);
                end_str[str_len++] = 0b10000000 | (i8)(num >> 6 & 0x3f);
                end_str[str_len++] = 0b10000000 | (i8)(num & 0x3f);
            }
            else if ( num <= 0x10ffff )
            {
                end_str[str_len++] = 0b11110000 | (i8)(num >> 18);
                end_str[str_len++] = 0b10000000 | (i8)(num >> 12 & 0x3f);
                end_str[str_len++] = 0b10000000 | (i8)(num >> 6 & 0x3f);
                end_str[str_len++] = 0b10000000 | (i8)(num & 0x3f);
            }
            else
            {
                SET_INVALID_ESCAPE_ERROR;
            }
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
                end_str[str_len++] = ch1;
                go_back();
                break;
            }
            i8 ch2 = cursor.ch - '0';

            advance();
            if ( !CH_IS_OCT(cursor.ch) )
            {
                end_str[str_len++] = (ch1 << 3) + ch2;
                go_back();
                break;
            }
            i8 ch3 = cursor.ch - '0';
            end_str[str_len++] = (ch1 << 6) + (ch2 << 3) + ch3;
            break;
        }
        default:
            SET_INVALID_ESCAPE_ERROR;
        }

        escape = false;
        advance();
    }

    go_back();
    Nst_Pos error_end = nst_copy_pos(cursor.pos);
    advance();

    if ( cursor.ch != closing_ch )
    {
        nst_free(end_str);
        _NST_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            error_end,
            _NST_EM_OPEN_STR_LITERAL);
        return;
    }

    if ( str_len + 20 < chunk_size )
    {
        end_str = (i8 *)nst_realloc(
            end_str,
            str_len + 1,
            sizeof(i8),
            chunk_size, NULL);
    }

    end_str[str_len] = '\0';

    Nst_StrObj *val_obj = STR(nst_string_new(end_str, str_len, true, &lexer_err));
    SET_ERROR_IF_OP_ERR(val_obj == NULL, nst_free(end_str););
    nst_obj_hash(OBJ(val_obj));

    *tok = nst_tok_new_value(start, cursor.pos, NST_TT_VALUE, OBJ(val_obj), &lexer_err);
    SET_ERROR_IF_OP_ERR(*tok == NULL, nst_dec_ref(val_obj););
}

void nst_add_lines(Nst_SourceText* text, i32 start_offset)
{
    if ( start_offset < 0 )
    {
        start_offset = 0;
    }

    i8 *text_p = text->text;
    i8 **starts = (i8 **)nst_raw_calloc(100, sizeof(i8 *));
    if ( starts == NULL )
    {
        text->lines = NULL;
        text->line_count = 0;
        return;
    }

    starts[0] = text_p + start_offset;
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
            void *temp = nst_raw_realloc(starts, i + 100);
            if ( temp == NULL )
            {
                nst_free(starts);
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

i32 nst_normalize_encoding(Nst_SourceText *text,
                           bool            is_cp1252,
                           Nst_Error      *error)
{
    u8 *text_p = (u8 *)text->text;
    usize ch_count = 0;
    usize i = 0;
    usize n = text->len;
    bool skipped_bom = false;

    if ( is_cp1252 )
    {
        goto fix_encoding;
    }

    if ( n >= 3 &&
         text_p[0] == (u8)'\xef' &&
         text_p[1] == (u8)'\xbb' &&
         text_p[2] == (u8)'\xbf' )
    {
        text_p += 3;
        n -= 3;
        skipped_bom = true;
    }

    for ( ; i < n; i++ )
    {
        if ( text_p[i] > 0x7f )
        {
            i32 offset = nst_check_utf8_bytes(text_p + i, n - i);
            if ( offset == -1 )
            {
                ch_count++;
                i++;
                goto fix_encoding;
            }
            i += offset - 1;
            ch_count += offset;
        }
    }
    return skipped_bom ? 3 : 0;

fix_encoding:
    if ( skipped_bom )
    {
        text_p -= 3;
        n += 3;
    }

    for ( ; i < n; i++ )
    {
        if ( text_p[i] > 0x7f )
        {
            ch_count++;
        }
    }

    i8 *new_text = (i8 *)nst_raw_calloc(n + ch_count * 3 + 1, sizeof(i8));
    if ( new_text == NULL )
    {
        Nst_Pos pos = { 1, 0, text };
        _NST_FAILED_ALLOCATION(error, pos, pos);
        return -1;
    }

    i8 *utf8_ptr = new_text;
    i32 line = 0;
    bool skip_line_feed = false;
    i32 col = 0;
    for ( i = 0; i < n; i++ )
    {
        if ( text_p[i] <= 0x7f )
        {
            if ( text_p[i] == '\n' )
            {
                if ( !skip_line_feed )
                {
                    line++;
                }
                col = 0;
            }
            else if ( text_p[i] == '\r' )
            {
                line++;
                skip_line_feed = true;
                col = 0;
            }

            *utf8_ptr++ = text_p[i];
            continue;
        }
        i32 offset = nst_cp1252_to_utf8(utf8_ptr, text_p[i]);

        if ( offset == -1 )
        {
            nst_free(new_text);
            Nst_Pos pos = { line, col, text };
            _NST_SET_RAW_VALUE_ERROR(
                error,
                pos, pos,
                "invalid cp1252 byte found");
            return -1;
        }

        utf8_ptr += offset;
        col++;
    }
    text->len = utf8_ptr - new_text;
    nst_free(text->text);
    text->text = new_text;
    return 0;
}

static void parse_first_line(i8  *text,
                             usize len,
                             i32   *opt_level,
                             bool  *force_cp1252,
                             bool  *no_default)
{
    if ( len >= 3 &&
         text[0] == '\xef' &&
         text[1] == '\xbb' &&
         text[2] == '\xbf' )
    {
        text += 3;
        len -= 3;
    }

    if ( len < 3 || text[0] != '-' || text[1] != '-' || text[2] != '$' )
    {
        return;
    }

    i8 curr_opt[13];
    usize i = 0;
    text += 3;
    len -= 3;

    while ( *text != '\n' && *text != '\r' && len != 0 )
    {
        if ( *text != ' ' )
        {
            if ( i < 12 )
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

        if ( i > 12 || i == 0 )
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
        else if ( strcmp(curr_opt, "--cp1252") == 0 )
        {
            *force_cp1252 = true;
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
    else if ( strcmp(curr_opt, "--cp1252") == 0 )
    {
        *force_cp1252 = true;
    }
}
