#include <string.h>
#include "nest.h"

#ifdef WINDOWS
#include "windows.h"
#endif

#define HELP_MESSAGE \
    "USAGE: nest [options] [filename | -c command] [args]\n" \
    "\n" \
    "Filename:\n" \
    "  The name of the file to execute\n" \
    "\n" \
    "Command:\n" \
    "  A string of code to execute\n" \
    "\n" \
    "Args:\n" \
    "  the arguments that will be accessible through _args_ during execution\n" \
    "\n" \
    "Options:\n" \
    "  -h -? --help          prints this message\n" \
    "  -V --version          prints the version of Nest being used\n" \
    "  -t --tokens           prints the list of tokens of the program\n" \
    "  -a --ast              prints the abstract syntax tree of the program\n" \
    "  -b --bytecode         prints the byte code of the program\n" \
    "  -f --force-execution  executes the program even when -t, -a or -b are used\n" \
    "  -D --no-default       does not set or optimize default variables such as\n" \
    "                        'true' or 'Int'; this does not affect the optimization\n" \
    "                        on imported modules\n" \
    "\n" \
    "  -O0                   do not optimize the program\n" \
    "  -O1                   optimize only expressions with known values\n" \
    "  -O2                   optimize bytecode instruction sequences that can be\n" \
    "                        made more concise\n" \
    "  -O3                   replace built-in names (e.g. 'true', 'Int', etc.) with\n" \
    "                        their corresponding value, this does not replace them\n" \
    "                        when they might be modified\n" \
    "\n" \
    "  -e --encoding         in the form -e=encoding, reads the file in the specified\n" \
    "                        encoding (e.g. --encoding=utf8)\n" \
    "\n" \
    "  -m --monochrome       prints the error messages without ANSI color escapes\n" \
    "                        sets also 'sys.SUPPORTS_COLOR' to false"

#define USAGE_MESSAGE \
    "USAGE: nest [options] [filename | -c command] [args]\n" \
    "Run 'nest --help' for more information\n"

#define VERSION_MESSAGE \
    "Using Nest version: " NST_VERSION

#define ENCODING_MESSAGE \
    "The supported encodings are:\n" \
    "- utf8 (aka utf-8)\n" \
    "- utf16 (aka utf-16, utf16le, utf-16le)\n" \
    "- utf16be (aka utf-16be)\n" \
    "- utf32 (aka utf-32, utf32le, utf-32le)\n" \
    "- windows-1250..windows-1258 (aka windows125x, cp-125x, cp125x)\n" \
    "- ascii (aka us-ascii)\n" \
    "- iso-8859-1 (aka iso8859-1, latin1, latin, l1\n" \
    "\n" \
    "All names are case-insensitive and underscores (_) and hyphens (-) are\n" \
    "interchangeable"

bool supports_color = true;

