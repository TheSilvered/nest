#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "obj.h"
#include "llist.h"
#include "tokens.h"
#include "compiler.h"
#include "optimizer.h"

#define VERSION "beta-0.2.2"

int main(int argc, char **argv)
{
#ifdef _DEBUG
    puts("USING DEBUG BUILD");
    fflush(stdout);
#endif

    if ( argc < 2 )
    {
        printf("USAGE: nest [compilation-options] <filename>\n"
               "     : nest <filename> [args]\n"
               "     : nest <options>\n"
        );
        return -1;
    }

    if ( argc == 2 && (strcmp(argv[1], "-h")     == 0
                   ||  strcmp(argv[1], "--help") == 0) )
    {
        printf("USAGE: nest [compilation-options] <filename>\n"
               "     : nest <filename> [args]\n"
               "     : nest <options>\n\n"

               "Filename:\n"
               "  The name of the file you wish to compile and run\n\n"

               "Compilation options:\n"
               "  -t --tokens : prints the list of tokens of the program\n"
               "  -a --ast    : prints the abstract syntax tree of the program\n\n"

               "Args:\n"
               "  the arguments that will be accessible throgh _args_ during execution\n\n"

               "Options:\n"
               "  -h --help   : prints this message\n"
               "  -v --version: prints the version of nest being used\n"
        );
        return 0;
    }
    else if ( argc == 2 && (strcmp(argv[1], "-v")        == 0
                        ||  strcmp(argv[1], "--version") == 0) )
    {
        printf("Using nest version: " VERSION);
        return 0;
    }

    init_obj();

    char *file_name = NULL;
    bool print_tokens = false;
    bool print_tree = false;
    bool print_bc = false;

    if ( strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "--tokens") == 0 )
    {
        if ( argc < 3 )
        {
            printf("USAGE: nest [compilation-options] <filename>\n"
                "     : nest <filename> [args]\n"
                "     : nest <options>\n"
            );
            del_obj();
            return -1;
        }
        print_tokens = true;
        file_name = argv[2];
    }
    else if ( strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--ast") == 0 )
    {
        if ( argc < 3 )
        {
            printf("USAGE: nest [compilation-options] <filename>\n"
                "     : nest <filename> [args]\n"
                "     : nest <options>\n"
            );
            del_obj();
            return -1;
        }
        print_tree = true;
        file_name = argv[2];
    }
    else if ( strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--bytecode") == 0 )
    {
        if ( argc < 3 )
        {
            printf("USAGE: nest [compilation-options] <filename>\n"
                "     : nest <filename> [args]\n"
                "     : nest <options>\n"
            );
            del_obj();
            return -1;
        }
        print_bc = true;
        file_name = argv[2];
    }
    else
        file_name = argv[1];

    LList *tokens = nst_ftokenize(file_name);

    if ( tokens == NULL )
        return 0;

    if ( print_tokens )
    {
        for ( LLNode *n = tokens->head; n != NULL; n = n->next )
        {
            nst_print_token(n->value);
            printf("\n");
        }
        del_obj();
        return 0;
    }

    Nst_Node *ast = nst_parse(tokens);
    if ( ast != NULL )
        ast = nst_optimize_ast(ast);

    if ( ast == NULL )
    {
        del_obj();
        return 0;
    }

    if ( print_tree )
    {
        nst_print_ast(ast);
        del_obj();
        return 0;
    }

    Nst_InstructionList *inst_ls = nst_compile(ast);

    if ( inst_ls == NULL )
    {
        nst_destroy_node(ast);
        del_obj();
        return 0;
    }

    if ( print_bc )
    {
        nst_print_bytecode(inst_ls, 0);
        del_obj();
        return 0;
    }

    nst_run(ast, argc, argv);
    nst_destroy_node(ast);

    del_obj();

    return 0;
}
