#include <stdio.h>
#include <string.h>
#include "argv_parser.h"
#include "nest.h"

#define HELP_MESSAGE \
    "USAGE: nest [options] [filename | -c command] [args]\n\n" \
    \
    "Filename:\n" \
    "  The name of the file to execute\n\n" \
    \
    "Command:\n" \
    "  A string of code to execute\n\n" \
    \
    "Args:\n" \
    "  the arguments that will be accessible through _args_ during execution\n\n" \
    \
    "Options:\n" \
    "  -h -? --help: prints this message\n" \
    "  -V --version: prints the version of nest being used\n" \
    "  -t --tokens: prints the list of tokens of the program\n" \
    "  -a --ast: prints the abstract syntax tree of the program\n" \
    "  -b --bytecode: prints the byte code of the program\n" \
    "  -f --force-execution: executes the program even when -t, -a or -b are used\n" \
    "  -O0: do not optimize the program (note that it is the letter O and not the number zero)\n" \
    "  -O1: optimize only the abstract syntax tree (e.g. basic expressions)\n" \
    "  -O2: optimize byte code instruction sequences that can be more concise\n" \
    "  -O3: replace built-in names (e.g. 'true', 'Int', etc.) with their corresponding value\n" \
    "       this does not replace them when they are re-purposed in the scope\n" \

#define USAGE_MESSAGE \
    "USAGE: nest [options] [filename | -c command] [args]\n" \
    "Run 'nest --help' for more information\n"

int nst_parse_args(int argc, char **argv,
                   bool *print_tokens,
                   bool *print_ast,
                   bool *print_bytecode,
                   bool *force_execution,
                   int  *opt_level,
                   char **command,
                   char **filename,
                   int *args_start)
{
    *print_tokens = false;
    *print_ast = false;
    *print_bytecode = false;
    *force_execution = false;
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
                case 't': *print_tokens = true;   break;
                case 'a': *print_ast = true;      break;
                case 'b': *print_bytecode = true; break;
                case 'f': *force_execution = true; break;
                case 'h':
                case '?':
                    printf(HELP_MESSAGE);
                    return 1;

                case 'V':
                    printf("Using nest version "NEST_VERSION "\n");
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

                    int level = arg[j + 1] - '0';

                    if ( level < 0 || level > 3)
                    {
                        printf("Invalid option: -O%c\n", (char)(level + '0'));
                        printf("\n" USAGE_MESSAGE);
                        return -1;
                    }

                    *opt_level = level;
                    j = (int)arg_len;
                    break;

                case 'c':
                    if ( j != 1 || arg_len != 2 || i + 1 == argc )
                    {
                        printf("Invalid option: -c\n");
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
                        goto end_args;

                    if ( strcmp(arg, "--tokens") == 0 )
                        *print_tokens = true;
                    else if ( strcmp(arg, "--ast") == 0 )
                        *print_ast = true;
                    else if ( strcmp(arg, "--bytecode") == 0 )
                        *print_bytecode = true;
                    else if ( strcmp(arg, "--force-execution") == 0 )
                        *force_execution = true;
                    else if ( strcmp(arg, "--help") == 0 )
                    {
                        printf(HELP_MESSAGE);
                        return 1;
                    }
                    else if ( strcmp(arg, "--version") == 0 )
                    {
                        printf("Using nest version "NEST_VERSION "\n");
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

    end_args: if ( ++i < argc )
        *filename = argv[i];
    else
    {
        printf("No file provided\n");
        printf("\n" USAGE_MESSAGE);
        return -1;
    }

    *args_start = ++i;

    return 0;
}
