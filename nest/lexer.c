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

#define START_CH_SIZE 4 * sizeof(char)

#define RESIZE_STR(str, re_str, len, ch_size, ret_on_fail) \
    do { \
        if ( len + 1 == ch_size ) \
        { \
            chunk_size = (int)(chunk_size * 1.5); \
            re_str = (char *)realloc(str, sizeof(char) * ch_size); \
            if ( re_str == NULL ) \
            { \
                free(str); \
                errno = ENOMEM; \
                return ret_on_fail; \
            } \
            str = re_str; \
        } \
    } while ( false )

#define SET_ERROR(err_macro, start, end, message, err, ret_val) \
    Nst_Error *error = (Nst_Error *)malloc(sizeof(Nst_Error)); \
    if ( error == NULL ) \
    { \
        errno = ENOMEM; \
        return ret_val; \
    } \
    err_macro(error, start, end, message); \
    err = error

#define SET_INVALID_ESCAPE_ERROR do { \
    free(end_str); \
    SET_ERROR( \
        _NST_SET_SYNTAX_ERROR, \
        escape_start, \
        cursor.pos, \
        INVALID_ESCAPE, \
        *err, \
    ); \
    return; } while (0)

typedef struct LexerCursor {
    char *text;
    size_t len;
    Nst_Pos pos;
    long idx;
    char ch;
} LexerCursor;

static LexerCursor cursor;

inline static void advance();
inline static void go_back();
inline static char *add_while_in(const char *charset);

static void make_symbol(Nst_LexerToken **tok, Nst_Error **err);
static void make_num_literal(Nst_LexerToken **tok, Nst_Error **err);
static void make_ident(Nst_LexerToken **tok);
static void make_str_literal(Nst_LexerToken **tok, Nst_Error **err);

LList *nst_ftokenize(char *filename, char **text_buffer)
{
    FILE *file = fopen(filename, "r");
    if ( file == NULL )
    {
        printf("File \"%s\" not found\n", filename);
        return NULL;
    }

    char chunk[101] = {'\0'};
    char *text = (char *)calloc(1, sizeof(char));
    char *realloc_text = NULL;
    size_t str_len = 0;
    size_t size_read = 0;

    while ( (size_read = fread(chunk, sizeof(char), 100, file)) != 0 )
    {
        str_len += size_read;
        realloc_text = (char *)realloc(text, (str_len + 1) * sizeof(char));
        if ( realloc_text == NULL )
        {
            fclose(file);
            free(text);
            printf("Ran out of memory while reading the file\n");
            return NULL;
        }

        chunk[size_read] = '\0';
        text = realloc_text;
        text = strcat(text, chunk);
    }
    fclose(file);
    *text_buffer = text;
    return nst_tokenize(text, str_len, filename);
}

