#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nest.h"

#ifdef Nst_MSVC
#include <windows.h>
#endif // !Nst_MSVC

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
static Nst_Obj *err_stream = NULL;
static Nst_Traceback tb;

void Nst_error_set_color(bool color)
{
    use_color = color;
}

static inline void set_error_stream(void)
{
    err_stream = Nst_inc_ref(Nst_io.err);
    if (Nst_IOF_IS_CLOSED(err_stream)) {
        Nst_dec_ref(err_stream);
        use_stderr = true;
        fprintf(stderr, "Cannot use @@io._get_stderr, using initial stderr\n");
        fflush(stderr);
    }
}

static inline void err_putc(const char *ch, usize len)
{
    Nst_fwrite((u8 *)ch, len, NULL, err_stream);
}

Nst_Pos Nst_pos_empty(void)
{
    Nst_Pos new_pos = { 0, 0, NULL };
    return new_pos;
}

Nst_Span Nst_span_new(Nst_Pos start, Nst_Pos end)
{
    Nst_Span span = {
        .start_line = start.line,
        .start_col = start.col,
        .end_line = end.line,
        .end_col = end.col,
        .text = start.text
    };
    return span;
}

Nst_Span Nst_span_from_pos(Nst_Pos pos)
{
    Nst_Span span = {
        .start_line = pos.line,
        .start_col = pos.col,
        .end_line = pos.line,
        .end_col = pos.col,
        .text = pos.text
    };
    return span;
}

Nst_Span Nst_span_empty(void)
{
    Nst_Span span = {
        .start_line = 0,
        .start_col = 0,
        .end_line = 0,
        .end_col = 0,
        .text = NULL
    };
    return span;
}

Nst_Span Nst_span_join(Nst_Span span1, Nst_Span span2)
{
    if (span1.text != span2.text)
        return span1;
    Nst_Span new_span = {
        .text = span1.text
    };
    if (span1.start_line < span2.start_line
        || (span1.start_line == span2.start_line
            && span1.start_col <= span2.start_col))
    {
        new_span.start_line = span1.start_line;
        new_span.start_col = span1.start_col;
    } else {
        new_span.start_line = span2.start_line;
        new_span.start_col = span2.start_col;
    }

    if (span1.end_line > span2.end_line
        || (span1.end_line == span2.end_line
        && span1.end_col >= span2.end_col))
    {
        new_span.end_line = span1.end_line;
        new_span.end_col = span1.end_col;
    } else {
        new_span.end_line = span2.end_line;
        new_span.end_col = span2.end_col;
    }
    return new_span;
}

Nst_Span Nst_span_expand(Nst_Span span, Nst_Pos pos)
{
    if (span.text != pos.text)
        return span;
    if (pos.line < span.start_line) {
        span.start_line = pos.line;
        span.start_col = pos.col;
    } else if (pos.line == span.start_line && pos.col < span.start_col)
        span.start_col = pos.col;
    else if (pos.line > span.end_line) {
        span.end_line = pos.line;
        span.end_col = pos.col;
    } else if (pos.line == span.end_line && pos.col > span.end_col)
        span.end_col = pos.col;
    return span;
}

Nst_Pos Nst_span_start(Nst_Span span)
{
    Nst_Pos start = {
        .line = span.start_line,
        .col = span.start_col,
        .text = span.text
    };
    return start;
}

Nst_Pos Nst_span_end(Nst_Span span)
{
    Nst_Pos end = {
        .line = span.end_line,
        .col = span.end_col,
        .text = span.text
    };
    return end;
}

static inline void print_repeat(char ch, i32 times)
{
    if (times < 0)
        times = 0;

    for (i32 i = 0; i < times; i++)
        err_putc(&ch, 1);
}

