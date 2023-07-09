#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "simple_types.h"
#include "error.h"
#include "global_consts.h"
#include "encoding.h"
#include "format.h"
#include "interpreter.h"

#ifdef Nst_WIN
#include <windows.h>
#endif // !Nst_WIN

#include <errno.h>
#include "mem.h"
#define INT_CH_COUNT 21

#define C_RED "\x1b[31m"
#define C_GRN "\x1b[32m"
#define C_YEL "\x1b[33m"
#define C_CYN "\x1b[96m"
#define C_RES "\x1b[0m"

static bool use_color = true;
static bool use_stderr = false;
static Nst_IOFileObj *err_stream = NULL;
static i8 *printf_buf = NULL;
static usize buf_size = 0;
static Nst_OpErr global_op_err = { NULL, NULL };

void Nst_set_color(bool color)
{
    use_color = color;
}

static inline void set_error_stream()
{
    err_stream = IOFILE(Nst_inc_ref(Nst_io.err));
    if (Nst_IOF_IS_CLOSED(err_stream)) {
        Nst_dec_ref(err_stream);
        use_stderr = true;
        fprintf(stderr, "Cannot use @@io._get_stderr, using initial stderr\n");
        fflush(stderr);
    }
}

static inline void err_putc(i8 ch)
{
    Nst_fwrite(&ch, sizeof(i8), 1, err_stream);
}

Nst_Pos Nst_copy_pos(Nst_Pos pos)
{
    Nst_Pos new_pos = {
        pos.line,
        pos.col,
        pos.text
    };

    return new_pos;
}

Nst_Pos Nst_no_pos()
{
    Nst_Pos new_pos = { 0, 0, NULL };
    return new_pos;
}

#ifdef Nst_WIN
#pragma warning( disable: 4100 )
#endif // !Nst_WIN

static inline void print_repeat(i8 ch, i32 times)
{
    if (times < 0)
        times = 0;

    for (i32 i = 0; i < times; i++)
        err_putc(ch);
}

static i32 get_indent(Nst_SourceText *text, i32 lineno)
{
    i32 indent = 0;

    for (i8 *p = text->lines[lineno]; *p != '\n' && *p != '\0'; p++) {
        if (*p == ' ' || *p == '\t')
            indent++;
        else
            return indent;
    }
    return indent;
}

static void print_line(Nst_Pos *pos, i32 start_col, i32 end_col,
                       i32 keep_indent, i32 max_line)
{
    i32 line_length = 0;
    int lineno_len = 0;
    i32 lineno = pos->line;
    i8 *text = pos->text->lines[lineno];
    i8 *start = pos->text->text;
    usize tot_len = pos->text->len;
    i32 indent = get_indent(pos->text, lineno) - keep_indent;
    i32 spaces = 0;
    i32 carets = 0;

    if (keep_indent == -1) {
        indent--;
        keep_indent = indent;
    }

    if (start_col == 0)
        start_col = keep_indent;

    if (max_line == 0)
        max_line = lineno + 1;

    while (max_line > 0) {
        lineno_len++;
        max_line /= 10;
    }

    if (use_color) {
        Nst_fprintf(
            err_stream,
            C_CYN " %*li" C_RES " | ",
            lineno_len,
            lineno + 1);
    } else
        Nst_fprintf(err_stream, " %*li | ", lineno_len, lineno + 1);

    for (i32 i = keep_indent; text[i] != '\n' && text[i] != '\0'; i++) {
        if (i < start_col)
            spaces++;
        else if (i <= end_col)
            carets++;

        if (i == start_col && use_color)
            Nst_fprint(err_stream, C_RED);

        err_putc(text[i]);
        line_length++;

        if (use_color && i == end_col)
            Nst_fprint(err_stream, C_RES);

        i32 res = Nst_check_utf8_bytes(
            (u8 *)text + i,
            tot_len - (start - text)
        );
        if (res == 1 || res == -1)
            continue;

        if ( i < start_col )
            spaces -= res - 1;
        else if ( i <= end_col )
            carets -= res - 1;
    }
    err_putc('\n');

    if (start_col == line_length) carets++;

    if (end_col == -1)
        end_col = keep_indent + line_length - 1;
    else
        end_col -= keep_indent;

    start_col -= keep_indent;

    if (end_col - start_col + 1 == line_length) {
        if (use_color)
            Nst_fprint(err_stream, C_RES);
        return;
    }

    if (use_color) {
        print_repeat(' ', lineno_len);
        Nst_fprint(err_stream, C_RES "  | " C_RED);
        print_repeat(' ', spaces);
        print_repeat('^', carets);
        Nst_fprint(err_stream, C_RES "\n");
    } else {
        print_repeat(' ', lineno_len);
        Nst_fprint(err_stream, "  | ");
        print_repeat(' ', spaces);
        print_repeat('^', carets);
        Nst_fprint(err_stream, "\n");
    }
}

