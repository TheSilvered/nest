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
            re_str = realloc(str, sizeof(char) * ch_size); \
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
    Nst_Error *error = malloc(sizeof(Nst_Error)); \
    if ( error == NULL ) \
    { \
        errno = ENOMEM; \
        return ret_val; \
    } \
    err_macro(error, start, end, message); \
    err = error

#define SET_INVALID_ESCAPE_ERROR do { \
    free(end_str); \
    SET_ERROR(SET_SYNTAX_ERROR_INT, escape_start, cursor.pos, INVALID_ESCAPE, *err, ); \
    return; } while (0)

LList *ftokenize(char *filename);
LList *tokenize(char *text, size_t text_len, char *filename);

typedef struct LexerCursor {
    char *text;
    size_t len;
    Pos pos;
    long idx;
    char ch;
} LexerCursor;

static LexerCursor cursor;

inline static void advance();
inline static void go_back();
inline static char *add_while_in(char *charset);

static void make_symbol(Token **tok, Nst_Error **err);
static void make_num_literal(Token **tok, Nst_Error **err);
static void make_ident(Token **tok, Nst_Error **err);
static void make_str_literal(Token **tok, Nst_Error **err);

LList *ftokenize(char *filename)
{
    FILE *file = fopen(filename, "r");
    if ( file == NULL )
    {
        printf("File \"%s\" not found\n", filename);
        return NULL;
    }

    char chunk[101] = {'\0'};
    char *text = calloc(1, sizeof(char));
    char *realloc_text = NULL;
    size_t str_len = 0;
    size_t size_read = 0;

    while ( (size_read = fread(chunk, sizeof(char), 100, file)) != 0 )
    {
        str_len += size_read;
        realloc_text = realloc(text, (str_len + 1) * sizeof(char));
        if ( realloc_text == NULL )
        {
            fclose(file);
            free(text);
            printf("Ran out of memory\n");
            return NULL;
        }

        chunk[size_read] = '\0';
        text = realloc_text;
        text = strcat(text, chunk);
    }
    fclose(file);
    return tokenize(text, str_len, filename);
}

