#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "error.h"
#include "simple_types.h"

#define MAX_INT_CHAR_COUNT 21

#define C_RED "\x1b[31m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_CYAN "\x1b[96m"
#define C_RESET "\x1b[0m"

static bool use_color = true;

void nst_set_color(bool color)
{
    use_color = color;
}

Nst_Pos nst_copy_pos(Nst_Pos pos)
{
    Nst_Pos new_pos = {
        pos.line,
        pos.col,
        pos.text
    };

    return new_pos;
}

Nst_Pos nst_no_pos()
{
    Nst_Pos new_pos = { 0, 0, NULL };
    return new_pos;
}

#if defined(_WIN32) || defined(WIN32)
#pragma warning( disable: 4100 )
#endif

static void print_repeat(char ch, long times)
{
    if ( times < 0 ) times = 0;
    for ( long i = 0; i < times; i++ )
        putc(ch, stdout);
}

static long get_indent(Nst_SourceText *text, long lineno)
{
    long indent = 0;

    for ( char *p = text->lines[lineno]; *p != '\n' && *p != '\0'; p++)
    {
        if ( *p == ' ' || *p == '\t' )
            indent++;
        else
            return indent;
    }
    return indent;
}

static void print_line(Nst_Pos *pos, long start_col, long end_col, int keep_indent, int max_line)
{
    long line_length = 0;
    int lineno_len = 0;
    long lineno = pos->line;
    char *text = pos->text->lines[lineno];
    long indent = get_indent(pos->text, lineno) - keep_indent;

    if ( keep_indent == -1 )
    {
        indent--;
        keep_indent = indent;
    }

    if ( start_col == 0 )
        start_col = keep_indent;

    if ( max_line == 0 )
        max_line = lineno + 1;

    while ( max_line > 0 )
    {
        lineno_len++;
        max_line /= 10;
    }

    if ( use_color )
        printf(C_CYAN " %*li" C_RESET " | ", lineno_len, lineno + 1);
    else
        printf(" %*li | ", lineno_len, lineno + 1);

    for ( long i = keep_indent; text[i] != '\n' && text[i] != '\0'; i++ )
    {
        if ( i == start_col && use_color )
            printf(C_RED);

        putc(text[i], stdout);
        line_length++;

        if ( use_color && i == end_col )
            printf(C_RESET);
    }
    printf("\n");

    if ( end_col == -1 )
        end_col = indent + line_length - 1;
    else
        end_col -= keep_indent;

    start_col -= keep_indent;

    if ( end_col - start_col + 1 == line_length )
    {
        if ( use_color )
            printf(C_RESET);
        return;
    }

    if ( use_color )
    {
        print_repeat(' ', lineno_len);
        printf(C_RESET "  | " C_RED);
        print_repeat(' ', start_col);
        print_repeat('^', end_col - start_col + 1);
        printf(C_RESET "\n");
    }
    else
    {
        print_repeat(' ', lineno_len);
        printf("  | ");
        print_repeat(' ', start_col);
        print_repeat('^', end_col - start_col + 1);
        printf("\n");
    }
}

static void print_position(Nst_Pos start, Nst_Pos end)
{
    assert(start.text == end.text);

    if ( start.text == NULL )
        return;

    if ( use_color )
        printf("File " C_GREEN "\"%s\"" C_RESET " at ", start.text->path);
    else
        printf("File \"%s\" at ", start.text->path);

    if ( start.line != end.line )
    {
        if ( use_color )
            printf("lines " C_CYAN "%li" C_RESET " to " C_CYAN "%li" C_RESET,
                start.line + 1,
                end.line + 1 );
        else
            printf("lines %li to %li",
                start.line + 1,
                end.line + 1 );
    }
    else
    {
        if ( use_color )
            printf("line " C_CYAN "%li" C_RESET, start.line + 1 );
        else
            printf("line %li", start.line + 1 );
    }
    printf(":\n");

    if ( start.line == end.line )
    {
        print_line(&start, start.col, end.col, -1, 0);
        return;
    }

    long min_indent = get_indent(start.text, start.line);
    for ( long i = start.line + 1; i <= end.line; i++ )
    {
        long indent = get_indent(start.text, i);
        if ( indent < min_indent )
            min_indent = indent;
    }

    print_line(&start, start.col, -1, min_indent, end.line + 1);

    for ( long i = 1, n = end.line - start.line; i < n; i++ )
    {
        Nst_Pos mid_line_pos = {
            start.line + i,
            0,
            start.text
        };

        print_line(&mid_line_pos, 0, -1, min_indent, end.line + 1);
    }

    print_line(&end, 0, end.col, min_indent, end.line);
}

void nst_print_error(Nst_Error err)
{
    print_position(err.start, err.end);

    if ( use_color )
        printf(C_YELLOW "%s" C_RESET " - %s\n", err.name->value, err.message->value);
    else
        printf("%s - %s\n", err.name->value, err.message->value);
}

void nst_print_traceback(Nst_Traceback tb)
{
    assert(tb.positions->size % 2 == 0);

    Nst_Pos prev_start = { -1, -1, NULL };
    Nst_Pos prev_end = { -1, -1, NULL };
    int repeat_count = 0;
    int i = 0;
    for ( LLNode *n1 = tb.positions->head, *n2 = n1->next;
          n1 != NULL;
          n1 = n2->next, n2 = n1 == NULL ? n1 : n1->next )
    {
        i++;
        Nst_Pos *start = (Nst_Pos *)n1->value;
        Nst_Pos *end   = (Nst_Pos *)n2->value;

        if ( start->text == NULL )
            continue;

        if ( start->col == prev_start.col && start->line == prev_start.line &&
             end->col   == prev_end.col   && end->line   == prev_end.line   &&
             start->text == prev_start.text )
        {
            repeat_count++;
            continue;
        }
        else
        {
            if ( repeat_count > 0 )
            {
                if ( use_color )
                    printf(C_RED "-- Previous position repeated %i more times --\n" C_RESET, repeat_count);
                else
                    printf("-- Previous position repeated %i more times --\n", repeat_count);
            }
            repeat_count = 0;
            prev_start = *start;
            prev_end = *end;
        }

        print_position(*start, *end);
    }
    if ( tb.error.start.col == prev_start.col && tb.error.start.line == prev_start.line &&
         tb.error.end.col   == prev_end.col   && tb.error.end.line   == prev_end.line   &&
         tb.error.start.text == prev_start.text )
    {
        repeat_count++;
    }

    if ( repeat_count > 0 )
    {
        if ( use_color )
            printf(C_RED "-- Previous position repeated %i more times --\n" C_RESET, repeat_count);
        else
            printf("-- Previous position repeated %i more times --\n", repeat_count);
    }
    else
        print_position(tb.error.start, tb.error.end);

    if ( use_color )
        printf(C_YELLOW "%s" C_RESET " - %s\n", tb.error.name->value, tb.error.message->value);
    else
        printf("%s - %s\n", tb.error.name->value, tb.error.message->value);
}

Nst_StrObj *_nst_format_error(const char *format, const char *format_args, ...)
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

    return STR(nst_new_cstring_raw((const char *)buffer, true));
}
