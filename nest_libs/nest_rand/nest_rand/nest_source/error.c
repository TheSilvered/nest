#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

#define MAX_INT_CHAR_COUNT 21
//#define FANCY_ERRORS

#define C_BG_RED "\x1b[41m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_RESET "\x1b[0m"

Pos copy_pos(Pos pos)
{
    Pos new_pos = {
        pos.line,
        pos.col,
        pos.filename,
        pos.text,
        pos.text_len
    };

    return new_pos;
}

typedef struct PrintLineResult
{
    size_t len;
    size_t indent;
}
PrintLineResult;

PrintLineResult print_line(size_t lineno, char *text, size_t text_len, int64_t start, int64_t end)
{
    PrintLineResult result = { 0, 0 };
    size_t curr_line = 0;
    bool is_indentation = true;
#ifdef FANCY_ERRORS
    bool is_printing_error = false;
#endif

    size_t li = 0;
    for ( size_t i = 0; i < text_len; i++ )
    {
        if ( curr_line == lineno )
        {
            if ( text[i] != ' ' && text[i] != '\t' )
                is_indentation = false;

#ifdef FANCY_ERRORS
            if ( li == start )
            {
                is_printing_error = true;
                printf(C_BG_RED);
            }
#endif

            if ( !is_indentation )
            {
#ifdef FANCY_ERRORS
                char ch = text[i];
                if ( is_printing_error && ch == ' ' )
                    printf(C_RESET C_BG_RED "%c" C_RESET C_BG_RED, ch);
                else if ( is_printing_error && ch == '\n' )
                    printf(C_RESET C_BG_RED " \n" C_RESET C_BG_RED);
                else
#endif
                    printf("%c", text[i]);
                result.len++;
            }
            else
                result.indent++;

#ifdef FANCY_ERRORS
            if ( end != -1 && li == end )
            {
                is_printing_error = false;
                printf(C_RESET);
            }
#endif
        }
        li++;
        if ( text[i] == '\n' )
        {
            curr_line++;
            li = 0;
            if ( curr_line - 1 == lineno )
                break;
        }
    }

    if ( curr_line == lineno )
#ifdef FANCY_ERRORS
        if ( is_printing_error || li == start )
            printf(C_RESET C_BG_RED " \n" C_RESET);
        else
#endif
            printf("\n");
    else
        result.len--;

#ifdef FANCY_ERRORS
    printf(C_RESET);
#endif
    return result;
}

inline void print_repeat(char ch, size_t times)
{
    if ( times < 0 ) times = 1;
    for ( size_t i = 0; i < times; i++ )
        printf("%c", ch);
}

void print_position(Pos start, Pos end)
{
    assert(start.filename == end.filename);
#ifdef FANCY_ERRORS
    printf("File " C_GREEN "\"%s\"" C_RESET " at line %zi", start.filename, start.line + 1);
#else
    printf("File \"%s\" at line %zi", start.filename, start.line + 1);
#endif
    if ( start.line != end.line ) printf(" to %zi", end.line + 1);
    printf(":\n| ");

    assert(start.text == end.text);

    if ( start.line == end.line )
    {
        PrintLineResult res = print_line(start.line, start.text, start.text_len, start.col, end.col);
        printf("| ");
        print_repeat(' ', start.col - res.indent);
        print_repeat('^', end.col - start.col + 1);
        printf("\n");
        return;
    }

    PrintLineResult res = print_line(start.line, start.text, start.text_len, start.col, -1);
    printf("| ");
    print_repeat(' ', start.col - res.indent);

    print_repeat('^', res.len - start.col);
    printf("\n| ");
    if ( end.line - start.line > 1 )
#ifdef FACNY_ERRORS
        printf(C_BG_RED "    [...]" C_RESET "\n| ");
#else
        printf("    [...]\n| ");
#endif
    print_line(end.line, end.text, end.text_len, 0, end.col);
    printf("| ");
    print_repeat('^', end.col + 1);
    printf("\n");
    return;
}

void print_error(Nst_Error err)
{
    print_position(err.start, err.end);
#ifdef FANCY_ERRORS
    printf(C_YELLOW "%s" C_RESET " - %s\n", err.name, err.message);
#else
    printf("%s - %s\n", err.name, err.message);
#endif
}

void print_traceback(Nst_Traceback tb)
{
    while ( tb.positions->head != NULL )
    {
        Pos start = *(Pos *)LList_pop(tb.positions);
        Pos end   = *(Pos *)LList_pop(tb.positions);
        print_position(start, end);
    }

    if ( tb.error != NULL )
        print_error(*tb.error);
}

char *format_type_error(const char *format, char *type_name)
{
    char *buffer = malloc(strlen(format) + 5);
    if ( buffer == NULL )
        return NULL;
    sprintf(buffer, format, type_name);
    return buffer;
}

char *format_idx_error(const char *format, int64_t idx, size_t seq_len)
{
    char *buffer = malloc(strlen(format) + MAX_INT_CHAR_COUNT * 2 - 1);
    if ( buffer == NULL )
        return NULL;
    sprintf(buffer, format, idx, seq_len);
    return buffer;
}

char *format_fnf_error(const char *format, char *file_name)
{
    char *buffer = malloc(strlen(format) + strlen(file_name) + 1);
    if ( buffer == NULL )
        return NULL;
    sprintf(buffer, format, file_name);
    return buffer;
}

char *format_arg_error(const char *format, char *type_name, size_t idx)
{
    char *buffer = malloc(strlen(format) + 5 + MAX_INT_CHAR_COUNT);
    if ( buffer == NULL )
        return NULL;
    sprintf(buffer, format, idx, type_name);
    return buffer;
}