static i32 get_indent(Nst_SourceText *text, i32 lineno)
{
    i32 indent = 0;

    for (char *p = text->lines[lineno]; *p != '\n' && *p != '\0'; p++) {
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
    char *text = pos->text->lines[lineno];
    char *start = pos->text->text;
    usize tot_len = pos->text->text_len;
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
            C_CYN " %*" PRIi32 C_RES " | ",
            lineno_len,
            lineno + 1);
    } else
        Nst_fprintf(err_stream, " %*" PRIi32 " | ", lineno_len, lineno + 1);

    for (i32 i = keep_indent;
         i < (isize)tot_len && text[i] != '\n' && text[i] != '\0';
         i++)
    {
        if (i < start_col)
            spaces++;
        else if (i <= end_col)
            carets++;

        if (i == start_col && use_color)
            Nst_fprint(err_stream, C_RED);

        i32 res = Nst_check_ext_utf8_bytes(
            (u8 *)text + i,
            tot_len - (start - text)
        );

        if (res == -1)
            continue;

        err_putc((char *)text + i, res);
        i += res - 1;
        line_length++;

        if (use_color && i == end_col)
            Nst_fprint(err_stream, C_RES);
    }
    err_putc("\n", 1);

    if (start_col == line_length) carets++;

    if (end_col == -1)
        end_col = keep_indent + line_length - 1;
    else
        end_col -= keep_indent;

    start_col -= keep_indent;

    if (end_col - start_col + 1 >= line_length) {
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
    Nst_assert_c(start.text == end.text);

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
                "lines " C_CYN "%" PRIi32 C_RES " to " C_CYN "%" PRIi32 C_RES,
                start.line + 1,
                end.line + 1);
        } else {
            Nst_fprintf(
                err_stream,
                "lines %" PRIi32 " to %" PRIi32,
                start.line + 1,
                end.line + 1);
        }
    } else {
        if (use_color) {
            Nst_fprintf(
                err_stream,
                "line " C_CYN "%" PRIi32 C_RES,
                start.line + 1);
        } else
            Nst_fprintf(err_stream, "line %" PRIi32, start.line + 1);
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

static inline void print_rep_count(i32 count)
{
    if (use_color) {
        Nst_fprintf(
            err_stream,
            C_RED "-- Previous position repeated %" PRIi32 " more times --\n"
            C_RES,
            count);
    } else {
        Nst_fprintf(
            err_stream,
            "-- Previous position repeated %" PRIi32 " more times --\n",
            count);
    }
}

void Nst_error_print()
{
    Nst_fflush(Nst_io.out);
    set_error_stream();

    Nst_Pos prev_start = { -1, -1, NULL };
    Nst_Pos prev_end = { -1, -1, NULL };
    i32 repeat_count = 0;
    for (usize i = 0, n = tb.positions.len; i < n; i++) {
        Nst_Span *span = (Nst_Span *)Nst_da_get(&tb.positions, n - i - 1);
        Nst_Pos start = Nst_span_start(*span);
        Nst_Pos end = Nst_span_end(*span);
        if (start.col == prev_start.col
            && start.line == prev_start.line
            && end.col == prev_end.col
            && end.line == prev_end.line
            && start.text == prev_start.text)
        {
            repeat_count++;
            continue;
        }

        if (repeat_count > 0)
            print_rep_count(repeat_count);
        repeat_count = 0;
        prev_start = start;
        prev_end = end;

        print_position(start, end);
    }

    u8 *err_name = Nst_str_value(tb.error_name);
    if (tb.error_msg == Nst_null() && use_color)
        Nst_fprintf(err_stream, C_YEL "%s" C_RES "\n", err_name);
    else if (tb.error_msg == Nst_null())
        Nst_fprintf(err_stream, "%s\n", err_name);
    else if (use_color) {
        Nst_fprintf(
            err_stream,
            C_YEL "%s" C_RES " - %s\n",
            err_name, Nst_str_value(tb.error_msg));
    } else {
        Nst_fprintf(
            err_stream,
            "%s - %s\n",
            err_name, Nst_str_value(tb.error_msg));
    }

    Nst_fflush(err_stream);
    Nst_dec_ref(err_stream);
}

void Nst_error_set(Nst_ObjRef *name, Nst_ObjRef *msg)
{
    if (msg == NULL) {
        Nst_error_failed_alloc();
        return;
    }

    Nst_assert_c(name != NULL);

    Nst_error_clear();
    tb.error_name = name;
    tb.error_msg = msg;
    tb.error_occurred = true;
}

void Nst_error_set_syntax(Nst_ObjRef *msg)
{
    Nst_error_set(Nst_inc_ref(Nst_s.e_SyntaxError), msg);
}

void Nst_error_set_memory(Nst_ObjRef *msg)
{
    Nst_error_set(Nst_inc_ref(Nst_s.e_MemoryError), msg);
}

void Nst_error_set_type(Nst_ObjRef *msg)
{
    Nst_error_set(Nst_inc_ref(Nst_s.e_TypeError), msg);
}

void Nst_error_set_value(Nst_ObjRef *msg)
{
    Nst_error_set(Nst_inc_ref(Nst_s.e_ValueError), msg);
}

void Nst_error_set_math(Nst_ObjRef *msg)
{
    Nst_error_set(Nst_inc_ref(Nst_s.e_MathError), msg);
}

void Nst_error_set_call(Nst_ObjRef *msg)
{
    Nst_error_set(Nst_inc_ref(Nst_s.e_CallError), msg);
}

void Nst_error_set_import(Nst_ObjRef *msg)
{
    Nst_error_set(Nst_inc_ref(Nst_s.e_ImportError), msg);
}

static void set_error_c(Nst_Obj *name, const char *msg)
{
    Nst_Obj *msg_obj = Nst_str_new_c_raw(msg, false);
    if (msg_obj == NULL)
        return;

    Nst_error_set(Nst_inc_ref(name), msg_obj);
}

void Nst_error_setc_syntax(const char *msg)
{
    set_error_c(Nst_s.e_SyntaxError, msg);
}

void Nst_error_setc_memory(const char *msg)
{
    set_error_c(Nst_s.e_MemoryError, msg);
}

void Nst_error_setc_type(const char *msg)
{
    set_error_c(Nst_s.e_TypeError, msg);
}

void Nst_error_setc_value(const char *msg)
{
    set_error_c(Nst_s.e_ValueError, msg);
}

void Nst_error_setc_math(const char *msg)
{
    set_error_c(Nst_s.e_MathError, msg);
}

void Nst_error_setc_call(const char *msg)
{
    set_error_c(Nst_s.e_CallError, msg);
}

void Nst_error_setc_import(const char *msg)
{
    set_error_c(Nst_s.e_ImportError, msg);
}

void Nst_error_setf_syntax(Nst_WIN_FMT const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Nst_Obj *msg = Nst_vsprintf(fmt, args);
    if (msg != NULL)
        Nst_error_set_syntax(msg);
}

void Nst_error_setf_memory(Nst_WIN_FMT const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Nst_Obj *msg = Nst_vsprintf(fmt, args);
    if (msg != NULL)
        Nst_error_set_memory(msg);
}

void Nst_error_setf_type(Nst_WIN_FMT const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Nst_Obj *msg = Nst_vsprintf(fmt, args);
    if (msg != NULL)
        Nst_error_set_type(msg);
}

void Nst_error_setf_value(Nst_WIN_FMT const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Nst_Obj *msg = Nst_vsprintf(fmt, args);
    if (msg != NULL)
        Nst_error_set_value(msg);
}

void Nst_error_setf_math(Nst_WIN_FMT const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Nst_Obj *msg = Nst_vsprintf(fmt, args);
    if (msg != NULL)
        Nst_error_set_math(msg);
}

void Nst_error_setf_call(Nst_WIN_FMT const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Nst_Obj *msg = Nst_vsprintf(fmt, args);
    if (msg != NULL)
        Nst_error_set_call(msg);
}

void Nst_error_setf_import(Nst_WIN_FMT const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Nst_Obj *msg = Nst_vsprintf(fmt, args);
    if (msg != NULL)
        Nst_error_set_import(msg);
}

void Nst_error_failed_alloc(void)
{
    Nst_error_set(
        Nst_inc_ref(Nst_s.e_MemoryError),
        Nst_inc_ref(Nst_s.o_failed_alloc)
    );
}

bool Nst_error_occurred(void)
{
    return tb.error_occurred;
}

Nst_Traceback *Nst_error_get(void)
{
    return &tb;
}

void Nst_error_clear(void)
{
    Nst_da_clear(&tb.positions, NULL);
    if (tb.error_name != NULL)
        Nst_dec_ref(tb.error_name);
    if (tb.error_msg != NULL)
        Nst_dec_ref(tb.error_msg);
    tb.error_name = NULL;
    tb.error_msg = NULL;
}

void _Nst_error_init(void)
{
    tb.error_name = NULL;
    tb.error_msg = NULL;
    tb.error_occurred = false;
    Nst_da_init(&tb.positions, sizeof(Nst_Span), 0);
}

void Nst_error_add_span(Nst_Span span)
{
    if (span.text == NULL)
        return;

    Nst_da_append(&tb.positions, &span);
}
