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

void Nst_set_color(bool color)
{
    use_color = color;
}

static inline void set_error_stream(void)
{
    err_stream = IOFILE(Nst_inc_ref(Nst_io.err));
    if (Nst_IOF_IS_CLOSED(err_stream)) {
        Nst_dec_ref(err_stream);
        use_stderr = true;
        fprintf(stderr, "Cannot use @@io._get_stderr, using initial stderr\n");
        fflush(stderr);
    }
}

static inline void err_putc(i8 *ch, usize len)
{
    Nst_fwrite(ch, len, NULL, err_stream);
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

Nst_Pos Nst_no_pos(void)
{
    Nst_Pos new_pos = { 0, 0, NULL };
    return new_pos;
}

static inline void print_repeat(i8 ch, i32 times)
{
    if (times < 0)
        times = 0;

    for (i32 i = 0; i < times; i++)
        err_putc(&ch, 1);
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
            C_CYN " %*li" C_RES " | ",
            lineno_len,
            lineno + 1);
    } else
        Nst_fprintf(err_stream, " %*li | ", lineno_len, lineno + 1);

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

        err_putc(text + i, res);
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

static inline void print_rep_count(i32 count)
{
    if (use_color) {
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

void Nst_print_traceback(Nst_Traceback *tb)
{
    Nst_fflush(Nst_io.out);
    Nst_assert_c(tb->positions->len % 2 == 0);
    set_error_stream();

    Nst_Pos prev_start = { -1, -1, NULL };
    Nst_Pos prev_end = { -1, -1, NULL };
    i32 repeat_count = 0;
    i32 i = 0;
    for (Nst_LLNode *n1 = tb->positions->head, *n2 = n1->next;
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

    if (use_color) {
        Nst_fprintf(
            err_stream,
            C_YEL "%s" C_RES " - %s\n",
            tb->error_name->value, tb->error_msg->value);
    } else {
        Nst_fprintf(
            err_stream,
            "%s - %s\n",
            tb->error_name->value, tb->error_msg->value);
    }

    Nst_fflush(err_stream);
    if (buf_size != 0)
        Nst_free(printf_buf);

    Nst_dec_ref(err_stream);
}

void Nst_source_text_destroy(Nst_SourceText *text)
{
    if (text == NULL)
        return;

    Nst_free(text->text);
    Nst_free(text->lines);
    Nst_free(text->path);
}

static void clear_error(Nst_Traceback *tb)
{
    if (!tb->error_occurred)
        return;
    Nst_llist_empty(tb->positions, Nst_free);
    Nst_dec_ref(tb->error_name);
    Nst_dec_ref(tb->error_msg);
    tb->error_name = NULL;
    tb->error_msg  = NULL;
    tb->error_occurred = false;
}

void _Nst_set_error(Nst_StrObj *name, Nst_StrObj *msg)
{
    if (msg == NULL) {
        Nst_failed_allocation();
        return;
    }

    Nst_assert_c(name != NULL);

    Nst_error_clear();
    if (Nst_state.es != NULL) {
        Nst_state.es->traceback.error_name = name;
        Nst_state.es->traceback.error_msg = msg;
        Nst_state.es->traceback.error_occurred = true;
    } else {
        Nst_state.global_traceback.error_name = name;
        Nst_state.global_traceback.error_msg = msg;
        Nst_state.global_traceback.error_occurred = true;
    }
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

void Nst_failed_allocation(void)
{
    Nst_set_error(
        Nst_inc_ref(Nst_s.e_MemoryError),
        Nst_inc_ref(Nst_s.o_failed_alloc)
    );
}

Nst_ErrorKind Nst_error_occurred(void)
{
    if (Nst_state.es == NULL) {
        if (Nst_state.global_traceback.error_occurred)
            return Nst_EK_GLOBAL;
        else
            return Nst_EK_NONE;
    } else {
        if (Nst_state.es->traceback.error_occurred)
            return Nst_EK_LOCAL;
        else
            return Nst_EK_NONE;
    }
}

Nst_Traceback *Nst_error_get(void)
{
    if (Nst_state.es == NULL)
        return &Nst_state.global_traceback;
    return &Nst_state.es->traceback;
}

void Nst_error_clear(void)
{
    if (Nst_state.es != NULL)
        clear_error(&Nst_state.es->traceback);
    clear_error(&Nst_state.global_traceback);
}

bool Nst_traceback_init(Nst_Traceback *tb)
{
    tb->error_name = NULL;
    tb->error_msg = NULL;
    tb->error_occurred = false;
    tb->positions = Nst_llist_new();

    return tb->positions != NULL;
}

void Nst_traceback_destroy(Nst_Traceback *tb)
{
    Nst_llist_destroy(tb->positions, Nst_free);
    if (tb->error_name != NULL)
        Nst_dec_ref(tb->error_name);
    if (tb->error_msg != NULL)
        Nst_dec_ref(tb->error_msg);
    tb->positions = NULL;
}

void Nst_source_text_init(Nst_SourceText *src)
{
    src->text = NULL;
    src->path = NULL;
    src->lines = NULL;
    src->text_len = 0;
    src->lines_len = 0;
}

void Nst_error_add_positions(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end)
{
    // when the text is null Nst_no_pos was used and no position should be
    // added
    if (start.text == NULL)
        return;

    Nst_Pos *positions = Nst_raw_malloc(2 * sizeof(Nst_Pos));
    if (positions == NULL)
        return;

    positions[0] = start;
    positions[1] = end;
    bool result = Nst_llist_push(
        tb->positions,
        positions + 1,
        false);
    if (!result) {
        Nst_free(positions);
        return;
    }
    result = Nst_llist_push(
        tb->positions,
        positions,
        true);
    if (!result) {
        Nst_llist_pop(tb->positions);
        Nst_free(positions);
    }
}

void Nst_set_internal_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                            Nst_StrObj *name, Nst_StrObj *msg)
{
    clear_error(tb);

    Nst_error_add_positions(tb, start, end);

    tb->error_occurred = true;
    tb->error_name = STR(Nst_inc_ref(name));
    tb->error_msg = msg;
}

void Nst_set_internal_error_c(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                              Nst_StrObj *name, const i8 *msg)
{
    Nst_StrObj *msg_obj = STR(Nst_string_new_c_raw(msg, false));
    if (msg_obj == NULL)
        return;

    Nst_set_internal_error(tb, start, end, name, msg_obj);
}

void Nst_set_internal_syntax_error(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, Nst_StrObj *msg)
{
    Nst_set_internal_error(tb, start, end, Nst_s.e_SyntaxError, msg);
}

void Nst_set_internal_memory_error(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, Nst_StrObj *msg)
{
    Nst_set_internal_error(tb, start, end, Nst_s.e_MemoryError, msg);
}

void Nst_set_internal_type_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                                 Nst_StrObj *msg)
{
    Nst_set_internal_error(tb, start, end, Nst_s.e_TypeError, msg);
}

void Nst_set_internal_value_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                                  Nst_StrObj *msg)
{
    Nst_set_internal_error(tb, start, end, Nst_s.e_ValueError, msg);
}

void Nst_set_internal_math_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                                 Nst_StrObj *msg)
{
    Nst_set_internal_error(tb, start, end, Nst_s.e_MathError, msg);
}

