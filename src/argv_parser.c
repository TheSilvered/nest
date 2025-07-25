#include <string.h>
#include "nest.h"

#ifdef Nst_MSVC
#include "windows.h"
#endif // !Nst_MSVC

#include "file.h"

#define HELP_MESSAGE                                                                     \
    "USAGE: nest [options] <filename | -c command> [args]\n\n"                           \
                                                                                         \
    "Filename:\n"                                                                        \
    "  The name of the file to execute\n\n"                                              \
                                                                                         \
    "Command:\n"                                                                         \
    "  A string of code to execute\n\n"                                                  \
                                                                                         \
    "Args:\n"                                                                            \
    "  the arguments that will be accessible through _args_ during execution\n\n"        \
                                                                                         \
    "Options:\n"                                                                         \
    "  -h -? --help          prints this message\n"                                      \
    "  -V --version          prints the version of Nest being used\n"                    \
    "  -t --tokens           prints the list of tokens of the program\n"                 \
    "  -a --ast              prints the abstract syntax tree of the program\n"           \
    "  -i --instructions     prints the instructions of the program\n"                   \
    "  -b --bytecode         prints the bytecode of the program\n"                       \
    "  -f --force-execution  executes the program even when -t, -a or -b are used\n"     \
    "  -D --no-default       does not set or optimize default variables such as\n"       \
    "                        'true' or 'Int'; this does not affect the optimization\n"   \
    "                        on imported modules\n\n"                                    \
                                                                                         \
    "  -O0                   do not optimize the program\n"                              \
    "  -O1                   optimize only expressions with known values\n"              \
    "  -O2                   optimize bytecode instruction sequences that can be\n"      \
    "                        made more concise\n"                                        \
    "  -O3                   replace built-in names (e.g. 'true', 'Int', etc.) with\n"   \
    "                        their corresponding value, this does not replace them\n"    \
    "                        when they might be modified\n\n"                            \
                                                                                         \
    "  -e --encoding         in the form -e=encoding, reads the file in the specified\n" \
    "                        encoding (e.g. --encoding=utf8)\n\n"                        \
                                                                                         \
    "  -m --monochrome       prints the error messages without ANSI color escapes\n"     \
    "                        sets also 'sys.SUPPORTS_COLOR' to false\n"

#define USAGE_MESSAGE                                                         \
    "USAGE: nest [options] <filename | -c command> [args]\n"                  \
    "Run 'nest --help' for more information\n"

#define VERSION_MESSAGE                                                       \
    "Using Nest version: " Nst_VERSION " (" __DATE__ ", " __TIME__ ")\n"

#define ENCODING_MESSAGE                                                             \
    "The supported encodings are:\n"                                                 \
    "- utf8 (aka utf-8)\n"                                                           \
    "- ext-utf8 (aka ext-utf-8, extutf-8, extutf8)\n"                                \
    "- utf16 (aka utf-16, utf16le, utf-16le)\n"                                      \
    "- ext-utf16 (aka ext-utf-16, extutf-16, extutf16, ext-utf16le, ext-utf-16le,\n" \
    "             extutf-16le, extutf16le)\n"                                        \
    "- utf16be (aka utf-16be)\n"                                                     \
    "- utf32 (aka utf-32, utf32le, utf-32le)\n"                                      \
    "- utf32be (aka utf-32be)\n"                                                     \
    "- windows-1250..windows-1258 (aka windows125x, cp-125x, cp125x)\n"              \
    "- ascii (aka us-ascii)\n"                                                       \
    "- iso-8859-1 (aka iso8859-1, latin1, latin-1, latin, l1)\n\n"                   \
                                                                                     \
    "All names are case-insensitive. Underscores (_), hyphens (-) and spaces are\n"  \
    "interchangeable.\n"

#ifdef Nst_MSVC
static bool supports_color = false;
#else
static bool supports_color = true;
#endif // !Nst_MSVC

void Nst_cl_args_init(Nst_CLArgs *args, int argc, char **argv)
{
    args->argc = argc;
    args->argv = argv;
    args->print_tokens = false;
    args->print_ast = false;
    args->print_instructions = false;
    args->print_bytecode = false;
    args->force_execution = false;
    args->encoding = Nst_EID_UNKNOWN;
    args->no_default = false;
    args->opt_level = 3;
    args->command = NULL;
    args->filename = NULL;
    args->args_start = argc >= 1 ? 1 : 0;
}

