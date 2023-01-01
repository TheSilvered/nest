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
        _NST_EM_INVALID_ESCAPE \
    ); \
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
            _NST_SET_RAW_MEMORY_ERROR(error, start, end, _NST_EM_INT_TOO_BIG); \
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

static void add_lines(Nst_SourceText *text);

LList *nst_ftokenize(char *filename, Nst_SourceText *src_text, Nst_Error *error)
{
    FILE *file = fopen(filename, "r");
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
    text[size] = '\0';

    char *full_path;
    nst_get_full_path(filename, &full_path, NULL);

    src_text->text = text;
    src_text->len = str_len;
    src_text->path = full_path;

    add_lines(src_text);

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
        else if ( CH_IS_DEC(cursor.ch) || cursor.ch == '+' || cursor.ch == '-' )
            make_num_literal(&tok, error);
        else if ( CH_IS_SYMBOL(cursor.ch) )
            make_symbol(&tok, error);
        else if ( CH_IS_ALPHA(cursor.ch))
            make_ident(&tok);
        else if ( cursor.ch == '"' || cursor.ch == '\'' )
            make_str_literal(&tok, error);
        else if ( cursor.ch == '\n' )
            tok = nst_new_token_noend(nst_copy_pos(cursor.pos), NST_TT_ENDL);
        else if ( cursor.ch == '\\' )
            advance();
        else
        {
            _NST_SET_RAW_SYNTAX_ERROR(
                error,
                cursor.pos,
                cursor.pos,
                _NST_EM_INVALID_CHAR
            );
        }

        if ( error->occurred )
        {
            if ( tok != NULL )
                nst_destroy_token(tok);
            LList_destroy(tokens, (LList_item_destructor)nst_destroy_token);
            return NULL;
        }

        if ( tok != NULL )
            LList_append(tokens, tok, true);
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
        return;

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
        cursor.pos.line--;
}

