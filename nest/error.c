#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "error.h"
#include "simple_types.h"

#define MAX_INT_CHAR_COUNT 21
#define FANCY_ERRORS

#define C_RED "\x1b[31m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_RESET "\x1b[0m"

Nst_Pos nst_copy_pos(Nst_Pos pos)
{
    Nst_Pos new_pos = {
        pos.line,
        pos.col,
        pos.filename,
        pos.text,
        pos.text_len
    };

    return new_pos;
}

Nst_Pos nst_no_pos()
{
    Nst_Pos new_pos = { 0, 0, NULL, NULL, 0 };
    return new_pos;
}

#if defined(_WIN32) || defined(WIN32)
#pragma warning( disable: 4100 )
#endif

static void print_repeat(char ch, size_t times)
{
    if ( times < 0 ) times = 1;
    for ( size_t i = 0; i < times; i++ )
        printf("%c", ch);
}

static void print_line(Nst_Pos *pos, long start_col, long end_col)
{
    printf("| ");

    long line_length = 0;
    long indent = 0;
    size_t curr_line = 0;
    bool is_indentation = true;
    char *text = pos->text;
    size_t text_len = pos->text_len;

    long lineno = pos->line;

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
            if ( li == start_col )
            {
                is_printing_error = true;
                printf(C_RED);
            }
#endif

            if ( !is_indentation )
            {
#ifdef FANCY_ERRORS
                char ch = text[i];
                if ( is_printing_error && ch == ' ' )
                    printf(C_RESET C_RED "%c" C_RESET C_RED, ch);
                else if ( is_printing_error && ch == '\n' )
                    printf(C_RESET C_RED " \n" C_RESET C_RED);
                else
#endif
                    printf("%c", text[i]);
                line_length++;
            }
            else
                ++indent;

#ifdef FANCY_ERRORS
            if ( end_col != -1 && li == end_col )
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

    // If there is a line feed at the end of the printed line curr_line == lineno + 1
    if ( curr_line == lineno )
        printf("\n");
    else
        line_length--;

    if ( end_col == -1 )
        end_col = line_length - 1;

    if ( end_col - start_col + 1 == line_length )
    {
#ifdef FANCY_ERRORS
        printf(C_RESET);
#endif
        return;
    }

#ifdef FANCY_ERRORS
    printf("| " C_RED);
    print_repeat(' ', start_col - indent);
    print_repeat('^', end_col - start_col + 1);
    printf(C_RESET "\n");
#else
    printf("| ");
    print_repeat(' ', start_col - indent);
    print_repeat('^', end_col - start_col + 1);
    printf("\n");
#endif
}

static void print_position(Nst_Pos start, Nst_Pos end)
{
    assert(start.filename == end.filename);
    assert(start.text == end.text);

#ifdef FANCY_ERRORS
    printf("File " C_GREEN "\"%s\"" C_RESET " at line %li",
           start.filename,
           start.line + 1);
#else
    printf("File \"%s\" at line %li",
        start.filename,
        start.line + 1);
#endif

    if ( start.line != end.line) printf(" to %li", end.line + 1 );
    printf(":\n");

    if ( start.line == end.line )
    {
        print_line(&start, start.col, end.col);
        return;
    }

    print_line(&start, start.col, -1);

    for (long i = 1, n = end.line - start.line; i < n; i++)
    {
        Nst_Pos mid_line_pos = {
            start.line + i,
            0,
            start.filename,
            start.text,
            start.text_len
        };

        print_line(&mid_line_pos, 0, -1);
    }

    print_line(&end, 0, end.col);
}

void nst_print_error(Nst_Error err)
{
    print_position(err.start, err.end);
#ifdef FANCY_ERRORS
    printf(C_YELLOW "%s" C_RESET " - %s\n", err.name, err.message);
#else
    printf("%s - %s\n", err.name, err.message);
#endif
}

void nst_print_traceback(Nst_Traceback tb)
{
    while ( tb.positions->head != NULL )
    {
        Nst_Pos *start = (Nst_Pos *)LList_pop(tb.positions);
        Nst_Pos *end   = (Nst_Pos *)LList_pop(tb.positions);
        print_position(*start, *end);
    }

    nst_print_error(tb.error);
}

char *_nst_format_error(const char *format, const char *format_args, ...)
{
    /*
    Format args types:
        s: string
        u: size_t
        i: Nst_Int
    */

    va_list args;
    va_start(args, format_args);

    size_t tot_size = strlen(format);
    char *arg_ptr = (char *)format_args;
    while ( *arg_ptr )
    {
        switch ( *arg_ptr )
        {
        case 's':
            tot_size += strlen(va_arg(args, char *));
            break;
        case 'u':
            va_arg(args, size_t);
            tot_size += MAX_INT_CHAR_COUNT;
            break;
        case 'i':
            va_arg(args, Nst_Int);
            tot_size += MAX_INT_CHAR_COUNT;
            break;
        default:
            break;
        }
        ++arg_ptr;
    }

    char *buffer = (char *)calloc(tot_size + 1, sizeof(char));
    if (buffer == NULL)
        return NULL;

    va_start(args, format_args);
    vsprintf(buffer, format, args);
    va_end(args);

    return buffer;
}
