#include <string.h>
#include "nest.h"

static Nst_SourceText *source_text_new(void);
static bool add_lines(Nst_SourceText *text);
static bool normalize_encoding(Nst_SourceText *text, Nst_EncodingID encoding);
static void parse_option(char *opt, Nst_CLArgs *args);
static void read_first_line(char *text, usize len, Nst_CLArgs *args);

static Nst_SourceText *load_file(Nst_CLArgs *inout_args, bool parse_line);
static Nst_SourceText *load_command(Nst_CLArgs *inout_args, bool parse_line);

static Nst_PtrArray loaded_texts;

bool _Nst_source_loader_init(void)
{
    return Nst_pa_init(&loaded_texts, 20);
}

void _Nst_source_loader_quit(void)
{
    Nst_pa_clear(&loaded_texts, (Nst_Destructor)Nst_source_text_destroy);
}

Nst_SourceText *Nst_source_load(Nst_CLArgs *inout_args)
{

    Nst_SourceText *src_text;
    if (inout_args->filename != NULL)
        src_text = load_file(inout_args, true);
    else if (inout_args->command != NULL)
        src_text = load_command(inout_args, true);
    else {
        Nst_error_setc_value("Nst_source_load: invalid arguments");
        return NULL;
    }
    if (src_text != NULL && !Nst_pa_append(&loaded_texts, src_text)) {
        Nst_source_text_destroy(src_text);
        return NULL;
    }
    return src_text;
}

Nst_SourceText *Nst_source_from_sv(Nst_StrView sv)
{
    Nst_CLArgs args;
    Nst_cl_args_init(&args, 0, NULL);
    args.command = (char *)sv.value;
    args.encoding = Nst_EID_EXT_UTF8;
    return load_command(&args, false);
}

Nst_SourceText *Nst_source_from_file(const char *path, Nst_EncodingID encoding)
{
    Nst_CLArgs args;
    Nst_cl_args_init(&args, 0, NULL);
    args.filename = (char *)path;
    args.encoding = encoding;
    return load_file(&args, false);
}

