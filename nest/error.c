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
        printf("%c", ch);
}

static int print_line(Nst_Pos *pos, long start_col, long end_col, int max_indent, int max_line)
{
    long line_length = 0;
    long indent = 0;
    long curr_line = 0;
    bool is_indentation = true;
    char *text = pos->text->text;
    size_t text_len = pos->text->len;
    long lineno = pos->line;
    bool is_printing_error = false;
    long li = 0;
    int line_len = 0;

    if ( max_line == 0 )
        max_line = lineno + 1;

    while ( max_line > 0 )
    {
        line_len++;
        max_line /= 10;
    }

    if ( use_color )
        printf(C_CYAN " %#*i" C_RESET " | ", line_len, lineno + 1);
    else
        printf(" %#*i | ", line_len, lineno + 1);

    for ( size_t i = 0; i < text_len; i++ )
    {
        if ( curr_line == lineno )
        {
            if ( (text[i] != ' ' && text[i] != '\t') || indent == max_indent )
                is_indentation = false;

            if ( li == start_col && use_color )
            {
                is_printing_error = true;
                printf(C_RED);
            }

            if ( !is_indentation )
            {
                printf("%c", text[i]);
                line_length++;
            }
            else
                ++indent;

            if ( use_color && li == end_col )
            {
                is_printing_error = false;
                printf(C_RESET);
            }
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
        end_col = indent + line_length - 1;

    if ( start_col < indent )
        start_col = indent;

    if ( end_col - start_col + 1 == line_length )
    {
        if ( use_color )
            printf(C_RESET);
        return indent;
    }

    if ( use_color )
    {
        print_repeat(' ', line_len);
        printf(C_RESET "  | " C_RED);
        print_repeat(' ', start_col - indent);
        print_repeat('^', end_col - start_col + 1);
        printf(C_RESET "\n");
    }
    else
    {
        print_repeat(' ', line_len);
        printf("  | ");
        print_repeat(' ', start_col - indent);
        print_repeat('^', end_col - start_col + 1);
        printf("\n");
    }

    return indent;
}

static void print_position(Nst_Pos start, Nst_Pos end)
{
    assert(start.text == end.text);

    if ( start.text == NULL )
        return;

    if ( use_color )
        printf("File " C_GREEN "\"%s\"" C_RESET " at line " C_CYAN "%li" C_RESET,
            start.text->path,
            start.line + 1);
    else
        printf("File \"%s\" at line %li",
            start.text->path,
            start.line + 1);

    if ( start.line != end.line)
    {
        if ( use_color )
            printf(" to " C_CYAN "% li" C_RESET, end.line + 1 );
        else
            printf(" to %li", end.line + 1 );
    }
    printf(":\n");

    if ( start.line == end.line )
    {
        print_line(&start, start.col, end.col, -1, 0);
        return;
    }

    int max_indent = print_line(&start, start.col, -1, -1, end.line + 1);

    for (long i = 1, n = end.line - start.line; i < n; i++)
    {
        Nst_Pos mid_line_pos = {
            start.line + i,
            0,
            start.text
        };

        print_line(&mid_line_pos, 0, -1, max_indent, end.line + 1);
    }

    print_line(&end, 0, end.col, max_indent, end.line);
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
    for ( LLNode *n1 = tb.positions->head, *n2 = n1->next;
          n1 != NULL;
          n1 = n2->next, n2 = n1 == NULL ? n1 : n1->next )
    {
        Nst_Pos *start = (Nst_Pos *)n1->value;
        Nst_Pos *end   = (Nst_Pos *)n2->value;

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