static i32 long_arg(char *arg, Nst_CLArgs *cl_args)
{
    if (strcmp(arg, "--tokens") == 0)
        cl_args->print_tokens = true;
    else if (strcmp(arg, "--ast") == 0)
        cl_args->print_ast = true;
    else if (strcmp(arg, "--instructions") == 0)
        cl_args->print_instructions = true;
    else if (strcmp(arg, "--bytecode") == 0)
        cl_args->print_bytecode = true;
    else if (strcmp(arg, "--force-execution") == 0)
        cl_args->force_execution = true;
    else if (strcmp(arg, "--monochrome") == 0)
        supports_color = false;
    else if (strcmp(arg, "--no-default") == 0)
        cl_args->no_default = true;
    else if (strcmp(arg, "--help") == 0) {
        Nst_printf(HELP_MESSAGE);
        return 1;
    } else if (strcmp(arg, "--version") == 0) {
        Nst_printf(VERSION_MESSAGE);
        return 1;
    } else if (strncmp(arg, "--encoding", 10) == 0) {
        if (strlen(arg) < 12 || arg[10] != '=') {
            Nst_printf("Invalid usage of the option: --encoding\n");
            Nst_printf("\n" USAGE_MESSAGE);
            return -1;
        }
        cl_args->encoding = Nst_encoding_from_name(arg + 11);
        if (cl_args->encoding == Nst_EID_UNKNOWN) {
            Nst_printf("Unknown encoding %s\n", arg + 11);
            Nst_printf("\n" ENCODING_MESSAGE);
            return -1;
        }
        cl_args->encoding = Nst_encoding_to_single_byte(cl_args->encoding);
    } else {
        Nst_printf("Invalid option: %s\n", arg);
        Nst_printf("\n" USAGE_MESSAGE);
        return -1;
    }
    return 0;
}

i32 Nst_cl_args_parse(Nst_CLArgs *cl_args)
{
    int argc = cl_args->argc;
    char **argv = cl_args->argv;

    if (argc < 2) {
        Nst_printf("No file provided\n");
        Nst_printf(USAGE_MESSAGE);
        return -1;
    }

    int i;
    for (i = 1; i < argc; i++) {
        char *arg = argv[i];
        usize arg_len = strlen(arg);

        if (arg[0] != '-') {
            cl_args->filename = arg;
            cl_args->args_start = i + 1;
            return 0;
        }

        if (arg_len == 1) {
            Nst_printf("Invalid option: -\n");
            Nst_printf("\n" USAGE_MESSAGE);
            return -1;
        }

        for (i32 j = 1; j < (i32)arg_len; j++) {
            switch (arg[j]) {
            case 't': cl_args->print_tokens       = true; break;
            case 'a': cl_args->print_ast          = true; break;
            case 'i': cl_args->print_instructions = true; break;
            case 'b': cl_args->print_bytecode     = true; break;
            case 'f': cl_args->force_execution    = true; break;
            case 'D': cl_args->no_default         = true; break;
            case 'm': supports_color              = false;break;
            case 'h':
            case '?':
                Nst_printf(HELP_MESSAGE);
                return 1;
            case 'V':
                Nst_printf(VERSION_MESSAGE);
                return 1;
            case 'e':
                if (j != 1 || arg_len < 4 || arg[2] != '=') {
                    Nst_printf("Invalid usage of the option: -e\n");
                    Nst_printf("\n" USAGE_MESSAGE);
                    return -1;
                }
                cl_args->encoding = Nst_encoding_from_name(arg + 3);
                if (cl_args->encoding == Nst_EID_UNKNOWN) {
                    Nst_printf("Unknown encoding %s\n", arg + 3);
                    Nst_printf("\n" ENCODING_MESSAGE);
                    return -1;
                }
                cl_args->encoding =
                    Nst_encoding_to_single_byte(cl_args->encoding);
                j = (i32)arg_len;
                break;
            case 'O': {
                if (j != 1) {
                    Nst_printf("Invalid option: -O\n");
                    Nst_printf("\n" USAGE_MESSAGE);
                    return -1;
                } else if (arg_len != 3) {
                    Nst_printf("Invalid option: %s\n", arg);
                    Nst_printf("\n" USAGE_MESSAGE);
                    return -1;
                }

                u8 level = arg[j + 1] - '0';

                if (level > 3) {
                    Nst_printf("Invalid option: -O%c\n", arg[j + 1]);
                    Nst_printf("\n" USAGE_MESSAGE);
                    return -1;
                }

                cl_args->opt_level = level;
                j = (i32)arg_len;
                break;
            }
            case 'c':
                if (j != 1 || arg_len != 2 || i + 1 == argc) {
                    Nst_printf("Invalid usage of the option: -c\n");
                    Nst_printf("\n" USAGE_MESSAGE);
                    return -1;
                }

                cl_args->command = argv[i + 1];
                cl_args->args_start = i + 2;
                return 0;
            case '-':
                if (j != 1) {
                    Nst_printf("Invalid option: --\n");
                    Nst_printf("\n" USAGE_MESSAGE);
                    return -1;
                } else if (arg_len == 2) {
                    if (++i < argc)
                        cl_args->filename = argv[i];
                    else {
                        Nst_printf("No file provided\n");
                        Nst_printf("\n" USAGE_MESSAGE);
                        return -1;
                    }

                    cl_args->args_start = ++i;
                    return 0;
                }

                i32 result = long_arg(arg, cl_args);
                if (result != 0)
                    return result;

                j = (i32)arg_len;
                break;

            default:
                Nst_printf("Invalid option: -%c\n", arg[j]);
                Nst_printf("\n" USAGE_MESSAGE);
                return -1;
            }
        }
    }

    if (++i < argc)
        cl_args->filename = argv[i];
    else {
        Nst_printf("No file provided\n");
        Nst_printf("\n" USAGE_MESSAGE);
        return -1;
    }

    cl_args->args_start = ++i;
    return 0;
}

