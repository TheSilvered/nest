#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_internal.h"
#include "hash.h"
#include "lexer.h"
#include "nst_types.h"
#include "tokens.h"
#include "interpreter.h"

#define START_CH_SIZE 8 * sizeof(char)

#define SET_INVALID_ESCAPE_ERROR do { \
    free(end_str); \
    _NST_SET_RAW_SYNTAX_ERROR( \
        error, \
        escape_start, \
        cursor.pos, \
        _NST_EM_INVALID_ESCAPE); \
    return; } while (0)

#define CUR_AT_END (cursor.idx >= (long)cursor.len)

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

#define CHECK_MIN(min_val) \
    if ( errno == ERANGE && sign == -1 ) \
    { \
        char *ones_start = int_part; \
        while ( *ones_start == '0' ) \
            ones_start++; \
        if ( strcmp(ones_start, #min_val) != 0 ) \
        { \
            free(int_part); \
            _NST_SET_RAW_MEMORY_ERROR( \
                error, \
                start, \
                end, \
                _NST_EM_INT_TOO_BIG); \
            return; \
        } \
        free(int_part); \
        value = -9223372036854775807; \
        --value; \
        errno = 0; \
    } \
    else if ( errno == ERANGE ) \
    { \
        free(int_part); \
        _NST_SET_RAW_MEMORY_ERROR(error, start, end, _NST_EM_INT_TOO_BIG); \
        return; \
    } \
    else \
        value *= sign

typedef struct LexerCursor {
    char *text;
    size_t len;
    Nst_Pos pos;
    long idx;
    char ch;
} LexerCursor;

static LexerCursor cursor;

static inline void advance();
static inline void go_back();
static char *add_while_in(bool (*cond_func)(char),
                          bool ignore_underscore,
                          size_t *len);

static void make_symbol(Nst_LexerToken **tok, Nst_Error *error);
static void make_num_literal(Nst_LexerToken **tok, Nst_Error *error);
static void make_ident(Nst_LexerToken **tok);
static void make_str_literal(Nst_LexerToken **tok, Nst_Error *error);
static void parse_first_line(char  *text,
                             size_t len,
                             int   *opt_level,
                             bool  *force_cp1252,
                             bool  *no_default);

LList *nst_ftokenize(char           *filename,
                     bool            force_cp1252,
                     int            *opt_level,
                     bool           *no_default,
                     Nst_SourceText *src_text,
                     Nst_Error      *error)
{
    *opt_level = 3;
    *no_default = false;

    FILE *file = fopen(filename, "rb");
    if ( file == NULL )
    {
        printf("File \"%s\" not found\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *text = (char *)calloc(size + 1, sizeof(char));
    if ( text == NULL )
    {
        printf("Ran out of memory while reading the file\n");
        return NULL;
    }

    size_t str_len = fread(text, sizeof(char), size + 1, file);
    fclose(file);
    text[str_len] = '\0';

    char *full_path;
    nst_get_full_path(filename, &full_path, NULL);

    src_text->text = text;
    src_text->len = str_len;
    src_text->path = full_path;

    parse_first_line(text, str_len, opt_level, &force_cp1252, no_default);

    nst_normalize_encoding(src_text, force_cp1252, error);
    nst_add_lines(src_text);
    if ( error->occurred )
    {
        return NULL;
    }

    return nst_tokenize(src_text, error);
}

LList *nst_tokenize(Nst_SourceText *text, Nst_Error *error)
{
    Nst_LexerToken *tok = NULL;
    LList *tokens = LList_new();

    cursor.idx = -1;
    cursor.ch = ' ';
    cursor.len = text->len;
    cursor.text = text->text;
    cursor.pos.col = -1;
    cursor.pos.line = 0;
    cursor.pos.text = text;

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
        else if ( CH_IS_ALPHA(cursor.ch))
            make_ident(&tok);
        else if ( cursor.ch == '"' || cursor.ch == '\'' )
        {
            make_str_literal(&tok, error);
        }
        else if ( cursor.ch == '\n' )
        {
            tok = nst_new_token_noend(nst_copy_pos(cursor.pos), NST_TT_ENDL);
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
                nst_destroy_token(tok);
            }
            LList_destroy(tokens, (LList_item_destructor)nst_destroy_token);
            return NULL;
        }

        if ( tok != NULL )
        {
            LList_append(tokens, tok, true);
        }
        tok = NULL;
        advance();
    }

    LList_append(tokens, nst_new_token_noend(cursor.pos, NST_TT_EOFILE), true);
    return tokens;
}

inline static void advance()
{
    cursor.idx++;
    cursor.pos.col++;

    if ( cursor.idx > (long) cursor.len )
    {
        return;
    }

    if ( cursor.ch == '\n' )
    {
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
    }
}

inline static char *add_while_in(bool (*cond_func)(char),
                                 bool ignore_underscore,
                                 size_t *len)
{
    if ( len != NULL )
    {
        *len = 0; // to set the value even if the function fails
    }
    char *str;
    if ( !(cond_func(cursor.ch)) )
    {
        return NULL;
    }

    char *str_start = cursor.text + cursor.idx;
    size_t str_len = 1;
    advance();

    while ( cursor.idx < (long) cursor.len &&
            (cond_func(cursor.ch) || (ignore_underscore && cursor.ch == '_')) )
    {
        if ( !ignore_underscore || cursor.ch != '_' )
        {
            str_len++;
        }
        advance();
    }
    go_back();

    str = (char *)malloc(str_len + 1);

    if ( str == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    if ( ignore_underscore )
    {
        for ( size_t i = 0; i < str_len; i++ )
        {
            if ( *str_start == '_' )
            {
                i--;
            }
            else
            {
                str[i] = *str_start;
            }
            str_start++;
        }
    }
    else
    {
        memcpy(str, str_start, str_len);
    }

    str[str_len] = '\0';

    if ( len != NULL )
    {
        *len = str_len;
    }

    return str;
}

static void make_symbol(Nst_LexerToken **tok, Nst_Error *error)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);
    char symbol[4] = { cursor.ch, 0, 0, 0 };
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
            advance();
            if ( can_close && cursor.ch == '-' )
            {
                advance();
                was_closed = true;
                break;
            }

            can_close = cursor.ch == '/';
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

    int token_type = nst_str_to_tok(symbol);

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

        token_type = nst_str_to_tok(symbol);
    }

    *tok = nst_new_token_noval(start, nst_copy_pos(cursor.pos), token_type);
}

