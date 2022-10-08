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
#include "argv_parser.h"
#include "nest.h"

#if defined(_WIN32) || defined(WIN32)

#include <windows.h>
#define SET_UTF8_TERMINAL SetConsoleOutputCP(CP_UTF8)

#else

#include <wchar.h>
#include <locale.h>
#define SET_UTF8_TERMINAL setlocale(LC_ALL, "")

#endif

int main(int argc, char **argv)
{
    SET_UTF8_TERMINAL;

#ifdef _DEBUG
    puts("**USING DEBUG BUILD - " NEST_VERSION "**");
    puts("----------------------------------");
    fflush(stdout);
#endif

    bool print_tokens;
    bool print_tree;
    bool print_bc;
    bool force_exe;
    int opt_level;
    char *command;
    char *filename;
    int args_start;

    int parse_result = nst_parse_args(
        argc, argv,
        &print_tokens,
        &print_tree,
        &print_bc,
        &force_exe,
        &opt_level,
        &command,
        &filename,
        &args_start
    );

    if ( parse_result == -1 )
        return -1;
    else if ( parse_result == 1 )
        return 0;

    _nst_init_obj();

    char *text = NULL;
    LList *tokens;

    if ( filename != NULL )
        tokens = nst_ftokenize(filename, &text);
    else
        tokens = nst_tokenize(command, strlen(command), "<command>");

    if ( tokens == NULL )
        goto end;

    if ( print_tokens )
    {
        for ( LLNode *n = tokens->head; n != NULL; n = n->next )
        {
            nst_print_token(n->value);
            printf("\n");
        }

        if ( !force_exe && !print_tree && !print_bc )
        {
            LList_destroy(tokens, (LList_item_destructor)nst_destroy_token);
            goto end;
        }
    }

    Nst_Node *ast = nst_parse(tokens);

    if ( opt_level >= 1 && ast != NULL )
        ast = nst_optimize_ast(ast);

    // nst_optimize_ast can delete the ast
    if ( ast == NULL )
        goto end;

    if ( print_tree )
    {
        nst_print_ast(ast);

        if ( !force_exe && !print_bc )
        {
            nst_destroy_node(ast);
            goto end;
        }
    }

    // nst_compile never fails
    Nst_InstructionList *inst_ls = nst_compile(ast, false);

    if ( opt_level >= 2 )
        inst_ls = nst_optimize_bytecode(inst_ls, opt_level == 3);

    if ( inst_ls == NULL )
    {
        _nst_del_obj();
        return 0;
    }

    if ( print_bc )
    {
        nst_print_bytecode(inst_ls, 0);

        if ( !force_exe )
        {
            nst_destroy_inst_list(inst_ls);
            goto end;
        }
    }

    Nst_FuncObj *main_func = AS_FUNC(new_func(0));
    main_func->body = inst_ls;
    nst_run(
        main_func,
        argc - args_start,
        argv + args_start,
        filename != NULL ? filename : "-c",
        opt_level
    );

    end: _nst_del_obj();
    if ( text != NULL ) free(text);
    return 0;
}