LList *nst_tokenize(char *text, size_t text_len, char *filename)
{
    Nst_LexerToken *tok = NULL;
    Nst_Error *err = NULL;
    LList *tokens = LList_new();

    cursor.idx = -1;
    cursor.ch = ' ';
    cursor.len = text_len;
    cursor.text = text;
    cursor.pos.col = -1;
    cursor.pos.filename = filename;
    cursor.pos.line = 0;
    cursor.pos.text = text;
    cursor.pos.text_len = text_len;

    advance();

    while ( cursor.idx < (long)cursor.len )
    {
        if ( cursor.ch == ' ' || cursor.ch == '\t' )
        {
            advance();
            continue;
        }
        else if ( strchr(_NST_DIGIT_CHARS "-", cursor.ch) != NULL )
            make_num_literal(&tok, &err);
        else if ( strchr(_NST_SYMBOL_CHARS, cursor.ch) != NULL )
            make_symbol(&tok, &err);
        else if ( strchr(_NST_LETTER_CHARS, cursor.ch) != NULL )
            make_ident(&tok);
        else if ( cursor.ch == '"' || cursor.ch == '\'' )
            make_str_literal(&tok, &err);
        else if ( cursor.ch == '\n' )
            tok = nst_new_token_noend(nst_copy_pos(cursor.pos), NST_TT_ENDL);
        else if ( cursor.ch == '\\' )
            advance();
        else
        {
            SET_ERROR(_NST_SET_SYNTAX_ERROR,
                      cursor.pos,
                      cursor.pos,
                      "invalid character",
                      err, NULL);
        }

        // When errno == ENOMEM means there wasn't enough
        // memory after a malloc, realloc or calloc call
        // and everything is freed
        if ( errno == ENOMEM )
        {
            if ( err != NULL )
                free(err);
            if ( tok != NULL )
                nst_destroy_token(tok);
            LList_destroy(tokens, (LList_item_destructor)nst_destroy_token);
            printf("Ran out of memory\n");
            return NULL;
        }

        if ( err != NULL )
        {
            nst_print_error(*err);
            free(err);
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

    if ( cursor.idx >= (long) cursor.len )
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

    if ( cursor.idx >= 0 )
        cursor.ch = cursor.text[cursor.idx];
}

inline static char *add_while_in(const char *charset)
{
    char *str = (char *)malloc(START_CH_SIZE);
    char *realloc_str = NULL;

    if ( str == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    size_t str_len = 0;
    size_t chunk_size = START_CH_SIZE;

    // While cursor.ch is in charset and there is text left to check
    while ( cursor.idx < (long) cursor.len && strchr(charset, cursor.ch) != NULL )
    {
        RESIZE_STR(str, realloc_str, str_len, chunk_size, NULL);

        str[str_len++] = cursor.ch;
        advance();
    }

    // I'm guaranteed to pass at least once in the loop
    // since I check the first character before calling this funciton
    go_back();

    // Makes the string the correct size
    if ( str_len < chunk_size )
        realloc_str = (char *)realloc(str, sizeof(char) * (str_len + 1));
    if ( realloc_str == NULL )
    {
        free(str);
        errno = ENOMEM;
        return NULL;
    }

    str = realloc_str;
    str[str_len] = '\0';
    return str;
}

static void make_symbol(Nst_LexerToken **tok, Nst_Error **err)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);
    char *symbol = add_while_in(_NST_SYMBOL_CHARS);
    Nst_Pos end = nst_copy_pos(cursor.pos);
    char *comment_start = strstr(symbol, "--");
    char *multcom_start = strstr(symbol, "-/");

    if ( comment_start == symbol )
    {
        while ( cursor.idx < (long)cursor.len && cursor.ch != '\n' )
        {
            if ( cursor.ch == '\\' )
            {
                advance();
                if ( cursor.ch == '\\' )
                    go_back();
            }
            advance();
        }
        go_back();
        free(symbol);
        return;
    }
    else if ( multcom_start == symbol )
    {
        bool can_close = false;
        bool was_closed = false;
        go_back();
        advance();
        while ( cursor.idx < (long)cursor.len )
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
        free(symbol);

        if ( !was_closed )
        {
            SET_ERROR(
                _NST_SET_SYNTAX_ERROR,
                start,
                cursor.pos,
                UNCLOSED_COMMENT,
                *err,
            );
        }

        return;
    }

    // Checks only up to the comment, the comment itself will be ignored in
    // the next call of 'make_symbol'
    if ( comment_start != NULL )
    {
        *comment_start = '\0';
    }
    // Not `else if` beacause `-/ --` would not start a multiline comment
    if ( multcom_start != NULL )
    {
        *multcom_start = '\0';
    }

    int token_type = nst_str_to_tok(symbol);
    char *symbol_end = symbol + strlen(symbol);

    while ( token_type == -1 )
    {
        go_back();
        end = nst_copy_pos(cursor.pos);
        --symbol_end;
        *symbol_end = '\0';
        token_type = nst_str_to_tok(symbol);
    }

    *tok = nst_new_token_noval(start, end, token_type);
}

static void make_num_literal(Nst_LexerToken **tok, Nst_Error **err)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);
    bool is_negative = false;

    // If there is a minus, there might be a negative number or a symbol
    if ( cursor.ch == '-' )
    {
        advance();
        // In case it's a symbol, make_symbol handles that
        if ( strchr(_NST_DIGIT_CHARS, cursor.ch) == NULL )
        {
            go_back();
            make_symbol(tok, err);
            return;
        }
        // Otherwise it's a negative number
        is_negative = true;
    }

    char *ltrl = add_while_in(_NST_DIGIT_CHARS);
    advance();

    // If there is no dot it's an integer
    if ( cursor.ch != '.' )
    {
        go_back();
        Nst_Pos end = nst_copy_pos(cursor.pos);

        // Don't really need str_end but it's required by the function
        char **str_end = NULL;
        Nst_Int value = strtoll(ltrl, str_end, 10);

        if ( errno == ERANGE )
        {
            if ( strcmp(ltrl, "9223372036854775808") == 0 && is_negative )
            {
                errno = 0;
                value = -9223372036854775807;
                --value; // parsers think that -9223372036854775808 is too large
            }
            else
            {
                free(ltrl);
                SET_ERROR(_NST_SET_SYNTAX_ERROR, start, end, INT_TOO_BIG, *err, );
                return;
            }
        }
        else if ( is_negative )
            value *= -1;
        *tok = nst_new_token_value(start, end, NST_TT_VALUE, nst_new_int(value));
        return;
    }

    // Otherwise it's a float

    // Get the number after '.'
    advance();
    char *fract_part = add_while_in(_NST_DIGIT_CHARS);
    Nst_Pos end = nst_copy_pos(cursor.pos);

    // If there is no number it's invalid
    if ( strlen(fract_part) == 0 )
    {
        free(ltrl);
        free(fract_part);
        SET_ERROR(_NST_SET_MEMORY_ERROR, start, end, BAD_REAL_LITEARL, *err, );
        return;
    }

    // Concatinate the strings and the dot (+ 2 includes '.' and '\0')
    size_t len = strlen(ltrl) + strlen(fract_part) + 2;
    char *fltrl = (char *)realloc(ltrl, len * sizeof(char));

    if ( fltrl == NULL )
    {
        free(ltrl);
        free(fract_part);
        errno = ENOMEM;
        return;
    }

    ltrl = fltrl;

    ltrl = strcat(ltrl, ".");
    ltrl = strcat(ltrl, fract_part);
    free(fract_part);

    // Don't really need str_end but it's required by the function
    char *str_end = NULL;
    Nst_Real value = strtod(ltrl, &str_end);

    if ( errno == ERANGE )
    {
        free(ltrl);
        SET_ERROR(_NST_SET_MEMORY_ERROR, start, end, REAL_TOO_BIG, *err, );
        return;
    }

    if ( is_negative ) value *= -1;
    *tok = nst_new_token_value(start, end, NST_TT_VALUE, nst_new_real(value));
    free(ltrl);
    return;
}

