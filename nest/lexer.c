#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "error.h"
#include "tokens.h"
#include "nst_types.h"

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

LList *ftokenize(char *filename);
LList *tokenize(char *text, size_t text_len, char *filename);

typedef struct LexerCursor {
    char *text;
    size_t len;
    Pos pos;
    long idx;
    char ch;
} LexerCursor;


inline static void advance(LexerCursor *cursor);
inline static void go_back(LexerCursor *cursor);
inline static char *add_while_in(LexerCursor *cursor, char *charset);

static void make_parens(LexerCursor *cursor, Token **tok, Nst_Error **err);
static void make_symbol(LexerCursor *cursor, Token **tok, Nst_Error **err);
static void make_num_literal(LexerCursor *cursor, Token **tok, Nst_Error **err);
static void make_ident(LexerCursor *cursor, Token **tok, Nst_Error **err);
static void make_str_literal(LexerCursor *cursor, Token **tok, Nst_Error **err);

LList *ftokenize(char *filename)
{
    FILE *file = fopen(filename, "r");
    if ( file == NULL )
    {
        printf("File not found\n");
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
    LexerCursor cursor = {
        .text = text,
        .len = text_len,
        .idx = -1,
        .ch = ' ',
        .pos = {
            .line = 0,
            .col = -1,
            .filename = filename,
            .text = text,
            .text_len = text_len
        }
    };

    advance(&cursor);

    while ( cursor.idx < (long)cursor.len )
    {
        if ( cursor.ch == ' ' || cursor.ch == '\t' )
        {
            advance(&cursor);
            continue;
        }
        else if ( strchr(PAREN_CHARS, cursor.ch) != NULL )
            make_parens(&cursor, &tok, &err);
        else if ( strchr(DIGIT_CHARS "-", cursor.ch) != NULL )
            make_num_literal(&cursor, &tok, &err);
        else if ( strchr(SYMBOL_CHARS, cursor.ch) != NULL )
            make_symbol(&cursor, &tok, &err);
        else if ( strchr(LETTER_CHARS, cursor.ch) != NULL )
            make_ident(&cursor, &tok, &err);
        else if ( cursor.ch == '"' || cursor.ch == '\'' )
            make_str_literal(&cursor, &tok, &err);
        else if ( cursor.ch == '\n' )
            tok = new_token_noend(copy_pos(cursor.pos), ENDL);
        else if ( cursor.ch == ',' )
            tok = new_token_noend(copy_pos(cursor.pos), COMMA);
        else if ( cursor.ch == '$' )
            tok = new_token_noend(copy_pos(cursor.pos), LEN);
        else if ( cursor.ch == '\\' )
            advance(&cursor);
        else
        {
            SET_ERROR(
                SYNTAX_ERROR, cursor.pos, cursor.pos, "invalid character",
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

inline static void advance(LexerCursor *cursor)
{
    cursor->idx++;
    cursor->pos.col++;

    if ( cursor->idx >= (long) cursor->len )
        return;

    if ( cursor->ch == '\n' )
    {
        cursor->pos.col = 0;
        cursor->pos.line++;
    }

    cursor->ch = cursor->text[cursor->idx];
}

inline static void go_back(LexerCursor *cursor)
{
    cursor->idx--;
    cursor->pos.col--;

    if ( cursor->idx >= 0 )
        cursor->ch = cursor->text[cursor->idx];
}

inline static char *add_while_in(LexerCursor *cursor, char *charset)
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
    while ( cursor->idx < (long) cursor->len && strchr(charset, cursor->ch) != NULL )
    {
        RESIZE_STR(str, realloc_str, str_len, chunk_size, NULL);

        str[str_len++] = cursor->ch;
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

static void make_parens(LexerCursor *cursor, Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor->pos);
    switch ( cursor->ch )
    {
    // Normal parenthesis are fine
    case '(':
        *tok = new_token_noend(start, L_PAREN);
        break;
    case ')':
        *tok = new_token_noend(start, R_PAREN);
        break;
    case '[':
        *tok = new_token_noend(start, L_BRACKET);
        break;
    case ']':
        *tok = new_token_noend(start, R_BRACKET);
        break;
    case '{':
        *tok = new_token_noend(start, L_BRACE);
        break;
    // The problem is with vector braces wich have more than one character
    case '}':
        advance(cursor);
        if ( cursor->ch == '>' )
            *tok = new_token_noval(start, copy_pos(cursor->pos), R_VBRACE);
        else
        {
            go_back(cursor);
            *tok = new_token_noend(start, R_BRACE);
        }
        break;
    case '<':
        advance(cursor);
        if ( cursor->ch == '{' )
            *tok = new_token_noval(start, copy_pos(cursor->pos), L_VBRACE);
        else
        {
            go_back(cursor);
            make_symbol(cursor, tok, err);
        }
        break;
    default:
        break;
    }
}

static void make_symbol(LexerCursor *cursor, Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor->pos);
    char *symbol = add_while_in(cursor, SYMBOL_CHARS);
    Pos end = copy_pos(cursor->pos);

    // Ignores the comment
    if ( strstr(symbol, "--") != NULL )
    {
        while ( cursor->idx < (long)cursor->len && cursor->ch != '\n' )
        {
            advance(cursor);
            if ( cursor->ch == '\\' )
            {
                advance(cursor);
                advance(cursor);
            }
        }
        go_back(cursor);
        return;
    }

    int token_type = str_to_tok(symbol);

    // str_to_tok returns -1 when no match is found
    if ( token_type == -1 )
    {
        free(symbol);
        SET_ERROR(SYNTAX_ERROR, start, end, INVALID_SYMBOL, *err, );
        return;
    }

    *tok = new_token_noval(start, end, token_type);
}

static void make_num_literal(LexerCursor *cursor, Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor->pos);
    bool is_negative = false;

    // If there is a minus, there might be a negative number or a symbol
    if ( cursor->ch == '-' )
    {
        advance(cursor);
        // In case it's a symbol, make_symbol handles that
        if ( strchr(DIGIT_CHARS, cursor->ch) == NULL )
        {
            go_back(cursor);
            make_symbol(cursor, tok, err);
            return;
        }
        // Otherwise it's a negative number
        is_negative = true;
    }
    
    char *ltrl = add_while_in(cursor, DIGIT_CHARS);
    advance(cursor);
    
    // If there is no dot it's an integer
    if ( cursor->ch != '.' )
    {
        go_back(cursor);
        Pos end = copy_pos(cursor->pos);
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
            SET_ERROR(SYNTAX_ERROR, start, end, INT_TOO_BIG, *err, );
            return;
        }

        if ( is_negative ) *value *= -1;
        *tok = new_token_value(start, end, INT, value);
        return;
    }

    // Otherwise it's a float

    // Get the number after '.'
    advance(cursor);
    char *fract_part = add_while_in(cursor, DIGIT_CHARS);
    Pos end = copy_pos(cursor->pos);
    
    // If there is no number it's invalid
    if ( strlen(fract_part) == 0 )
    {
        free(ltrl);
        free(fract_part);
        SET_ERROR(MEMORY_ERROR, start, end, BAD_REAL_LITEARL, *err, );
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
        SET_ERROR(MEMORY_ERROR, start, end, REAL_TOO_BIG, *err, );
        return;
    }

    if ( is_negative ) *value *= -1;
    *tok = new_token_value(start, end, REAL, value);
    free(ltrl);
    return;
}

static void make_ident(LexerCursor *cursor, Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor->pos);
    char *str = add_while_in(cursor, LETTER_CHARS DIGIT_CHARS);
    Pos end = copy_pos(cursor->pos);

    Nst_string *value = malloc(sizeof(Nst_string));
    if ( value == NULL )
    {
        free(str);
        errno = ENOMEM;
        return;
    }

    value->value = str;
    value->len = strlen(str);

    *tok = new_token_value(start, end, IDENT, value);
}

static void make_str_literal(LexerCursor *cursor, Token **tok, Nst_Error **err)
{
    Pos start = copy_pos(cursor->pos);
    Pos escape_start = copy_pos(cursor->pos);
    char closing_ch = cursor->ch;
    bool allow_multiline = cursor->ch == '"';
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
    while ( cursor->idx < (long) cursor->len && (cursor->ch != closing_ch || escape) )
    {
        // the string is resized to fit at least one character
        RESIZE_STR(end_str, end_str_realloc, str_len, chunk_size,);

        if ( !escape )
        {
            if ( cursor->ch == '\n' && !allow_multiline )
            {
                free(end_str);
                SET_ERROR(SYNTAX_ERROR, cursor->pos, cursor->pos, UNEXPECTED_NEWLINE, *err, );
                return;
            }
            else if ( cursor->ch == '\\' )
            {
                escape = true;
                escape_start = copy_pos(cursor->pos);
            }
            else
                end_str[str_len++] = cursor->ch;
            advance(cursor);
            continue;
        }

        // If there is an escape sequence
        switch ( cursor->ch )
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
        default:
            free(end_str);
            SET_ERROR(SYNTAX_ERROR, escape_start, cursor->pos, INVALID_ESCAPE, *err, );
            return;
        }

        escape = false;
        advance(cursor);
    }

    Pos end = copy_pos(cursor->pos);
    
    if ( cursor->ch != closing_ch )
    {
        SET_ERROR(SYNTAX_ERROR, start, end, UNCLOSED_STR_LITERAL, *err, );
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

    *tok = new_token_value(start, end, STRING, value);
}