static bool is_dec(char ch)
{
    return CH_IS_DEC(ch);
}

static bool is_bin(char ch)
{
    return CH_IS_BIN(ch);
}

static bool is_oct(char ch)
{
    return CH_IS_OCT(ch);
}

static bool is_hex(char ch)
{
    return CH_IS_HEX(ch);
}

static void make_num_literal(Nst_LexerToken **tok, Nst_Error *error)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);
    Nst_Pos end = start;
    int sign = 1;

    // If there is a minus or a plus it can also be a symbol
    if ( cursor.ch == '-' || cursor.ch == '+' )
    {
        sign = cursor.ch == '-' ? -1 : 1;

        advance();
        // In case it's a symbol, make_symbol handles that
        if ( !CH_IS_DEC(cursor.ch) )
        {
            go_back();
            make_symbol(tok, error);
            return;
        }
    }

    char *int_part;
    size_t len_int_part;
    size_t len_frac_part;

    if ( cursor.ch == '0' )
    {
        Nst_Int value;
        advance();

        // binary literals
        if ( cursor.ch == 'b' || cursor.ch == 'B' )
        {
            advance();
            int_part = add_while_in(is_bin, true, NULL);

            if ( int_part == NULL)
            {
                go_back();
                end = nst_copy_pos(cursor.pos);
                *tok = nst_new_token_value(
                    start,
                    end,
                    NST_TT_VALUE,
                    nst_new_byte(0));
                return;
            }

            advance();

            // they cannot be followed by a digit but can by an identifier
            // so 0b1019 is invalid but 0b01hello is valid
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
            advance();

            value = strtoll(int_part, NULL, 2);

            CHECK_MIN(1000000000000000000000000000000000000000000000000000000000000000);
            free(int_part);

            goto byte;
        }
        // Int hex literals
        else if ( cursor.ch == 'x' || cursor.ch == 'X' )
        {
            advance();
            int_part = add_while_in(is_hex, true, NULL);

            if ( int_part == NULL)
            {
                go_back();
                _NST_SET_RAW_SYNTAX_ERROR(
                    error,
                    start,
                    cursor.pos,
                    _NST_EM_BAD_INT_LITERAL);
                return;
            }

            advance();

            // they cannot be followed by a word
            // 0xabc(hello) is fine but 0xabchello is not
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

            value = strtoll(int_part, NULL, 16);

            CHECK_MIN(8000000000000000);
            free(int_part);

            // hex literals cannot have a b suffix as b is a digit,
            // for hex bytex the 0h prefix is used instead
            *tok = nst_new_token_value(
                start, end,
                NST_TT_VALUE,
                nst_new_int(value));
            return;
        }
        // Byte hex literals
        else if ( cursor.ch == 'h' || cursor.ch == 'H' )
        {
            advance();
            int_part = add_while_in(is_hex, true, NULL);

            if ( int_part == NULL)
            {
                go_back();
                _NST_SET_RAW_SYNTAX_ERROR(
                    error,
                    start,
                    cursor.pos,
                    _NST_EM_BAD_BYTE_LITERAL);
                return;
            }

            advance();

            // they cannot be followed by a word
            // 0habc(hello) is fine but 0habchello is not
            if ( CH_IS_ALPHA(cursor.ch) )
            {
                _NST_SET_RAW_SYNTAX_ERROR(
                    error,
                    start,
                    cursor.pos,
                    _NST_EM_BAD_BYTE_LITERAL);
                return;
            }
            go_back();
            end = nst_copy_pos(cursor.pos);

            value = strtoll(int_part, NULL, 16);

            CHECK_MIN(8000000000000000);
            free(int_part);

            *tok = nst_new_token_value(
                start, end,
                NST_TT_VALUE,
                nst_new_byte((Nst_Byte)(value & 0xff)));
            return;
        }
        // octal literals
        else if ( cursor.ch == 'o' || cursor.ch == 'O' )
        {
            advance();
            int_part = add_while_in(is_oct, true, NULL);

            if ( int_part == NULL)
            {
                go_back();
                _NST_SET_RAW_SYNTAX_ERROR(
                    error,
                    start,
                    cursor.pos,
                    _NST_EM_BAD_INT_LITERAL);
                return;
            }

            advance();

            // they cannot be followed by a digit but can by an identifier
            // so 0o76459 is invalid but 0o53hello is valid
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

            value = strtoll(int_part, NULL, 8);

            CHECK_MIN(1000000000000000000000);
            free(int_part);

            advance();
            goto byte;
        }
        else if ( !CH_IS_DEC(cursor.ch) && cursor.ch != '.' )
        {
            value = 0;
            goto byte;
        }
        else
        {
            go_back();
            // there is at least a 0, no error can occur here
            int_part = add_while_in(is_dec, true, &len_int_part);

            advance();
            if ( cursor.ch == '.' )
            {
                goto real_lit;
            }
            int_lit: go_back();

            end = nst_copy_pos(cursor.pos);
            value = strtoll(int_part, NULL, 10);

            CHECK_MIN(9223372036854775808);

            advance();
            goto byte;
        }

    byte:
        if ( cursor.ch == 'b' || cursor.ch == 'B' )
        {
            *tok = nst_new_token_value(
                start, cursor.pos,
                NST_TT_VALUE,
                nst_new_byte((Nst_Byte)(value & 0xff)));
        }
        else
        {
            go_back();
            *tok = nst_new_token_value(
                start, end,
                NST_TT_VALUE,
                nst_new_int(value));
        }
        return;
    }

    int_part = add_while_in(is_dec, true, &len_int_part);
    advance();
    if ( cursor.ch != '.' )
    {
        goto int_lit;
    }

