#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "error.h"
#include "simple_types.h"
#include "global_consts.h"
#include "encoding.h"

#define INT_CH_COUNT 21

#define PRINT(str, len) \
    err_stream->write_f(\
        (void *)str, \
        sizeof(i8), \
        (usize)len, \
        err_stream->value)

#define C_RED "\x1b[31m"
#define C_GRN "\x1b[32m"
#define C_YEL "\x1b[33m"
#define C_CYN "\x1b[96m"
#define C_RES "\x1b[0m"

#define R_LEN 4
#define C_LEN 5

static bool use_color = true;
static Nst_IOFileObj *err_stream = NULL;
static i8 *printf_buf = NULL;
static usize buf_size = 0;

void nst_set_color(bool color)
{
    use_color = color;
}

static inline void set_error_stream()
{
    err_stream = IOFILE(nst_inc_ref(nst_io->err));
    if ( NST_IOF_IS_CLOSED(err_stream) )
    {
        nst_dec_ref(err_stream);
        err_stream = IOFILE(nst_iof_new(stderr, false, false, true));
        fprintf(stderr, "Cannot use @@io._get_stderr, using initial stderr\n");
        fflush(stderr);
    }
}

static inline void err_printf(const i8 *format, i32 size, ...)
{
    va_list args;
    va_start(args, size);

    if ( buf_size == 0 )
    {
        printf_buf = (i8 *)calloc(size + 1, sizeof(i8));
        if ( printf_buf == NULL )
        {
            return;
        }
        buf_size = (usize)size + 1;
    }
    else if ( buf_size < (usize)size + 1 )
    {
        i8 *new_buf = (i8 *)realloc(printf_buf, size + 1);
        if ( new_buf == NULL )
        {
            return;
        }
        printf_buf = new_buf;
        buf_size = (usize)size + 1;
    }

    vsprintf(printf_buf, format, args);
    err_stream->write_f(printf_buf, sizeof(i8),
                        strlen(printf_buf), err_stream->value);
}