static Nst_SourceText *load_file(Nst_CLArgs *inout_args, bool parse_line)
{
    Nst_SourceText *src_text = NULL;
    FILE *file = NULL;
    char *text = NULL;
    char *full_path = NULL;

    file = Nst_fopen_unicode(inout_args->filename, "rb");

    if (file == NULL) {
        if (!Nst_error_occurred()) {
            Nst_error_setf_value(
                "File \"%.100s\" not found.",
                inout_args->filename);
        }
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    text = Nst_calloc(size + 1, sizeof(char), NULL);
    if (text == NULL)
        goto cleanup;

    usize str_len = fread(text, sizeof(char), size + 1, file);
    text[str_len] = '\0';
    fclose(file);
    file = NULL;

    Nst_abs_path(inout_args->filename, &full_path, NULL);
    if (full_path == NULL)
        goto cleanup;

    if (parse_line)
        read_first_line(text, str_len, inout_args);

    src_text = source_text_new();
    if (src_text == NULL)
        goto cleanup;
    src_text->text = text;
    src_text->text_len = str_len;
    src_text->path = full_path;

    if (!normalize_encoding(src_text, inout_args->encoding))
        goto cleanup;
    text = NULL;
    if (!add_lines(src_text))
        goto cleanup;
    return src_text;

cleanup:
    if (file != NULL)
        fclose(file);
    if (text != NULL)
        Nst_free(text);
    if (full_path != NULL)
        Nst_free(full_path);
    if (src_text != NULL) {
        if (src_text->lines != NULL)
            Nst_free(src_text->lines);
        Nst_free(src_text);
    }
    return NULL;
}

static Nst_SourceText *load_command(Nst_CLArgs *inout_args, bool parse_line)
{
    Nst_SourceText *src_text = NULL;
    char *text = NULL;

    usize str_len = strlen(inout_args->command);

    text = Nst_calloc(1, str_len + 1, inout_args->command);
    if (text == NULL)
        goto cleanup;

    if (parse_line)
        read_first_line(text, str_len, inout_args);
    src_text = source_text_new();
    if (src_text == NULL)
        goto cleanup;

    src_text->text = text;
    src_text->text_len = str_len;
    src_text->path = NULL;

    // ignore the encoding parsed in the first line if any
    if (!normalize_encoding(src_text, Nst_EID_UNKNOWN))
        goto cleanup;
    text = NULL;
    if (!add_lines(src_text))
        goto cleanup;

    return src_text;

cleanup:
    if (text != NULL)
        Nst_free(text);
    if (src_text != NULL) {
        if (src_text->lines != NULL)
            Nst_free(src_text->lines);
        Nst_free(src_text);
    }
    return NULL;
}

static Nst_SourceText *source_text_new(void)
{
    Nst_SourceText *src = Nst_malloc_c(1, Nst_SourceText);
    if (src == NULL)
        return NULL;

    src->text = NULL;
    src->path = NULL;
    src->lines = NULL;
    src->text_len = 0;
    src->lines_len = 0;
    return src;
}

void Nst_source_text_destroy(Nst_SourceText *text)
{
    if (text == NULL)
        return;

    if (text->text != NULL)
        Nst_free(text->text);
    if (text->lines != NULL)
        Nst_free(text->lines);
    if (text->path != NULL)
        Nst_free(text->path);

    Nst_free(text);
}

bool add_lines(Nst_SourceText *text)
{
    char *text_p = text->text;
    char **starts = (char **)Nst_calloc(100, sizeof(char *), NULL);
    if (starts == NULL) {
        text->lines = NULL;
        text->lines_len = 0;
        return false;
    }

    starts[0] = text_p;
    usize line_count = 1;

    // normalize line endings

    // if the file contains \n, then \n doesn't change and \r\n becomes just \n
    // if the file only contains \r, it is replaced with \n

    bool remove_r = false;
    for (usize i = 0, n = text->text_len; i < n; i++) {
        if (text_p[i] == '\n') {
            remove_r = true;
            break;
        }
    }

    usize offset = 0;
    for (usize i = 0, n = text->text_len; i < n; i++) {
        if (text_p[i] != '\r')
            text_p[i - offset] = text_p[i];
        else if (remove_r)
            offset++;
        else
            text_p[i] = '\n';
    }

    text->text_len = text->text_len - offset;
    text->text[text->text_len] = '\0';

    // now all lines end with \n
    for (usize i = 0, n = text->text_len; i < n; i++) {
        if (text_p[i] != '\n')
            continue;

        if (i + 1 == n) {
            text->lines = starts;
            text->lines_len = line_count;
        }

        line_count++;

        if (line_count % 100 == 0) {
            char **temp = (char **)Nst_realloc(
                starts,
                i + 100,
                sizeof(char *),
                0);
            if (temp == NULL) {
                Nst_free(starts);
                text->lines = NULL;
                text->lines_len = 0;
                return false;
            }
            starts = temp;
        }

        starts[line_count - 1] = text_p + i + 1;
    }

    text->lines = starts;
    text->lines_len = line_count;
    return true;
}

bool normalize_encoding(Nst_SourceText *text, Nst_EncodingID encoding)
{
    i32 bom_size = 0;
    if (encoding == Nst_EID_UNKNOWN)
        encoding = Nst_encoding_detect(text->text, text->text_len, &bom_size);
    else
        Nst_check_bom(text->text, text->text_len, &bom_size);

    encoding = Nst_encoding_to_single_byte(encoding);
    Nst_Encoding *from = Nst_encoding(encoding);

    Nst_Pos pos = { 0, 0, text };
    Nst_StrBuilder sb;
    if (!Nst_sb_init(&sb, text->text_len + 40)) {
        Nst_error_add_span(Nst_span_from_pos(pos));
        return false;
    }

    isize n = (isize)text->text_len - bom_size;
    u8 *text_p = (u8 *)text->text + bom_size;

    bool skip_line_feed = false;

    while (n > 0) {
        // Decode character
        i32 ch_len = from->check_bytes(text_p, n);
        if (ch_len < 0) {
            Nst_sb_destroy(&sb);
            Nst_error_setf_syntax(
                "could not encode byte %ib for %s encoding",
                *text_p, from->name);
            Nst_error_add_span(Nst_span_from_pos(pos));
            return false;
        }
        usize ch_size = ch_len * from->ch_size;
        u32 utf32_ch = from->to_utf32(text_p);
        text_p += ch_size;
        n -= ch_len;

        if (utf32_ch == '\n' && !skip_line_feed) {
            pos.line++;
            pos.col = 0;
        }
        if (skip_line_feed)
            skip_line_feed = false;

        if (utf32_ch == '\r') {
            pos.line++;
            pos.col = 0;
            skip_line_feed = true;
        }

        // Re-encode character
        if (!Nst_sb_reserve(&sb, 5)) {
            Nst_sb_destroy(&sb);
            Nst_error_add_span(Nst_span_from_pos(pos));
            return false;
        }
        ch_len = Nst_encoding_ext_utf8.from_utf32(utf32_ch, sb.value + sb.len);
        sb.len += ch_len;
        pos.col++;
    }
    sb.value[sb.len] = 0;

    Nst_free(text->text);
    text->text = (char *)sb.value;
    text->text_len = sb.len;
    return true;
}

static void parse_option(char *opt, Nst_CLArgs *args)
{
    if (strcmp(opt, "-O0") == 0)
        args->opt_level = 0;
    else if (strcmp(opt, "-O1") == 0)
        args->opt_level = 1;
    else if (strcmp(opt, "-O2") == 0)
        args->opt_level = 2;
    else if (strcmp(opt, "-O3") == 0)
        args->opt_level = 3;
    else if (strcmp(opt, "--no-default") == 0)
        args->no_default = true;
    else if (strncmp(opt, "--encoding=", 11) == 0) {
        Nst_EncodingID new_encoding = Nst_encoding_from_name(opt + 11);
        new_encoding = Nst_encoding_to_single_byte(new_encoding);
        if (new_encoding != Nst_EID_UNKNOWN)
            args->encoding = new_encoding;
    }
}

static void read_first_line(char *text, usize len, Nst_CLArgs *args)
{
    i32 bom_size;
    Nst_check_bom(text, len, &bom_size);
    text += bom_size;
    len -= bom_size;

    // the first line must start with --$
    if (len < 3 || strncmp(text, "--$", 3) != 0)
        return;

    // max length: '--encoding=' + 15 characters for the value
    char curr_opt[27];
    usize i = 0;
    char ch = 0;
    text += 3;
    len -= 3;

    while ((ch = *text++) != '\n' && ch != '\r' && len-- != 0) {
        if (ch != ' ') {
            if (i < 26)
                curr_opt[i] = ch;
            i++;
            continue;
        }
        if (i > 26 || i == 0) {
            i = 0;
            continue;
        }
        curr_opt[i] = '\0';
        parse_option(curr_opt, args);
        i = 0;
    }

    if (i > 12 || i == 0)
        return;

    curr_opt[i] = '\0';
    parse_option(curr_opt, args);
}