void Nst_set_internal_call_error(Nst_Traceback *tb, Nst_Pos start, Nst_Pos end,
                                 Nst_StrObj *msg)
{
    Nst_set_internal_error(tb, start, end, Nst_s.e_CallError, msg);
}

void Nst_set_internal_import_error(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, Nst_StrObj *msg)
{
    Nst_set_internal_error(tb, start, end, Nst_s.e_ImportError, msg);
}

void Nst_set_internal_syntax_error_c(Nst_Traceback *tb, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
{
    Nst_set_internal_error_c(tb, start, end, Nst_s.e_SyntaxError, msg);
}

void Nst_set_internal_memory_error_c(Nst_Traceback *tb, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
{
    Nst_set_internal_error_c(tb, start, end, Nst_s.e_MemoryError, msg);
}

void Nst_set_internal_type_error_c(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
{
    Nst_set_internal_error_c(tb, start, end, Nst_s.e_TypeError, msg);
}

void Nst_set_internal_value_error_c(Nst_Traceback *tb, Nst_Pos start,
                                    Nst_Pos end, const i8 *msg)
{
    Nst_set_internal_error_c(tb, start, end, Nst_s.e_ValueError, msg);
}

void Nst_set_internal_math_error_c(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
{
    Nst_set_internal_error_c(tb, start, end, Nst_s.e_MathError, msg);
}

void Nst_set_internal_call_error_c(Nst_Traceback *tb, Nst_Pos start,
                                   Nst_Pos end, const i8 *msg)
{
    Nst_set_internal_error_c(tb, start, end, Nst_s.e_CallError, msg);
}

void Nst_set_internal_import_error_c(Nst_Traceback *tb, Nst_Pos start,
                                     Nst_Pos end, const i8 *msg)
{
    Nst_set_internal_error_c(tb, start, end, Nst_s.e_ImportError, msg);
}

void Nst_internal_failed_allocation(Nst_Traceback *tb, Nst_Pos start,
                                    Nst_Pos end)
{
    Nst_set_internal_error(
        tb,
        start, end,
        Nst_s.e_MemoryError,
        STR(Nst_inc_ref(Nst_s.o_failed_alloc)));
}