real_lit:
    advance();
    char *frac_part = add_while_in(is_dec, true, &len_frac_part);
    if ( frac_part == NULL )
    {
        free(int_part);
        go_back();
        _NST_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            cursor.pos,
            _NST_EM_BAD_REAL_LITERAL);
        return;
    }
    advance();

    if ( cursor.ch == 'e' || cursor.ch == 'E' )
    {
        int exp_neg = false;
        size_t len_exp;

        advance();
        if ( cursor.ch == '-' || cursor.ch == '+' )
        {
            exp_neg = cursor.ch == '-';
            advance();
        }

        char *exp = add_while_in(is_dec, true, &len_exp);
        if ( exp == NULL )
        {
            free(int_part);
            free(frac_part);
            go_back();
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                start,
                cursor.pos,
                _NST_EM_BAD_REAL_LITERAL);
            return;
        }

        end = nst_copy_pos(cursor.pos);
        char *complete_lit = (char *)malloc(
        //        9        .       26          e-    5       \0
            len_int_part + 1 + len_frac_part + 2 + len_exp + 1);
        if ( complete_lit == NULL )
        {
            errno = ENOMEM;
            return;
        }

        sprintf(
            complete_lit,
            "%s%s%s%s%s",
            int_part, ".", frac_part,
            exp_neg ? "e-" : "e+", exp);
        free(int_part);
        free(frac_part);
        free(exp);

        Nst_Real value = strtod(complete_lit, NULL);
        *tok = nst_new_token_value(
            start, end,
            NST_TT_VALUE,
            nst_new_real(value * sign));
        free(complete_lit);
        return;
    }
    else
    {
        go_back();
        char *complete_lit = (char *)malloc(len_int_part + len_frac_part + 2);
        if ( complete_lit == NULL )
        {
            errno = ENOMEM;
            return;
        }

        memcpy(complete_lit, int_part, len_int_part);
        complete_lit[len_int_part] = '.';
        memcpy(complete_lit + len_int_part + 1, frac_part, len_frac_part);
        complete_lit[len_int_part + len_frac_part + 1] = '\0';
        free(int_part);
        free(frac_part);

        Nst_Real value = strtod(complete_lit, NULL);
        *tok = nst_new_token_value(
            start, cursor.pos,
            NST_TT_VALUE,
            nst_new_real(value * sign));
        free(complete_lit);
        return;
    }
}