bool Nst_supports_color(void)
{
    return supports_color;
}

void _Nst_supports_color_override(bool value)
{
    supports_color = value;
}

#ifdef Nst_MSVC

bool _Nst_wargv_to_argv(int argc, wchar_t **wargv, char ***argv)
{
    usize tot_size = 0;

    for (i32 i = 0; i < argc; i++)
        tot_size += (wcslen(wargv[i])) * 3 + 1;

    char **local_argv = (char **)Nst_raw_malloc(
        ((argc + 1) * sizeof(char *))
      + (tot_size * sizeof(char)));

    if (local_argv == NULL) {
        Nst_free(local_argv);
        puts("Failed allocation while converting argv");
        return false;
    }

    char *argv_ptr = (char *)(local_argv + argc + 1);

    for (i32 i = 0; i < argc; i++) {
        wchar_t *warg = wargv[i];
        local_argv[i] = argv_ptr;

        for (usize j = 0, n = wcslen(warg); j < n; j++) {
            i32 ch_len = Nst_check_utf16_bytes(warg + j, n - j);
            if (ch_len < 0) {
                Nst_free(local_argv);
                puts("Invalid argv enconding");
                return false;
            }
            argv_ptr += Nst_utf16_to_utf8((u8 *)argv_ptr, warg + j, n - j);
            j += ch_len - 1;
        }
        *argv_ptr++ = '\0';
    }
    *argv = local_argv;
    return true;
}

void _Nst_console_mode_init(void)
{
    supports_color = true;

    SetErrorMode(SEM_FAILCRITICALERRORS);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Nst_stdin.hd = NULL;
    Nst_stdin.fp = stdin;
    Nst_stdin.buf_ptr = 0;
    Nst_stdin.buf_size = 0;

    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdout_handle == INVALID_HANDLE_VALUE) {
        supports_color = false;
        goto try_stdin;
    }

    DWORD stdout_prev_mode = 0;
    if (!GetConsoleMode(stdout_handle, &stdout_prev_mode)) {
        supports_color = false;
        goto try_stdin;
    }

    DWORD stdout_mode = stdout_prev_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(stdout_handle, stdout_mode))
        supports_color = false;

    HANDLE stdin_handle;
try_stdin:
    stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (stdin_handle == INVALID_HANDLE_VALUE)
        return;
    Nst_stdin.hd = stdin_handle;
}

#endif // !Nst_MSVC