i32 _nst_parse_args(i32 argc, i8 **argv,
                    bool     *print_tokens,
                    bool     *print_ast,
                    bool     *print_bytecode,
                    bool     *force_execution,
                    Nst_CPID *encoding,
                    bool     *no_default,
                    i32      *opt_level,
                    i8      **command,
                    i8      **filename,
                    i32      *args_start)
{
    *print_tokens = false;
    *print_ast = false;
    *print_bytecode = false;
    *force_execution = false;
    *encoding = NST_CP_UNKNOWN;
    *no_default = false;
    *opt_level = 3;
    *command = NULL;
    *filename = NULL;
    *args_start = 1;

    if ( argc < 2 )
    {
        printf(USAGE_MESSAGE);
        return -1;
    }

    i32 i = 1;
    for ( ; i < argc; i++ )
    {
        i8 *arg = argv[i];
        i32 arg_len = (i32)strlen(arg);

        if ( arg[0] == '-' )
        {
            if ( arg_len == 1 )
            {
                printf("Invalid option: -\n");
                printf("\n" USAGE_MESSAGE);
                return -1;
            }

            for ( i32 j = 1; j < (i32)arg_len; j++)
            {
                switch ( arg[j] )
                {
                case 't': *print_tokens    = true; break;
                case 'a': *print_ast       = true; break;
                case 'b': *print_bytecode  = true; break;
                case 'f': *force_execution = true; break;
                case 'D': *no_default      = true; break;
                case 'm': supports_color   = false;break;
                case 'e':
                    if ( j != 1 || arg_len < 4 || arg[2] != '=' )
                    {
                        printf("Invalid usage of the option: -e\n");
                        printf("\n" USAGE_MESSAGE);
                        return -1;
                    }
                    *encoding = nst_encoding_from_name(arg + 3);
                    if ( *encoding == NST_CP_UNKNOWN )
                    {
                        printf("Unknown encoding %s\n", arg + 3);
                        printf("\n" ENCODING_MESSAGE);
                        return -1;
                    }
                    j = arg_len;
                    break;
                case 'h':
                case '?':
                    printf(HELP_MESSAGE);
                    return 1;

                case 'V':
                    printf(VERSION_MESSAGE "\n");
                    return 1;

                case 'O':
                    if ( j != 1 )
                    {
                        printf("Invalid option: -O\n");
                        printf("\n" USAGE_MESSAGE);
                        return -1;
                    }
                    else if ( arg_len != 3 )
                    {
                        printf("Invalid option: %s\n", arg);
                        printf("\n" USAGE_MESSAGE);
                        return -1;
                    }

                    {
                        i32 level = arg[j + 1] - '0';

                        if ( level < 0 || level > 3)
                        {
                            printf("Invalid option: -O%c\n", (i8)(level + '0'));
                            printf("\n" USAGE_MESSAGE);
                            return -1;
                        }

                        *opt_level = level;
                        j = (i32)arg_len;
                    }
                    break;

                case 'c':
                    if ( j != 1 || arg_len != 2 || i + 1 == argc )
                    {
                        printf("Invalid usage of the option: -c\n");
                        printf("\n" USAGE_MESSAGE);
                        return -1;
                    }

                    *command = argv[i + 1];
                    *args_start = i + 2;
                    return 0;

                case '-':
                    if ( j != 1 )
                    {
                        printf("Invalid option: --\n");
                        printf("\n" USAGE_MESSAGE);
                        return -1;
                    }
                    else if ( arg_len == 2 )
                    {
                        if ( ++i < argc )
                        {
                            *filename = argv[i];
                        }
                        else
                        {
                            printf("No file provided\n");
                            printf("\n" USAGE_MESSAGE);
                            return -1;
                        }

                        *args_start = ++i;

                        return 0;
                    }

                    if ( strcmp(arg, "--tokens") == 0 )
                    {
                        *print_tokens = true;
                    }
                    else if ( strcmp(arg, "--ast") == 0 )
                    {
                        *print_ast = true;
                    }
                    else if ( strcmp(arg, "--bytecode") == 0 )
                    {
                        *print_bytecode = true;
                    }
                    else if ( strcmp(arg, "--force-execution") == 0 )
                    {
                        *force_execution = true;
                    }
                    else if ( strcmp(arg, "--monochrome") == 0 )
                    {
                        supports_color = false;
                    }
                    else if ( strcmp(arg, "--no-default") == 0 )
                    {
                        *no_default = true;
                    }
                    else if ( strcmp(arg, "--help") == 0 )
                    {
                        printf(HELP_MESSAGE);
                        return 1;
                    }
                    else if ( strcmp(arg, "--version") == 0 )
                    {
                        printf(VERSION_MESSAGE "\n");
                        return 1;
                    }
                    else if ( strncmp(arg, "--encoding", 10) == 0 )
                    {
                        if ( j != 1 || arg_len < 12 || arg[10] != '=' )
                        {
                            printf("Invalid usage of the option: --encoding\n");
                            printf("\n" USAGE_MESSAGE);
                            return -1;
                        }
                        *encoding = nst_encoding_from_name(arg + 11);
                        if ( *encoding == NST_CP_UNKNOWN )
                        {
                            printf("Unknown encoding %s\n", arg + 11);
                            printf("\n" ENCODING_MESSAGE);
                            return -1;
                        }
                    }
                    else
                    {
                        printf("Invalid option: %s\n", arg);
                        printf("\n" USAGE_MESSAGE);
                        return -1;
                    }

                    j = (i32)arg_len;
                    break;

                default:
                    printf("Invalid option: -%c\n", arg[j]);
                    printf("\n" USAGE_MESSAGE);
                    return -1;
                }
            }
        }
        else
        {
            *filename = arg;
            *args_start = i + 1;
            return 0;
        }
    }

    if ( ++i < argc )
    {
        *filename = argv[i];
    }
    else
    {
        printf("No file provided\n");
        printf("\n" USAGE_MESSAGE);
        return -1;
    }

    *args_start = ++i;

    return 0;
}

bool nst_supports_color()
{
    return supports_color;
}

#ifdef WINDOWS

struct {
    HANDLE hd;
    FILE *fp;
    wchar_t buf[1024];
    i8 ch[4];
    i32 buf_size;
    i32 buf_ptr;
    i32 ch_idx;
} w_in;