LList *tokenize(char *text, size_t text_len, char *filename)
{
    Token *tok = NULL;
    Nst_Error *err = NULL;
    LList *tokens = LList_new();

    advance();

    while ( cursor.idx < (long)cursor.len )
    {
        if ( cursor.ch == ' ' || cursor.ch == '\t' )
        {
            advance(&cursor);
            continue;
        }
        else if ( strchr(DIGIT_CHARS "-", cursor.ch) != NULL )
            make_num_literal(&tok, &err);
        else if ( strchr(SYMBOL_CHARS, cursor.ch) != NULL )
            make_symbol(&tok, &err);
        else if ( strchr(LETTER_CHARS, cursor.ch) != NULL )
            make_ident(&tok, &err);
        else if ( cursor.ch == '"' || cursor.ch == '\'' )
            make_str_literal(&tok, &err);
        else if ( cursor.ch == '\n' )
            tok = new_token_noend(copy_pos(cursor.pos), ENDL);
        else if ( cursor.ch == '\\' )
            advance();
        else
        {
            SET_ERROR(
                SET_SYNTAX_ERROR_INT, cursor.pos, cursor.pos, "invalid character",
                err, NULL
            );
        }

        // When errno == ENOMEM means there wasn't enough
        // memory after a malloc, realloc or calloc call
        // and everything is freed
        if ( errno == ENOMEM )
        {
            if ( err != NULL )
                free(err);
            if ( tok != NULL )
                destroy_token(tok);
            LList_destroy(tokens, destroy_token);
            printf("Ran out of memory\n");
            return NULL;
        }

        if ( err != NULL )
        {
            print_error(*err);
            free(err);
            if ( tok != NULL )
                destroy_token(tok);
            LList_destroy(tokens, destroy_token);
            return NULL;
        }

        if ( tok != NULL )
            LList_append(tokens, tok, true);
        tok = NULL;
        advance(&cursor);
    }

    LList_append(tokens, new_token_noend(cursor.pos, EOFILE), true);
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

inline static char *add_while_in(char *charset)
{
    char *str = malloc(START_CH_SIZE);
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
        advance(cursor);
    }

    // I'm guaranteed to pass at least once in the loop
    // since I check the first character before calling this funciton
    go_back(cursor);

    // Makes the string the correct size
    if ( str_len < chunk_size )
        realloc_str = realloc(str, sizeof(char) * (str_len + 1));
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

static void make_symbol(Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor.pos);
    char *symbol = add_while_in(SYMBOL_CHARS);
    Pos end = copy_pos(cursor.pos);
    char *comment_start = strstr(symbol, "--");
    char *multcom_start = strstr(symbol, "-/");

    if ( comment_start == symbol )
    {
        while ( cursor.idx < (long)cursor.len && cursor.ch != '\n' )
        {
            advance(cursor);
            if ( cursor.ch == '\\' )
            {
                advance(cursor);
                advance(cursor);
            }
        }
        go_back(cursor);
        free(symbol);
        return;
    }
    else if ( multcom_start == symbol )
    {
        bool can_close = false;
        while ( cursor.idx < (long)cursor.len )
        {
            advance(cursor);
            if ( can_close && cursor.ch == '-' )
            {
                advance(cursor);
                break;
            }

            can_close = cursor.ch == '/';
        }
        go_back(cursor);
        free(symbol);
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

    int token_type = str_to_tok(symbol);
    char *symbol_end = symbol + strlen(symbol);

    while ( token_type == -1 )
    {
        go_back(cursor);
        end = copy_pos(cursor.pos);
        --symbol_end;
        *symbol_end = '\0';
        token_type = str_to_tok(symbol);
    }

    *tok = new_token_noval(start, end, token_type);
}

static void make_num_literal(Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor.pos);
    bool is_negative = false;

    // If there is a minus, there might be a negative number or a symbol
    if ( cursor.ch == '-' )
    {
        advance(cursor);
        // In case it's a symbol, make_symbol handles that
        if ( strchr(DIGIT_CHARS, cursor.ch) == NULL )
        {
            go_back(cursor);
            make_symbol(tok, err);
            return;
        }
        // Otherwise it's a negative number
        is_negative = true;
    }

    char *ltrl = add_while_in(DIGIT_CHARS);
    advance(cursor);

    // If there is no dot it's an integer
    if ( cursor.ch != '.' )
    {
        go_back(cursor);
        Pos end = copy_pos(cursor.pos);
        Nst_int *value = malloc(sizeof(Nst_int));

        if ( value == NULL )
        {
            free(ltrl);
            errno = ENOMEM;
            return;
        }

        // Don't really need str_end but it's required by the function
        char **str_end = NULL;
        *value = strtoll(ltrl, str_end, 10);

        if ( errno == ERANGE )
        {
            free(ltrl);
            free(value);
            SET_ERROR(SET_SYNTAX_ERROR_INT, start, end, INT_TOO_BIG, *err, );
            return;
        }

        if ( is_negative ) *value *= -1;
        *tok = new_token_value(start, end, N_INT, make_obj_free(value, nst_t_int));
        return;
    }

    // Otherwise it's a float

    // Get the number after '.'
    advance(cursor);
    char *fract_part = add_while_in(DIGIT_CHARS);
    Pos end = copy_pos(cursor.pos);

    // If there is no number it's invalid
    if ( strlen(fract_part) == 0 )
    {
        free(ltrl);
        free(fract_part);
        SET_ERROR(SET_MEMORY_ERROR_INT, start, end, BAD_REAL_LITEARL, *err, );
        return;
    }

    // Concatinate the strings and the dot (+ 2 includes '.' and '\0')
    size_t len = strlen(ltrl) + strlen(fract_part) + 2;
    char *fltrl = realloc(ltrl, len * sizeof(char));

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

    // Evaluate the value
    Nst_real *value = malloc(sizeof(Nst_real));
    if ( value == NULL )
    {
        free(ltrl);
        errno = ENOMEM;
        return;
    }

    // Don't really need str_end but it's required by the function
    char **str_end = NULL;
    *value = strtod(ltrl, str_end);

    if ( errno == ERANGE )
    {
        free(ltrl);
        free(value);
        SET_ERROR(SET_MEMORY_ERROR_INT, start, end, REAL_TOO_BIG, *err, );
        return;
    }

    if ( is_negative ) *value *= -1;
    *tok = new_token_value(start, end, N_REAL, make_obj_free(value, nst_t_real));
    free(ltrl);
    return;
}