static bool is_ident_ch(char ch)
{
    return CH_IS_ALPHA(ch) || CH_IS_DEC(ch);
}

static void make_ident(Nst_LexerToken **tok)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);
    char *str = add_while_in(is_ident_ch, false, NULL);

    Nst_Pos end = nst_copy_pos(cursor.pos);
    Nst_StrObj *val_obj = STR(nst_new_cstring_raw(str, true));
    nst_hash_obj(OBJ(val_obj));

    *tok = nst_new_token_value(start, end, NST_TT_IDENT, OBJ(val_obj));
}

static void make_str_literal(Nst_LexerToken **tok, Nst_Error *error)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);
    Nst_Pos escape_start = nst_copy_pos(cursor.pos);
    char closing_ch = cursor.ch;
    bool allow_multiline = cursor.ch == '"';
    bool escape = false;

    char *end_str = (char *)malloc(START_CH_SIZE);
    char *end_str_realloc = NULL;
    char ch1;
    char ch2;

    if ( end_str == NULL )
    {
        errno = ENOMEM;
        return;
    }

    size_t str_len = 0;
    size_t chunk_size = START_CH_SIZE;

    advance(); // still on the opening character

    // while there is text to add and (the string has not ended or
    // the end is inside and escape)
    while ( cursor.idx < (long) cursor.len &&
            (cursor.ch != closing_ch || escape) )
    {
        if ( str_len + 1 == chunk_size )
        {
            chunk_size = (size_t)(chunk_size * 1.5);
            end_str_realloc = (char *)realloc(
                end_str,
                sizeof(char) * chunk_size);
            if ( end_str_realloc == NULL )
            {
                free(end_str);
                _NST_FAILED_ALLOCATION(error, cursor.pos, cursor.pos);
                return;
            }
            end_str = end_str_realloc;
        }

        if ( !escape )
        {
            if ( cursor.ch == '\n' && !allow_multiline )
            {
                free(end_str);
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
            advance();
            if ( CUR_AT_END || cursor.ch == closing_ch )
            {
                SET_INVALID_ESCAPE_ERROR;
            }

            ch1 = (char)tolower(cursor.ch);
            advance();

            if ( CUR_AT_END || cursor.ch == closing_ch )
            {
                SET_INVALID_ESCAPE_ERROR;
            }

            ch2 = (char)tolower(cursor.ch);

            if ( ch1 < '0' || ch1 > 'f'  ||
                (ch1 > '9' && ch1 < 'a') ||
                 ch2 < '0' || ch2 > 'f'  ||
                (ch2 > '9' && ch2 < 'a') )
            {
                SET_INVALID_ESCAPE_ERROR;
            }

            {
                char result = ((ch1 > '9' ? ch1 - 'a' + 10 : ch1 - '0') << 4) +
                              (ch2 > '9' ? ch2 - 'a' + 10 : ch2 - '0');

                end_str[str_len++] = result;
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            ch1 = cursor.ch;

            advance();
            if ( cursor.ch < '0' || cursor.ch > '7' )
            {
                end_str[str_len++] = ch1 - '0';
                go_back();
                break;
            }

            ch2 = cursor.ch;

            advance();
            if ( cursor.ch < '0' || cursor.ch > '7' )
            {
                end_str[str_len++] = ((ch1 - '0') << 3) + ch2 - '0';
                go_back();
                break;
            }

            end_str[str_len++] = ((ch1 - '0') << 6) + ((ch2 - '0') << 3) +
                                 cursor.ch - '0';
            break;

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
        free(end_str);
        _NST_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            error_end,
            _NST_EM_OPEN_STR_LITERAL);
        return;
    }

    if ( str_len < chunk_size )
    {
        end_str_realloc = (char*)realloc(
            end_str,
            sizeof(char) * (str_len + 1));
    }
    if ( end_str_realloc != NULL )
    {
        end_str = end_str_realloc;
    }

    end_str = end_str_realloc;
    end_str[str_len] = '\0';

    Nst_StrObj *val_obj = STR(nst_new_string(end_str, str_len, true));
    nst_hash_obj(OBJ(val_obj));

    *tok = nst_new_token_value(start, cursor.pos, NST_TT_VALUE, OBJ(val_obj));
}

void nst_add_lines(Nst_SourceText* text)
{
    char *text_p = text->text;
    char **starts = (char **)calloc(100, sizeof(char *));
    if ( starts == NULL )
    {
        text->lines = NULL;
        text->line_count = 0;
        return;
    }

    starts[0] = text_p;
    size_t line_count = 1;

    // normalize line endings

    // if the file contains \n, then \n doesn't change and \r\n becomes just \n
    // if the file only contains \r, it is replaced with \n

    bool remove_r = false;
    for ( size_t i = 0, n = text->len; i < n; i++ )
    {
        if ( text_p[i] == '\n' )
        {
            remove_r = true;
            break;
        }
    }

    size_t offset = 0;
    for ( size_t i = 0, n = text->len; i < n; i++ )
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
    for ( size_t i = 0, n = text->len; i < n; i++ )
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
            void *temp = realloc(starts, i + 100);
            if ( temp == NULL )
            {
                free(starts);
                text->lines = NULL;
                text->line_count = 0;
                return;
            }
            starts = (char **)temp;
        }

        starts[line_count - 1] = text_p + i + 1;
    }

    text->lines = starts;
    text->line_count = line_count;
}