static void print_position(Nst_Pos start, Nst_Pos end)
{
    assert(start.text == end.text);

    if (start.text == NULL)
        return;

    if (use_color) {
        Nst_fprintf(
            err_stream,
            "File " C_GRN "\"%s\"" C_RES " at ",
            start.text->path);
    } else
        Nst_fprintf(err_stream, "File \"%s\" at ", start.text->path);

    if (start.line != end.line) {
        if (use_color) {
            Nst_fprintf(err_stream,
                "lines " C_CYN "%li" C_RES " to " C_CYN "%li" C_RES,
                start.line + 1,
                end.line + 1);
        } else {
            Nst_fprintf(
                err_stream,
                "lines %li to %li",
                start.line + 1,
                end.line + 1);
        }
    } else {
        if (use_color)
            Nst_fprintf(err_stream, "line " C_CYN "%li" C_RES, start.line + 1);
        else
            Nst_fprintf(err_stream, "line %li", start.line + 1);
    }
    Nst_fprint(err_stream, ":\n");

    if (start.line == end.line) {
        print_line(&start, start.col, end.col, -1, 0);
        return;
    }

    i32 min_indent = get_indent(start.text, start.line);
    for (i32 i = start.line + 1; i <= end.line; i++) {
        i32 indent = get_indent(start.text, i);
        if (indent < min_indent)
            min_indent = indent;
    }

    print_line(&start, start.col, -1, min_indent, end.line + 1);

    for (i32 i = 1, n = end.line - start.line; i < n; i++) {
        Nst_Pos mid_line_pos = {
            start.line + i,
            0,
            start.text
        };

        print_line(&mid_line_pos, 0, -1, min_indent, end.line + 1);
    }

    print_line(&end, 0, end.col, min_indent, end.line);
}

void Nst_print_error(Nst_Error err)
{
    Nst_fflush(Nst_io.out);
    set_error_stream();
    print_position(err.start, err.end);

    if (use_color)
    {
        Nst_fprintf(
            err_stream,
            C_YEL "%s" C_RES " - %s\n",
            err.name->value, err.message->value);
    } else {
        Nst_fprintf(
            err_stream,
            "%s - %s\n",
            err.name->value, err.message->value);
    }
    Nst_fflush(err_stream);

    if ( buf_size != 0 )
        Nst_free(printf_buf);
    Nst_dec_ref(err_stream);
}

static inline void print_rep_count(i32 count)
{
    if ( use_color ) {
        Nst_fprintf(
            err_stream,
            C_RED "-- Previous position repeated %li more times --\n" C_RES,
            count);
    } else {
        Nst_fprintf(
            err_stream,
            "-- Previous position repeated %li more times --\n",
            count);
    }
}

void Nst_print_traceback(Nst_Traceback tb)
{
    Nst_fflush(Nst_io.out);
    assert(tb.positions->size % 2 == 0);
    set_error_stream();

    Nst_Pos prev_start = { -1, -1, NULL };
    Nst_Pos prev_end = { -1, -1, NULL };
    i32 repeat_count = 0;
    i32 i = 0;
    for (Nst_LLNode *n1 = tb.positions->head, *n2 = n1->next;
         n1 != NULL;
         n1 = n2->next, n2 = n1 == NULL ? n1 : n1->next)
    {
        i++;
        Nst_Pos *start = (Nst_Pos *)n1->value;
        Nst_Pos *end   = (Nst_Pos *)n2->value;

        if (start->text == NULL)
            continue;

        if (start->col     == prev_start.col
            && start->line == prev_start.line
            && end->col    == prev_end.col
            && end->line   == prev_end.line
            && start->text == prev_start.text)
        {
            repeat_count++;
            continue;
        } else {
            if (repeat_count > 0)
                print_rep_count(repeat_count);

            repeat_count = 0;
            prev_start = *start;
            prev_end = *end;
        }

        print_position(*start, *end);
    }
    if (tb.error.start.col     == prev_start.col
        && tb.error.start.line == prev_start.line
        && tb.error.end.col    == prev_end.col
        && tb.error.end.line   == prev_end.line
        && tb.error.start.text == prev_start.text)
    {
        repeat_count++;
        print_rep_count(repeat_count);
    } else if (repeat_count > 0) {
        print_rep_count(repeat_count);
        print_position(tb.error.start, tb.error.end);
    } else {
        print_position(tb.error.start, tb.error.end);
    }

    if (use_color) {
        Nst_fprintf(
            err_stream,
            C_YEL "%s" C_RES " - %s\n",
            tb.error.name->value, tb.error.message->value);
    } else {
        Nst_fprintf(
            err_stream,
            "%s - %s\n",
            tb.error.name->value, tb.error.message->value);
    }

    Nst_fflush(err_stream);
    if (buf_size != 0)
        Nst_free(printf_buf);

    Nst_dec_ref(err_stream);
}

