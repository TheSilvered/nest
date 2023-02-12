#include "argv_parser.h"
#include "nest.h"

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
    "                        more concise\n" \
    "  -O3                   replace built-in names (e.g. 'true', 'Int', etc.) with\n" \
    "                        their corresponding value, this does not replace them\n" \
    "                        when they might be modified\n" \
    "\n" \
    "  --cp1252              reads the file using the CP-1252 encoding\n" \
    "\n" \
    "  -m --monochrome       prints the error messages without ANSI color escapes\n"

#define USAGE_MESSAGE \
    "USAGE: nest [options] [filename | -c command] [args]\n" \
    "Run 'nest --help' for more information\n"

#define VERSION_MESSAGE \
    "Using Nest version: " NST_VERSION

int nst_parse_args(int argc, char **argv,
                   bool  *print_tokens,
                   bool  *print_ast,
                   bool  *print_bytecode,
                   bool  *force_execution,
                   bool  *monochrome,
                   bool  *force_cp1252,
                   bool  *no_default,
                   int   *opt_level,
                   char **command,
                   char **filename,
                   int   *args_start)
{
    *print_tokens = false;
    *print_ast = false;
    *print_bytecode = false;
    *force_execution = false;
    *monochrome = false;
    *force_cp1252 = false;
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

    int i = 1;
    for ( ; i < argc; i++ )
    {
        char *arg = argv[i];
        int arg_len = (int)strlen(arg);

        if ( arg[0] == '-' )
        {
            if ( arg_len == 1 )
            {
                printf("Invalid option: -\n");
                printf("\n" USAGE_MESSAGE);
                return -1;
            }

            for ( int j = 1; j < (int)arg_len; j++)
            {
                switch ( arg[j] )
                {
                case 't': *print_tokens    = true; break;
                case 'a': *print_ast       = true; break;
                case 'b': *print_bytecode  = true; break;
                case 'f': *force_execution = true; break;
                case 'm': *monochrome      = true; break;
                case 'D': *no_default      = true; break;
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
                        int level = arg[j + 1] - '0';

                        if ( level < 0 || level > 3)
                        {
                            printf("Invalid option: -O%c\n", (char)(level + '0'));
                            printf("\n" USAGE_MESSAGE);
                            return -1;
                        }

                        *opt_level = level;
                        j = (int)arg_len;
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
                        *monochrome = true;
                    }
                    else if ( strcmp(arg, "--cp1252") == 0 )
                    {
                        *force_cp1252 = true;
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
                    else
                    {
                        printf("Invalid option: %s\n", arg);
                        printf("\n" USAGE_MESSAGE);
                        return -1;
                    }

                    j = (int)arg_len;
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