static void make_ident(Nst_LexerToken **tok)
{
    Nst_Pos start = nst_copy_pos(cursor.pos);
    char *str = add_while_in(_NST_LETTER_CHARS _NST_DIGIT_CHARS);
    Nst_Pos end = nst_copy_pos(cursor.pos);

    Nst_StrObj *val_obj = AS_STR(nst_new_string_raw(str, true));
    nst_hash_obj((Nst_Obj *)val_obj);

    *tok = nst_new_token_value(start, end, NST_TT_IDENT, (Nst_Obj *)val_obj);
}

static void make_str_literal(Nst_LexerToken **tok, Nst_Error **err)
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
        // the string is resized to fit at least one character
        RESIZE_STR(end_str, end_str_realloc, str_len, chunk_size,);

        if ( !escape )
        {
            if ( cursor.ch == '\n' && !allow_multiline )
            {
                free(end_str);
                SET_ERROR(
                    _NST_SET_SYNTAX_ERROR,
                    cursor.pos,
                    cursor.pos,
                    UNEXPECTED_NEWLINE,
                    *err,
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
        case 'f': end_str[str_len++] = '\f'; break;
        case 'n': end_str[str_len++] = '\n'; break;
        case 'r': end_str[str_len++] = '\r'; break;
        case 't': end_str[str_len++] = '\t'; break;
        case 'v': end_str[str_len++] = '\v'; break;
        case 'x':
            advance();
            if ( cursor.idx >= (long)cursor.len || cursor.ch == closing_ch )
                SET_INVALID_ESCAPE_ERROR;

            ch1 = (char)tolower(cursor.ch);
            advance();

            if ( cursor.idx >= (long)cursor.len || cursor.ch == closing_ch )
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
    Nst_Pos end = nst_copy_pos(cursor.pos);
    advance();

    if ( cursor.ch != closing_ch )
    {
        SET_ERROR(
            _NST_SET_SYNTAX_ERROR,
            start,
            end,
            UNCLOSED_STR_LITERAL,
            *err,
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

    Nst_StrObj *val_obj = AS_STR(nst_new_string(end_str, str_len, true));
    nst_hash_obj((Nst_Obj *)val_obj);

    *tok = nst_new_token_value(start, end, NST_TT_VALUE, (Nst_Obj *)val_obj);
}