void Nst_free_src_text(Nst_SourceText *text)
{
    if (text == NULL)
        return;

    Nst_free(text->text);
    Nst_free(text->lines);
    Nst_free(text->path);
    Nst_free(text);
}

void _Nst_set_error(Nst_StrObj *name, Nst_StrObj *msg)
{
    if (msg == NULL) {
        Nst_failed_allocation();
        return;
    }

    assert(name != NULL);

    Nst_error_clear();
    global_op_err.name = name;
    global_op_err.message = msg;
}

void _Nst_set_syntax_error(Nst_StrObj *msg)
{
    Nst_set_error(Nst_inc_ref(Nst_s.e_SyntaxError), msg);
}

void _Nst_set_memory_error(Nst_StrObj *msg)
{
    Nst_set_error(Nst_inc_ref(Nst_s.e_MemoryError), msg);
}

void _Nst_set_type_error(Nst_StrObj *msg)
{
    Nst_set_error(Nst_inc_ref(Nst_s.e_TypeError), msg);
}

void _Nst_set_value_error(Nst_StrObj *msg)
{
    Nst_set_error(Nst_inc_ref(Nst_s.e_ValueError), msg);
}

void _Nst_set_math_error(Nst_StrObj *msg)
{
    Nst_set_error(Nst_inc_ref(Nst_s.e_MathError), msg);
}

void _Nst_set_call_error(Nst_StrObj *msg)
{
    Nst_set_error(Nst_inc_ref(Nst_s.e_CallError), msg);
}

void _Nst_set_import_error(Nst_StrObj *msg)
{
    Nst_set_error(Nst_inc_ref(Nst_s.e_ImportError), msg);
}

static void set_error_c(Nst_StrObj *name, const i8 *msg)
{
    Nst_StrObj *msg_obj = STR(Nst_string_new_c_raw(msg, false));
    if (msg_obj == NULL)
        return;

    Nst_set_error(Nst_inc_ref(name), msg_obj);
}

void Nst_set_syntax_error_c(const i8 *msg)
{
    set_error_c(Nst_s.e_SyntaxError, msg);
}

void Nst_set_memory_error_c(const i8 *msg)
{
    set_error_c(Nst_s.e_MemoryError, msg);
}

void Nst_set_type_error_c(const i8 *msg)
{
    set_error_c(Nst_s.e_TypeError, msg);
}

void Nst_set_value_error_c(const i8 *msg)
{
    set_error_c(Nst_s.e_ValueError, msg);
}

void Nst_set_math_error_c(const i8 *msg)
{
    set_error_c(Nst_s.e_MathError, msg);
}

void Nst_set_call_error_c(const i8 *msg)
{
    set_error_c(Nst_s.e_CallError, msg);
}

void Nst_set_import_error_c(const i8 *msg)
{
    set_error_c(Nst_s.e_ImportError, msg);
}

void Nst_failed_allocation()
{
    Nst_set_error(
        Nst_inc_ref(Nst_s.e_MemoryError),
        Nst_inc_ref(Nst_s.o_failed_alloc)
    );
}

bool Nst_error_occurred()
{
    return global_op_err.name != NULL;
}

Nst_OpErr *Nst_error_get()
{
    return &global_op_err;
}

void Nst_error_clear()
{
    if (Nst_error_occurred()) {
        Nst_dec_ref(global_op_err.name);
        Nst_dec_ref(global_op_err.message);
        global_op_err.name = NULL;
        global_op_err.message = NULL;
    }
}

bool Nst_traceback_init()
{
    Nst_state.traceback.error.start = Nst_no_pos();
    Nst_state.traceback.error.end = Nst_no_pos();
    Nst_state.traceback.error.name = NULL;
    Nst_state.traceback.error.message = NULL;
    Nst_state.traceback.error.occurred = false;
    Nst_state.traceback.positions = Nst_llist_new();

    return Nst_state.traceback.positions != NULL;
}

void Nst_traceback_delete()
{
    Nst_llist_destroy(Nst_state.traceback.positions, Nst_free);
    if (Nst_state.traceback.error.name != NULL)
        Nst_dec_ref(Nst_state.traceback.error.name);
    if (Nst_state.traceback.error.message != NULL)
        Nst_dec_ref(Nst_state.traceback.error.message);
}