static void make_ident(Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor.pos);
    char *str = add_while_in(LETTER_CHARS DIGIT_CHARS);
    Pos end = copy_pos(cursor.pos);

    Nst_string *value = malloc(sizeof(Nst_string));
    if ( value == NULL )
    {
        free(str);
        errno = ENOMEM;
        return;
    }

    value->value = str;
    value->len = strlen(str);

    Nst_Obj *val_obj = new_str_obj(value);
    hash_obj(val_obj);

    *tok = new_token_value(start, end, IDENT, val_obj);
}

static void make_str_literal(Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor.pos);
    Pos escape_start = copy_pos(cursor.pos);
    char closing_ch = cursor.ch;
    bool allow_multiline = cursor.ch == '"';
    bool escape = false;
    char escape_str = 0;

    char *end_str = malloc(START_CH_SIZE);
    char *end_str_realloc = NULL;

    if ( end_str == NULL )
    {
        errno = ENOMEM;
        return;
    }

    size_t str_len = 0;
    size_t chunk_size = START_CH_SIZE;

    advance(cursor); // still on '"' or '\''

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
                SET_ERROR(SET_SYNTAX_ERROR_INT, cursor.pos, cursor.pos, UNEXPECTED_NEWLINE, *err, );
                return;
            }
            else if ( cursor.ch == '\\' )
            {
                escape = true;
                escape_start = copy_pos(cursor.pos);
            }
            else
                end_str[str_len++] = cursor.ch;
            advance(cursor);
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
            advance(cursor);
            if ( cursor.idx >= (long)cursor.len || cursor.ch == closing_ch )
                SET_INVALID_ESCAPE_ERROR;

            char ch1 = tolower(cursor.ch);
            advance(cursor);

            if ( cursor.idx >= (long)cursor.len || cursor.ch == closing_ch )
                SET_INVALID_ESCAPE_ERROR;

            char ch2 = tolower(cursor.ch);

            if ( (ch1 < '0' || ch1 > '7' || ch2 < '0' || (ch2 > '9' && ch2 < 'a') || ch2 > 'f') )
                SET_INVALID_ESCAPE_ERROR;

            char result = (ch1 - '0') * 16 + (ch2 > '9' ? ch2 - 'a' + 10 : ch2 - '0');
            if ( !result )
                SET_INVALID_ESCAPE_ERROR;

            end_str[str_len++] = result;
            break;
        default:
            SET_INVALID_ESCAPE_ERROR;
        }

        escape = false;
        advance(cursor);
    }

    Pos end = copy_pos(cursor.pos);

    if ( cursor.ch != closing_ch )
    {
        SET_ERROR(SET_SYNTAX_ERROR_INT, start, end, UNCLOSED_STR_LITERAL, *err, );
    }

    if ( str_len < chunk_size )
        end_str_realloc = realloc(end_str, sizeof(char) * (str_len + 1));
    if ( end_str_realloc == NULL )
    {
        free(end_str);
        errno = ENOMEM;
        return;
    }

    end_str = end_str_realloc;
    end_str[str_len] = '\0';

    Nst_string *value = malloc(sizeof(Nst_string));
    if ( value == NULL )
    {
        free(end_str);
        errno = ENOMEM;
        return;
    }

    value->value = end_str;
    value->len = str_len;

    Nst_Obj *val_obj = new_str_obj(value);
    hash_obj(val_obj);

    *tok = new_token_value(start, end, STRING, val_obj);
}