inline static char *add_while_in(bool (*cond_func)(char),
                                 bool ignore_underscore,
                                 size_t *len)
{
    if ( len != NULL )
        *len = 0; // to set the value even if the function fails
    char *str;
    if ( !(cond_func(cursor.ch)) )
        return NULL;

    char *str_start = cursor.text + cursor.idx;
    size_t str_len = 1;
    advance();

    while ( cursor.idx < (long) cursor.len &&
            (cond_func(cursor.ch) || (ignore_underscore && cursor.ch == '_')) )
    {
        if ( !ignore_underscore || cursor.ch != '_' )
            str_len++;
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
                i--;
            else
                str[i] = *str_start;
            str_start++;
        }
    }
    else
        memcpy(str, str_start, str_len);

    str[str_len] = '\0';

    if ( len != NULL )
        *len = str_len;

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
            symbol[2] = cursor.ch;
        else
            go_back();
    }
    else
        go_back();

    if ( symbol[0] == '-' && symbol[1] == '-' )
    {
        while ( !CUR_AT_END && cursor.ch != '\n' )
        {
            if ( cursor.ch == '\\' )
            {
                advance();

                // Allows an even number of \ to escape the new line
                if ( cursor.ch == '\\' )
                    go_back();
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
                _NST_EM_OPEN_COMMENT
            );
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
            symbol[2] = '\0';
        else if ( symbol[1] != '\0' )
            symbol[1] = '\0';

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
                *tok = nst_new_token_value(start, end, NST_TT_VALUE, nst_new_byte(0));
                return;
            }

            advance();

            // they cannot be followed by a digit but can by an identifier
            // so 0b1019 is invalid but 0b01hello is valid
            if ( CH_IS_DEC(cursor.ch) )
            {
                _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_INT_LITERAL);
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
                _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_INT_LITERAL);
                return;
            }

            advance();

            // they cannot be followed by a word
            // 0xabc(hello) is fine but 0xabchello is not
            if ( CH_IS_ALPHA(cursor.ch) )
            {
                _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_INT_LITERAL);
                return;
            }
            go_back();
            end = nst_copy_pos(cursor.pos);

            value = strtoll(int_part, NULL, 16);

            CHECK_MIN(8000000000000000);
            free(int_part);

            // hex literals cannot have a b suffix as b is a digit, for hex bytex the 0h
            // prefix is used instead
            *tok = nst_new_token_value(
                start, end,
                NST_TT_VALUE,
                nst_new_int(value)
            );
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
                _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_BYTE_LITERAL);
                return;
            }

            advance();

            // they cannot be followed by a word
            // 0habc(hello) is fine but 0habchello is not
            if ( CH_IS_ALPHA(cursor.ch) )
            {
                _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_BYTE_LITERAL);
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
                nst_new_byte((Nst_Byte)(value & 0xff))
            );
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
                _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_INT_LITERAL);
                return;
            }

            advance();

            // they cannot be followed by a digit but can by an identifier
            // so 0o76459 is invalid but 0o53hello is valid
            if ( CH_IS_DEC(cursor.ch) )
            {
                _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_INT_LITERAL);
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
                goto real_lit;
            int_lit: go_back();

            end = nst_copy_pos(cursor.pos);
            value = strtoll(int_part, NULL, 10);

            CHECK_MIN(9223372036854775808);

            advance();
            goto byte;
        }

        byte: if ( cursor.ch == 'b' || cursor.ch == 'B' )
        {
            *tok = nst_new_token_value(
                start, cursor.pos,
                NST_TT_VALUE,
                nst_new_byte((Nst_Byte)(value & 0xff))
            );
        }
        else
        {
            go_back();
            *tok = nst_new_token_value(
                start, end,
                NST_TT_VALUE,
                nst_new_int(value)
            );
        }
        return;
    }

    int_part = add_while_in(is_dec, true, &len_int_part);
    advance();
    if ( cursor.ch != '.' )
        goto int_lit;

    real_lit: advance();
    char *frac_part = add_while_in(is_dec, true, &len_frac_part);
    if ( frac_part == NULL )
    {
        free(int_part);
        go_back();
        _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_REAL_LITERAL);
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
            _NST_SET_RAW_SYNTAX_ERROR(error, start, cursor.pos, _NST_EM_BAD_REAL_LITERAL);
            return;
        }

        end = nst_copy_pos(cursor.pos);
        //                                        9        .       26          e-    5       \0
        char *complete_lit = (char *)malloc(len_int_part + 1 + len_frac_part + 2 + len_exp + 1);
        if ( complete_lit == NULL )
        {
            errno = ENOMEM;
            return;
        }

        sprintf(complete_lit, "%s%s%s%s%s", int_part, ".", frac_part, exp_neg ? "e-" : "e+", exp );
        free(int_part);
        free(frac_part);
        free(exp);

        Nst_Real value = strtold(complete_lit, NULL);
        *tok = nst_new_token_value(
            start, end,
            NST_TT_VALUE,
            nst_new_real(value * sign)
        );
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

        Nst_Real value = strtold(complete_lit, NULL);
        *tok = nst_new_token_value(
            start, end,
            NST_TT_VALUE,
            nst_new_real(value * sign)
        );
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

    advance(); // still on '"' or '\''

    // while there is text to add and (the string has not ended or the end is inside and escape)
    while ( cursor.idx < (long) cursor.len && (cursor.ch != closing_ch || escape) )
    {
        if ( str_len + 1 == chunk_size )
        {
            chunk_size = (size_t)(chunk_size * 1.5);
            end_str_realloc = (char *)realloc(end_str, sizeof(char) * chunk_size);
            if ( end_str_realloc == NULL )
            {
                free(end_str);
                errno = ENOMEM;
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
                    _NST_EM_UNEXPECTED_NEWLINE
                );
                return;
            }
            else if ( cursor.ch == '\\' )
            {
                escape = true;
                escape_start = nst_copy_pos(cursor.pos);
            }
            else
                end_str[str_len++] = cursor.ch;
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
                SET_INVALID_ESCAPE_ERROR;

            ch1 = (char)tolower(cursor.ch);
            advance();

            if ( CUR_AT_END || cursor.ch == closing_ch )
                SET_INVALID_ESCAPE_ERROR;

            ch2 = (char)tolower(cursor.ch);

            if ( ch1 < '0' || ch1 > 'f'  ||
                (ch1 > '9' && ch1 < 'a') ||
                 ch2 < '0' || ch2 > 'f'  ||
                (ch2 > '9' && ch2 < 'a') )
                SET_INVALID_ESCAPE_ERROR;

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

            end_str[str_len++] = ((ch1 - '0') << 6) + ((ch2 - '0') << 3) + cursor.ch - '0';
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
        _NST_SET_RAW_SYNTAX_ERROR(
            error,
            start,
            error_end,
            _NST_EM_OPEN_STR_LITERAL
        );
    }

    if ( str_len < chunk_size )
        end_str_realloc = (char *)realloc(end_str, sizeof(char) * (str_len + 1));
    if ( end_str_realloc == NULL )
    {
        free(end_str);
        errno = ENOMEM;
        return;
    }

    end_str = end_str_realloc;
    end_str[str_len] = '\0';

    Nst_StrObj *val_obj = STR(nst_new_string(end_str, str_len, true));
    nst_hash_obj(OBJ(val_obj));

    *tok = nst_new_token_value(start, cursor.pos, NST_TT_VALUE, OBJ(val_obj));
}

static void add_lines(Nst_SourceText* text)
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

    // if the file contains \n, then \r\n becomes just \n
    // if the file only contains \r, it becomes \n

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
            text_p[i - offset] = text_p[i];
        else if ( remove_r )
            offset++;
        else
            text_p[i] = '\n';
    }

    text->len = text->len - offset;

    // now all lines end with \n
    for ( size_t i = 0, n = text->len; i < n; i++ )
    {
        if ( text_p[i] != '\n' )
            continue;

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
