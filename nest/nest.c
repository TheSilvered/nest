#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "obj.h"
#include "llist.h"
#include "tokens.h"

#define VERSION "beta-0.1.2"

int main(int argc, char **argv)
{
#ifdef _DEBUG
    puts("USING DEBUG BUILD");
    fflush(stdout);
#endif

    if ( argc < 2 )
    {
        printf("USAGE: nest <filename>\n");
        printf("     : nest <options>\n");
        return -1;
    }

    if ( argc == 2 && (strcmp(argv[1], "-h")     == 0
                   ||  strcmp(argv[1], "--help") == 0) )
    {
        printf("USAGE: nest [compilation-options] <filename>\n"
               "     : nest <options>\n\n"

               "Filename:\n"
               "  The name of the file you wish to compile and run\n\n"

               "Compilation options:\n"
               "  -t --tokens : prints the list of tokens of the program\n"
               "  -a --ast    : prints the abstract syntax tree of the program\n\n"

               "Options:\n"
               "  -h --help   : prints this message\n"
               "  -v --version: prints the version on nest being used\n"
        );
        return 0;
    }
    else if ( argc == 2 && (strcmp(argv[1], "-v")        == 0
                        ||  strcmp(argv[1], "--version") == 0) )
    {
        printf("Using Nest version " VERSION);
        return 0;
    }

    init_obj();

    LList *tokens = ftokenize(argv[1]);

    if ( tokens == NULL )
        return 1;

    Node *ast = parse(tokens);

    if ( ast == NULL )
        return 2;

    run(ast, argc, argv);
    destroy_node(ast);

    del_obj();

    return 0;
}