static inline void err_putc(i8 ch)
{
    err_stream->write_f(&ch, sizeof(i8), 1, err_stream->value);
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

static inline void print_repeat(i8 ch, i32 times)
{
    if ( times < 0 )
    {
        times = 0;
    }
    for ( i32 i = 0; i < times; i++ )
    {
        err_putc(ch);
    }
}

static i32 get_indent(Nst_SourceText *text, i32 lineno)
{
    i32 indent = 0;

    for ( i8 *p = text->lines[lineno]; *p != '\n' && *p != '\0'; p++)
    {
        if ( *p == ' ' || *p == '\t' )
        {
            indent++;
        }
        else
        {
            return indent;
        }
    }
    return indent;
}

static void print_line(Nst_Pos *pos,
                       i32     start_col,
                       i32     end_col,
                       i32      keep_indent,
                       i32      max_line)
{
    i32 line_length = 0;
    i32 lineno_len = 0;
    i32 lineno = pos->line;
    i8 *text = pos->text->lines[lineno];
    i8 *start = pos->text->text;
    usize tot_len = pos->text->len;
    i32 indent = get_indent(pos->text, lineno) - keep_indent;
    i32 spaces = 0;
    i32 carets = 0;

    if ( keep_indent == -1 )
    {
        indent--;
        keep_indent = indent;
    }

    if ( start_col == 0 )
    {
        start_col = keep_indent;
    }

    if ( max_line == 0 )
    {
        max_line = lineno + 1;
    }

    while ( max_line > 0 )
    {
        lineno_len++;
        max_line /= 10;
    }

    if ( use_color )
    {
        err_printf(
            C_CYN " %*li" C_RES " | ",
            C_LEN + R_LEN + lineno_len + 4,
            lineno_len, lineno + 1);
    }
    else
    {
        err_printf(" %*li | ", lineno_len + 4, lineno_len, lineno + 1);
    }

    for ( i32 i = keep_indent; text[i] != '\n' && text[i] != '\0'; i++ )
    {
        if ( i < start_col )
        {
            spaces++;
        }
        else if ( i <= end_col )
        {
            carets++;
        }

        if ( i == start_col && use_color )
        {
            PRINT(C_RED, C_LEN);
        }

        err_putc(text[i]);
        line_length++;

        if ( use_color && i == end_col )
        {
            PRINT(C_RES, R_LEN);
        }

        i32 res = nst_check_utf8_bytes(
            (u8 *)text + i,
            tot_len - (start - text));
        if ( res == 1 || res == -1 )
        {
            continue;
        }
        if ( i < start_col )
        {
            spaces -= res - 1;
        }
        else if ( i <= end_col )
        {
            carets -= res - 1;
        }
    }
    err_putc('\n');

    if ( start_col == line_length )
    {
        carets++;
    }

    if ( end_col == -1 )
    {
        end_col = keep_indent + line_length - 1;
    }
    else
    {
        end_col -= keep_indent;
    }

    start_col -= keep_indent;

    if ( end_col - start_col + 1 == line_length )
    {
        if ( use_color )
        {
            PRINT(C_RES, R_LEN);
        }
        return;
    }

    if ( use_color )
    {
        print_repeat(' ', lineno_len);
        PRINT(C_RES "  | " C_RED, R_LEN + 4 + C_LEN);
        print_repeat(' ', spaces);
        print_repeat('^', carets);
        PRINT(C_RES "\n", R_LEN + 1);
    }
    else
    {
        print_repeat(' ', lineno_len);
        PRINT("  | ", 4);
        print_repeat(' ', spaces);
        print_repeat('^', carets);
        PRINT("\n", 1);
    }
}

static void print_position(Nst_Pos start, Nst_Pos end)
{
    assert(start.text == end.text);

    if ( start.text == NULL )
    {
        return;
    }

    if ( use_color )
    {
        err_printf(
            "File " C_GRN "\"%s\"" C_RES " at ",
            C_LEN + R_LEN + 11 + (i32)strlen(start.text->path),
            start.text->path);
    }
    else
    {
        err_printf(
            "File \"%s\" at ",
            11 + (i32)strlen(start.text->path),
            start.text->path);
    }

    if ( start.line != end.line )
    {
        if ( use_color )
        {
            err_printf(
                "lines " C_CYN "%li" C_RES " to " C_CYN "%li" C_RES,
                C_LEN * 2 + R_LEN * 2 + INT_CH_COUNT * 2 + 10,
                start.line + 1,
                end.line + 1);
        }
        else
        {
            err_printf(
                "lines %li to %li",
                INT_CH_COUNT * 2 + 10,
                start.line + 1,
                end.line + 1);
        }
    }
    else
    {
        if ( use_color )
        {
            err_printf(
                "line " C_CYN "%li" C_RES,
                C_LEN + R_LEN + INT_CH_COUNT + 5,
                start.line + 1);
        }
        else
        {
            err_printf("line %li", INT_CH_COUNT + 5, start.line + 1);
        }
    }
    PRINT(":\n", 2);

    if ( start.line == end.line )
    {
        print_line(&start, start.col, end.col, -1, 0);
        return;
    }

    i32 min_indent = get_indent(start.text, start.line);
    for ( i32 i = start.line + 1; i <= end.line; i++ )
    {
        i32 indent = get_indent(start.text, i);
        if ( indent < min_indent )
        {
            min_indent = indent;
        }
    }

    print_line(&start, start.col, -1, min_indent, end.line + 1);

    for ( i32 i = 1, n = end.line - start.line; i < n; i++ )
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
    nst_flush();
    if ( !NST_IOF_IS_CLOSED(nst_io->out) )
    {
        nst_io->out->flush_f(nst_io->out->value);
    }

    set_error_stream();
    print_position(err.start, err.end);

    if ( use_color )
    {
        err_printf(
            C_YEL "%s" C_RES " - %s\n",
            (i32)err.name->len + (i32)err.message->len + C_LEN + R_LEN + 4,
            err.name->value, err.message->value);
    }
    else
    {
        err_printf(
            "%s - %s\n",
            (i32)err.name->len + (i32)err.message->len + 4,
            err.name->value, err.message->value);
    }
    err_stream->flush_f(err_stream->value);

    if ( buf_size != 0 )
    {
        free(printf_buf);
    }
    nst_dec_ref(err_stream);
}

static inline void print_rep_count(i32 count)
{
    if ( use_color )
    {
        err_printf(
            C_RED "-- Previous position repeated %i more times --\n" C_RES,
            C_LEN + R_LEN + INT_CH_COUNT + 45, count);
    }
    else
    {
        err_printf(
            "-- Previous position repeated %i more times --\n",
            INT_CH_COUNT + 45, count);
    }
}

void nst_print_traceback(Nst_Traceback tb)
{
    nst_flush();
    assert(tb.positions->size % 2 == 0);
    set_error_stream();

    Nst_Pos prev_start = { -1, -1, NULL };
    Nst_Pos prev_end = { -1, -1, NULL };
    i32 repeat_count = 0;
    i32 i = 0;
    for ( Nst_LLNode *n1 = tb.positions->head, *n2 = n1->next;
          n1 != NULL;
          n1 = n2->next, n2 = n1 == NULL ? n1 : n1->next )
    {
        i++;
        Nst_Pos *start = (Nst_Pos *)n1->value;
        Nst_Pos *end   = (Nst_Pos *)n2->value;

        if ( start->text == NULL )
        {
            continue;
        }

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
                print_rep_count(repeat_count);
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
        print_rep_count(repeat_count);
    }
    else if ( repeat_count > 0 )
    {
        print_rep_count(repeat_count);
        print_position(tb.error.start, tb.error.end);
    }
    else
    {
        print_position(tb.error.start, tb.error.end);
    }

    if ( use_color )
    {
        err_printf(
            C_YEL "%s" C_RES " - %s\n",
            (i32)tb.error.name->len + (i32)tb.error.message->len + C_LEN + R_LEN + 4,
            tb.error.name->value, tb.error.message->value);
    }
    else
    {
        err_printf(
            "%s - %s\n",
            (i32)tb.error.name->len + (i32)tb.error.message->len + 4,
            tb.error.name->value, tb.error.message->value);
    }

    err_stream->flush_f(err_stream->value);
    if ( buf_size != 0 )
    {
        free(printf_buf);
    }
    nst_dec_ref(err_stream);
}

Nst_StrObj *nst_format_error(const i8 *format, const i8 *format_args, ...)
{
    /*
    Format args types:
        s: i8 *
        u: usize
        i: i64
    */

    va_list args;
    va_start(args, format_args);

    usize tot_size = strlen(format);
    i8 *arg_ptr = (i8 *)format_args;
    while ( *arg_ptr )
    {
        switch ( *arg_ptr )
        {
        case 's':
            tot_size += strlen(va_arg(args, i8 *));
            break;
        case 'u':
            va_arg(args, usize);
            tot_size += INT_CH_COUNT;
            break;
        case 'i':
            va_arg(args, Nst_Int);
            tot_size += INT_CH_COUNT;
            break;
        default:
            break;
        }
        ++arg_ptr;
    }

    i8 *buffer = (i8 *)calloc(tot_size + 1, sizeof(i8));
    if (buffer == NULL)
    {
        return NULL;
    }

    va_start(args, format_args);
    vsprintf(buffer, format, args);
    va_end(args);
    Nst_Obj *str = nst_string_new_c_raw((const i8 *)buffer, true);
    str->ref_count = 0;
    return STR(str);
}

void nst_free_src_text(Nst_SourceText *text)
{
    if ( text == NULL )
    {
        return;
    }

    free(text->text);
    free(text->lines);
    free(text->path);
    free(text);
}