void nst_normalize_encoding(Nst_SourceText *text,
                            bool            is_cp1252,
                            Nst_Error      *error)
{
    unsigned char *text_p = (unsigned char *)text->text;
    size_t ch_count = 0;
    size_t i = 0;
    size_t n = text->len;

    if ( is_cp1252 )
    {
        goto fix_encoding;
    }

    for ( ; i < n; i++ )
    {
        if ( text_p[i] > 0x7f )
        {
            int offset = nst_check_utf8_bytes(text_p + i, n - i);
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
    return;

fix_encoding:
    for ( ; i < n; i++ )
    {
        if ( text_p[i] > 0x7f )
        {
            ch_count++;
        }
    }

    char *new_text = (char *)calloc(n + ch_count * 3 + 1, sizeof(char));
    if ( new_text == NULL )
    {
        return;
    }

    char *utf8_ptr = new_text;
    long line = 0;
    long col = 0;
    for ( i = 0; i < n; i++ )
    {
        if ( text_p[i] <= 0x7f )
        {
            if ( text_p[i] == '\n' )
            {
                line++;
                col = 0;
            }

            *utf8_ptr++ = text_p[i];
            continue;
        }
        int offset = nst_cp1252_to_utf8(utf8_ptr, text_p[i]);

        if ( offset == -1 )
        {
            free(new_text);
            Nst_Pos pos = { line, col, text };
            _NST_SET_RAW_VALUE_ERROR(
                error,
                pos, pos,
                "invalid cp1252 byte found");
            return;
        }

        utf8_ptr += offset;
        col++;
    }
    text->len = utf8_ptr - new_text;
    free(text->text);
    text->text = new_text;
}

int nst_check_utf8_bytes(unsigned char *byte, size_t len)
{
    int n = 0;

    if ( *byte <= 0b01111111 )
    {
        return 1;
    }
    else if ( *byte >= 0b11110000 && *byte <= 0b11110111 )
    {
        if ( len < 4 )
        {
            return -1;
        }
        n = 3;
    }
    else if ( *byte >= 0b11100000 && *byte <= 0b11101111 )
    {
        if ( len < 3 )
        {
            return -1;
        }
        n = 2;
    }
    else if ( *byte >= 0b11000000 && *byte <= 0b11011111 )
    {
        if ( len < 2 )
        {
            return -1;
        }
        n = 1;
    }
    else
    {
        return -1;
    }

    for ( int i = 0; i < n; i++ )
    {
        if ( *(++byte) < 0b10000000 || *byte > 0b10111111 )
        {
            return -1;
        }
    }
    return n + 1;
}

int nst_cp1252_to_utf8(char *str, char byte)
{
    if ( (unsigned char)byte <= 0x7f )
    {
        *str = byte;
        return 1;
    }

    unsigned char b1, b2, b3;
    switch ( (unsigned char)byte )
    {
    case 0x80: b1 = 0xe2; b2 = 0x82; b3 = 0xac; break;
    case 0x81: return -1;
    case 0x82: b1 = 0xe2; b2 = 0x80; b3 = 0x9a; break;
    case 0x83: b1 = 0xc6; b2 = 0x92; b3 = 0x00; break;
    case 0x84: b1 = 0xe2; b2 = 0x80; b3 = 0x9e; break;
    case 0x85: b1 = 0xe2; b2 = 0x80; b3 = 0xa6; break;
    case 0x86: b1 = 0xe2; b2 = 0x80; b3 = 0xa0; break;
    case 0x87: b1 = 0xe2; b2 = 0x80; b3 = 0xa1; break;
    case 0x88: b1 = 0xcb; b2 = 0x86; b3 = 0x00; break;
    case 0x89: b1 = 0xe2; b2 = 0x80; b3 = 0xb0; break;
    case 0x8a: b1 = 0xc5; b2 = 0xa0; b3 = 0x00; break;
    case 0x8b: b1 = 0xe2; b2 = 0x80; b3 = 0xb9; break;
    case 0x8c: b1 = 0xc5; b2 = 0x92; b3 = 0x00; break;
    case 0x8d: return -1;
    case 0x8e: b1 = 0xc5; b2 = 0xbd; b3 = 0x00; break;
    case 0x8f: return -1;
    case 0x90: return -1;
    case 0x91: b1 = 0xe2; b2 = 0x80; b3 = 0x98; break;
    case 0x92: b1 = 0xe2; b2 = 0x80; b3 = 0x99; break;
    case 0x93: b1 = 0xe2; b2 = 0x80; b3 = 0x9c; break;
    case 0x94: b1 = 0xe2; b2 = 0x80; b3 = 0x9d; break;
    case 0x95: b1 = 0xe2; b2 = 0x80; b3 = 0xa2; break;
    case 0x96: b1 = 0xe2; b2 = 0x80; b3 = 0x93; break;
    case 0x97: b1 = 0xe2; b2 = 0x80; b3 = 0x94; break;
    case 0x98: b1 = 0xcb; b2 = 0x9c; b3 = 0x00; break;
    case 0x99: b1 = 0xe2; b2 = 0x84; b3 = 0xa2; break;
    case 0x9a: b1 = 0xc5; b2 = 0xa1; b3 = 0x00; break;
    case 0x9b: b1 = 0xe2; b2 = 0x80; b3 = 0xba; break;
    case 0x9c: b1 = 0xc5; b2 = 0x93; b3 = 0x00; break;
    case 0x9d: return -1;
    case 0x9e: b1 = 0xc5; b2 = 0xbe; b3 = 0x00; break;
    case 0x9f: b1 = 0xc5; b2 = 0xb8; b3 = 0x00; break;
    case 0xa0: b1 = 0xc2; b2 = 0xa0; b3 = 0x00; break;
    case 0xa1: b1 = 0xc2; b2 = 0xa1; b3 = 0x00; break;
    case 0xa2: b1 = 0xc2; b2 = 0xa2; b3 = 0x00; break;
    case 0xa3: b1 = 0xc2; b2 = 0xa3; b3 = 0x00; break;
    case 0xa4: b1 = 0xc2; b2 = 0xa4; b3 = 0x00; break;
    case 0xa5: b1 = 0xc2; b2 = 0xa5; b3 = 0x00; break;
    case 0xa6: b1 = 0xc2; b2 = 0xa6; b3 = 0x00; break;
    case 0xa7: b1 = 0xc2; b2 = 0xa7; b3 = 0x00; break;
    case 0xa8: b1 = 0xc2; b2 = 0xa8; b3 = 0x00; break;
    case 0xa9: b1 = 0xc2; b2 = 0xa9; b3 = 0x00; break;
    case 0xaa: b1 = 0xc2; b2 = 0xaa; b3 = 0x00; break;
    case 0xab: b1 = 0xc2; b2 = 0xab; b3 = 0x00; break;
    case 0xac: b1 = 0xc2; b2 = 0xac; b3 = 0x00; break;
    case 0xad: b1 = 0xc2; b2 = 0xad; b3 = 0x00; break;
    case 0xae: b1 = 0xc2; b2 = 0xae; b3 = 0x00; break;
    case 0xaf: b1 = 0xc2; b2 = 0xaf; b3 = 0x00; break;
    case 0xb0: b1 = 0xc2; b2 = 0xb0; b3 = 0x00; break;
    case 0xb1: b1 = 0xc2; b2 = 0xb1; b3 = 0x00; break;
    case 0xb2: b1 = 0xc2; b2 = 0xb2; b3 = 0x00; break;
    case 0xb3: b1 = 0xc2; b2 = 0xb3; b3 = 0x00; break;
    case 0xb4: b1 = 0xc2; b2 = 0xb4; b3 = 0x00; break;
    case 0xb5: b1 = 0xc2; b2 = 0xb5; b3 = 0x00; break;
    case 0xb6: b1 = 0xc2; b2 = 0xb6; b3 = 0x00; break;
    case 0xb7: b1 = 0xc2; b2 = 0xb7; b3 = 0x00; break;
    case 0xb8: b1 = 0xc2; b2 = 0xb8; b3 = 0x00; break;
    case 0xb9: b1 = 0xc2; b2 = 0xb9; b3 = 0x00; break;
    case 0xba: b1 = 0xc2; b2 = 0xba; b3 = 0x00; break;
    case 0xbb: b1 = 0xc2; b2 = 0xbb; b3 = 0x00; break;
    case 0xbc: b1 = 0xc2; b2 = 0xbc; b3 = 0x00; break;
    case 0xbd: b1 = 0xc2; b2 = 0xbd; b3 = 0x00; break;
    case 0xbe: b1 = 0xc2; b2 = 0xbe; b3 = 0x00; break;
    case 0xbf: b1 = 0xc2; b2 = 0xbf; b3 = 0x00; break;
    case 0xc0: b1 = 0xc3; b2 = 0x80; b3 = 0x00; break;
    case 0xc1: b1 = 0xc3; b2 = 0x81; b3 = 0x00; break;
    case 0xc2: b1 = 0xc3; b2 = 0x82; b3 = 0x00; break;
    case 0xc3: b1 = 0xc3; b2 = 0x83; b3 = 0x00; break;
    case 0xc4: b1 = 0xc3; b2 = 0x84; b3 = 0x00; break;
    case 0xc5: b1 = 0xc3; b2 = 0x85; b3 = 0x00; break;
    case 0xc6: b1 = 0xc3; b2 = 0x86; b3 = 0x00; break;
    case 0xc7: b1 = 0xc3; b2 = 0x87; b3 = 0x00; break;
    case 0xc8: b1 = 0xc3; b2 = 0x88; b3 = 0x00; break;
    case 0xc9: b1 = 0xc3; b2 = 0x89; b3 = 0x00; break;
    case 0xca: b1 = 0xc3; b2 = 0x8a; b3 = 0x00; break;
    case 0xcb: b1 = 0xc3; b2 = 0x8b; b3 = 0x00; break;
    case 0xcc: b1 = 0xc3; b2 = 0x8c; b3 = 0x00; break;
    case 0xcd: b1 = 0xc3; b2 = 0x8d; b3 = 0x00; break;
    case 0xce: b1 = 0xc3; b2 = 0x8e; b3 = 0x00; break;
    case 0xcf: b1 = 0xc3; b2 = 0x8f; b3 = 0x00; break;
    case 0xd0: b1 = 0xc3; b2 = 0x90; b3 = 0x00; break;
    case 0xd1: b1 = 0xc3; b2 = 0x91; b3 = 0x00; break;
    case 0xd2: b1 = 0xc3; b2 = 0x92; b3 = 0x00; break;
    case 0xd3: b1 = 0xc3; b2 = 0x93; b3 = 0x00; break;
    case 0xd4: b1 = 0xc3; b2 = 0x94; b3 = 0x00; break;
    case 0xd5: b1 = 0xc3; b2 = 0x95; b3 = 0x00; break;
    case 0xd6: b1 = 0xc3; b2 = 0x96; b3 = 0x00; break;
    case 0xd7: b1 = 0xc3; b2 = 0x97; b3 = 0x00; break;
    case 0xd8: b1 = 0xc3; b2 = 0x98; b3 = 0x00; break;
    case 0xd9: b1 = 0xc3; b2 = 0x99; b3 = 0x00; break;
    case 0xda: b1 = 0xc3; b2 = 0x9a; b3 = 0x00; break;
    case 0xdb: b1 = 0xc3; b2 = 0x9b; b3 = 0x00; break;
    case 0xdc: b1 = 0xc3; b2 = 0x9c; b3 = 0x00; break;
    case 0xdd: b1 = 0xc3; b2 = 0x9d; b3 = 0x00; break;
    case 0xde: b1 = 0xc3; b2 = 0x9e; b3 = 0x00; break;
    case 0xdf: b1 = 0xc3; b2 = 0x9f; b3 = 0x00; break;
    case 0xe0: b1 = 0xc3; b2 = 0xa0; b3 = 0x00; break;
    case 0xe1: b1 = 0xc3; b2 = 0xa1; b3 = 0x00; break;
    case 0xe2: b1 = 0xc3; b2 = 0xa2; b3 = 0x00; break;
    case 0xe3: b1 = 0xc3; b2 = 0xa3; b3 = 0x00; break;
    case 0xe4: b1 = 0xc3; b2 = 0xa4; b3 = 0x00; break;
    case 0xe5: b1 = 0xc3; b2 = 0xa5; b3 = 0x00; break;
    case 0xe6: b1 = 0xc3; b2 = 0xa6; b3 = 0x00; break;
    case 0xe7: b1 = 0xc3; b2 = 0xa7; b3 = 0x00; break;
    case 0xe8: b1 = 0xc3; b2 = 0xa8; b3 = 0x00; break;
    case 0xe9: b1 = 0xc3; b2 = 0xa9; b3 = 0x00; break;
    case 0xea: b1 = 0xc3; b2 = 0xaa; b3 = 0x00; break;
    case 0xeb: b1 = 0xc3; b2 = 0xab; b3 = 0x00; break;
    case 0xec: b1 = 0xc3; b2 = 0xac; b3 = 0x00; break;
    case 0xed: b1 = 0xc3; b2 = 0xad; b3 = 0x00; break;
    case 0xee: b1 = 0xc3; b2 = 0xae; b3 = 0x00; break;
    case 0xef: b1 = 0xc3; b2 = 0xaf; b3 = 0x00; break;
    case 0xf0: b1 = 0xc3; b2 = 0xb0; b3 = 0x00; break;
    case 0xf1: b1 = 0xc3; b2 = 0xb1; b3 = 0x00; break;
    case 0xf2: b1 = 0xc3; b2 = 0xb2; b3 = 0x00; break;
    case 0xf3: b1 = 0xc3; b2 = 0xb3; b3 = 0x00; break;
    case 0xf4: b1 = 0xc3; b2 = 0xb4; b3 = 0x00; break;
    case 0xf5: b1 = 0xc3; b2 = 0xb5; b3 = 0x00; break;
    case 0xf6: b1 = 0xc3; b2 = 0xb6; b3 = 0x00; break;
    case 0xf7: b1 = 0xc3; b2 = 0xb7; b3 = 0x00; break;
    case 0xf8: b1 = 0xc3; b2 = 0xb8; b3 = 0x00; break;
    case 0xf9: b1 = 0xc3; b2 = 0xb9; b3 = 0x00; break;
    case 0xfa: b1 = 0xc3; b2 = 0xba; b3 = 0x00; break;
    case 0xfb: b1 = 0xc3; b2 = 0xbb; b3 = 0x00; break;
    case 0xfc: b1 = 0xc3; b2 = 0xbc; b3 = 0x00; break;
    case 0xfd: b1 = 0xc3; b2 = 0xbd; b3 = 0x00; break;
    case 0xfe: b1 = 0xc3; b2 = 0xbe; b3 = 0x00; break;
    case 0xff: b1 = 0xc3; b2 = 0xbf; b3 = 0x00; break;
    default: return -1;
    }

    *str++ = b1;
    *str++ = b2;
    if ( b3 == 0 )
    {
        return 2;
    }
    *str = b3;
    return 3;
}

static void parse_first_line(char  *text,
                             size_t len,
                             int   *opt_level,
                             bool  *force_cp1252,
                             bool  *no_default)
{
    if ( len < 3 || text[0] != '-' || text[1] != '-' || text[2] != '$' )
    {
        return;
    }

    char curr_opt[13];
    size_t i = 0;
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