bool _nst_wargv_to_argv(int       argc,
                        wchar_t **wargv,
                        i8     ***argv,
                        i8      **argv_content)
{
    usize tot_size = 0;
    for ( i32 i = 0; i < argc; i++ )
    {
        tot_size += (wcslen(wargv[i])) * 3 + 1;
    }
    i8 **local_argv = (i8 **)nst_raw_malloc(argc * sizeof(i8 *));
    i8 *local_argv_content = (i8 *)nst_raw_malloc(tot_size * sizeof(i8 *));

    if ( local_argv == NULL || local_argv_content == NULL )
    {
        nst_free(local_argv);
        nst_free(local_argv_content);
        puts("Failed allocation while converting argv");
        return false;
    }

    i8 *argv_ptr = local_argv_content;

    for ( i32 i = 0; i < argc; i++ )
    {
        wchar_t *warg = wargv[i];
        local_argv[i] = argv_ptr;

        for ( usize j = 0, n = wcslen(warg); j < n; j++ )
        {
            usize ch_len = nst_check_utf16_bytes(warg + j, n - j);
            if ( ch_len < 0 )
            {
                nst_free(local_argv);
                nst_free(local_argv_content);
                puts("Invalid argv enconding");
                return false;
            }
            argv_ptr += nst_utf16_to_utf8(argv_ptr, warg + j, n - j);
            j += ch_len - 1;
        }
        *argv_ptr++ = '\0';
    }
    *argv = local_argv;
    *argv_content = local_argv_content;

    return true;
}

void _nst_set_console_mode()
{
    SetErrorMode(SEM_FAILCRITICALERRORS);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if ( stdout_handle == INVALID_HANDLE_VALUE )
    {
        supports_color = false;
        return;
    }

    DWORD stdout_prev_mode = 0;
    if (!GetConsoleMode(stdout_handle, &stdout_prev_mode))
    {
        supports_color = false;
        return;
    }

    DWORD stdout_new_mode = ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    DWORD stdout_mode = stdout_prev_mode | stdout_new_mode;
    if ( !SetConsoleMode(stdout_handle, stdout_mode) )
    {
        stdout_new_mode = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        stdout_mode = stdout_new_mode | stdout_new_mode;
        if ( !SetConsoleMode(stdout_handle, stdout_mode) )
        {
            supports_color = false;
        }
    }

    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    if ( stdin_handle == INVALID_HANDLE_VALUE )
    {
        w_in.hd = NULL;
        return;
    }
    w_in.hd = stdin_handle;
    w_in.fp = stdin;
    w_in.buf_ptr = 0;
    w_in.buf_size = 0;
    w_in.ch_idx = 0;
    w_in.ch[0] = 0;
    w_in.ch[1] = 0;
    w_in.ch[2] = 0;
    w_in.ch[3] = 0;
}

static bool read_characters()
{
    DWORD len;
    if ( !ReadConsoleW(w_in.hd, w_in.buf, 1024, &len, NULL) )
    {
        return false;
    }
    w_in.buf_size = (usize)len;
    w_in.buf_ptr = 0;
    return true;
}

static bool get_byte(i8 *out_ch)
{
    if ( w_in.ch_idx < 4 && w_in.ch[w_in.ch_idx] != 0 )
    {
        *out_ch = w_in.ch[w_in.ch_idx++];
        return true;
    }

    if ( w_in.buf_ptr >= w_in.buf_size )
    {
        if ( !read_characters() )
        {
            return false;
        }
    }
    i32 ch_len = nst_check_utf16_bytes(w_in.buf + w_in.buf_ptr, w_in.buf_size - w_in.buf_ptr);
    if ( ch_len < 0 )
    {
        return false;
    }

    w_in.ch[0] = 0;
    w_in.ch[1] = 0;
    w_in.ch[2] = 0;
    w_in.ch[3] = 0;
    nst_utf16_to_utf8(w_in.ch, w_in.buf + w_in.buf_ptr, (usize)ch_len);
    w_in.buf_ptr += ch_len;
    w_in.ch_idx = 1;
    *out_ch = w_in.ch[0];
    return true;
}

usize _nst_windows_stdin_read(i8 *buf, usize size, usize count, void *f_value)
{
    if ( w_in.hd == NULL )
    {
        return fread(buf, size, count, (FILE *)f_value);
    }
    usize bytes = size * count;

    for ( usize i = 0; i < bytes; i++ )
    {
        if ( !get_byte(buf) )
        {
            return i;
        }
        buf++;
    }
    return bytes;
}

#endif // !WINDOWS
