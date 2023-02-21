#include <string.h>
#include "argv_parser.h"
#include "nest.h"

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

#define EXIT(code) \
    do { \
    _nst_consts_del(); \
    _nst_strs_del(); \
    _nst_types_del(); \
    _nst_streams_del(); \
    if ( filename != NULL ) { \
        free(src_text.text); \
        free(src_text.lines); \
        free(src_text.path); \
    } \
    return code; \
    } while (0)

#define ERROR_EXIT \
    do { \
    nst_print_error(error); \
    nst_dec_ref(error.name); \
    nst_dec_ref(error.message); \
    EXIT(1); \
    } while (0)

int main(int argc, char **argv)
{
#if defined(_WIN32) || defined(WIN32)
    SetErrorMode(SEM_FAILCRITICALERRORS);
    SetConsoleOutputCP(CP_UTF8);
#endif

#ifdef _DEBUG
    puts("**USING DEBUG BUILD - " NST_VERSION "**");

    for ( usize i = 0, n = strlen(NST_VERSION) + 24; i < n; i++ )
    {
        putc('-', stdout);
    }
    putc('\n', stdout);
    fflush(stdout);
#endif

    bool print_tokens;
    bool print_tree;
    bool print_bc;
    bool force_exe;
    bool monochrome;
    bool force_cp1252;
    bool no_default;
    i32 opt_level;
    i8 *command;
    i8 *filename;
    i32 args_start;

    i32 parse_result = nst_parse_args(
        argc, argv,
        &print_tokens,
        &print_tree,
        &print_bc,
        &force_exe,
        &monochrome,
        &force_cp1252,
        &no_default,
        &opt_level,
        &command,
        &filename,
        &args_start);

    nst_set_color(!monochrome);

    if ( parse_result == -1 )
    {
        return -1;
    }
    else if ( parse_result == 1 )
    {
        return 0;
    }

    _nst_types_init();
    _nst_strs_init();
    _nst_consts_init();
    _nst_streams_init();

    Nst_LList *tokens;
    Nst_Error error = { false, nst_no_pos(), nst_no_pos(), NULL, NULL };
    Nst_SourceText src_text = { NULL, NULL, 0 };

    if ( filename != NULL )
    {
        i32 spec_opt_lvl;
        bool spec_no_def;
        tokens = nst_tokenizef(
            filename,
            force_cp1252,
            &spec_opt_lvl,
            &spec_no_def,
            &src_text,
            &error);

        if ( spec_opt_lvl < opt_level )
        {
            opt_level = spec_opt_lvl;
        }
        if ( spec_no_def )
        {
            no_default = true;
        }
    }
    else
    {
        src_text.path = (i8 *)"<command>";
        src_text.len = strlen(command);
        src_text.text = command;
        src_text.line_count = 1;
        src_text.lines = &src_text.text;
        tokens = nst_tokenize(&src_text, &error);
    }

    if ( tokens == NULL )
    {
        if ( src_text.text == NULL )
        {
            EXIT(0);
        }
        else
        {
            ERROR_EXIT;
        }
    }

    if ( print_tokens )
    {
        for ( Nst_LLNode *n = tokens->head; n != NULL; n = n->next )
        {
            nst_print_tok(TOK(n->value));
            printf("\n");
        }

        if ( !force_exe && !print_tree && !print_bc )
        {
            nst_llist_destroy(tokens, (nst_llist_destructor)nst_token_destroy);
            EXIT(0);
        }
    }

    Nst_Node *ast = nst_parse(tokens, &error);

    if ( opt_level >= 1 && ast != NULL )
    {
        ast = nst_optimize_ast(ast, &error);
    }

    // nst_optimize_ast can delete the ast
    if ( ast == NULL )
    {
        ERROR_EXIT;
    }

    if ( print_tree )
    {
        if ( print_tokens ) printf("\n");
        nst_print_ast(ast);

        if ( !force_exe && !print_bc )
        {
            nst_node_destroy(ast);
            EXIT(0);
        }
    }

    // nst_compile never fails
    Nst_InstList *inst_ls = nst_compile(ast, false);

    if ( opt_level >= 2 )
    {
        inst_ls = nst_optimize_bytecode(
            inst_ls,
            opt_level == 3 && !no_default,
            &error);

        if ( inst_ls == NULL )
        {
            ERROR_EXIT;
        }
    }

    if ( print_bc )
    {
        if ( print_tokens || print_tree ) printf("\n");
        nst_print_bytecode(inst_ls, 0);

        if ( !force_exe )
        {
            nst_inst_list_destroy(inst_ls);
            EXIT(0);
        }
    }

    Nst_FuncObj *main_func = FUNC(nst_func_new(0, inst_ls));

    i32 exe_result = nst_run(
        main_func,
        argc - args_start,
        argv + args_start,
        filename,
        opt_level,
        no_default);

    EXIT(exe_result);
}
