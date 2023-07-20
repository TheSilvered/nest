#include <string.h>
#include <stdlib.h>
#include "nest.h"

#ifdef Nst_WIN
#include <windows.h>
#endif

#ifdef _EXIT
#undef _EXIT
#endif

#define _EXIT(code) \
    do { \
    _Nst_del_objects(); \
    _Nst_unload_libs(); \
    if ( filename != NULL ) { \
        Nst_free(src_text.text); \
        Nst_free(src_text.lines); \
        Nst_free(src_text.path); \
    } \
    return code; \
    } while ( 0 )

#ifdef Nst_WIN

#define EXIT(code) \
    do { \
        Nst_free(argv); \
        _EXIT(code); \
    } while ( 0 )

#else

#define EXIT _EXIT

#endif

#define ERROR_EXIT \
    do { \
    Nst_print_error(error); \
    Nst_dec_ref(error.name); \
    Nst_dec_ref(error.message); \
    EXIT(1); \
    } while ( 0 )

#ifdef Nst_WIN

int wmain(int argc, wchar_t **wargv)
{
    _Nst_set_console_mode();

    char **argv;

    if ( !_Nst_wargv_to_argv(argc, wargv, &argv) )
    {
        return -1;
    }

#else
int main(int argc, char **argv)
{
#endif

#ifdef _DEBUG
    puts("**USING DEBUG BUILD - " Nst_VERSION "**");

    for ( usize i = 0, n = strlen(Nst_VERSION) + 24; i < n; i++ )
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
    Nst_CPID encoding;
    bool no_default;
    i32 opt_level;
    i8 *command;
    i8 *filename;
    i32 args_start;

    i32 parse_result = _Nst_parse_args(
        argc, argv,
        &print_tokens,
        &print_tree,
        &print_bc,
        &force_exe,
        &encoding,
        &no_default,
        &opt_level,
        &command,
        &filename,
        &args_start);

    Nst_set_color(Nst_supports_color());

    if ( parse_result == -1 )
    {
        return -1;
    }
    else if ( parse_result == 1 )
    {
        return 0;
    }

    if (!_Nst_init_objects())
    {
#ifdef Nst_WIN
        Nst_free(argv);
#endif
        printf("Failed allocation\n");
        return -1;
    }

    Nst_LList *tokens;
    Nst_Error error = { false, Nst_no_pos(), Nst_no_pos(), NULL, NULL };
    Nst_SourceText src_text = { NULL, NULL, NULL, 0, 0 };

    if ( filename != NULL )
    {
        i32 spec_opt_lvl;
        bool spec_no_def;
        tokens = Nst_tokenizef(
            filename,
            encoding,
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
        tokens = Nst_tokenize(&src_text, &error);
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
            Nst_print_tok(Nst_TOK(n->value));
            printf("\n");
        }

        if ( !force_exe && !print_tree && !print_bc )
        {
            Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
            EXIT(0);
        }
    }

    Nst_Node *ast = Nst_parse(tokens, &error);

    if ( opt_level >= 1 && ast != NULL )
    {
        ast = Nst_optimize_ast(ast, &error);
    }

    // Nst_optimize_ast can delete the ast
    if ( ast == NULL )
    {
        ERROR_EXIT;
    }

    if ( print_tree )
    {
        if ( print_tokens ) printf("\n");
        Nst_print_ast(ast);

        if ( !force_exe && !print_bc )
        {
            Nst_node_destroy(ast);
            EXIT(0);
        }
    }

    // nst_compile never fails
    Nst_InstList *inst_ls = Nst_compile(ast, false, &error);

    if ( opt_level >= 2 && inst_ls != NULL )
    {
        bool optimize_builtins = opt_level == 3 && !no_default;
        inst_ls = Nst_optimize_bytecode(inst_ls, optimize_builtins, &error);
    }
    if ( inst_ls == NULL )
    {
        ERROR_EXIT;
    }

    if ( print_bc )
    {
        if ( print_tokens || print_tree )
        {
            printf("\n");
        }
        Nst_print_bytecode(inst_ls);

        if ( !force_exe )
        {
            Nst_inst_list_destroy(inst_ls);
            EXIT(0);
        }
    }

    Nst_FuncObj *main_func = FUNC(Nst_func_new(0, inst_ls));

    i32 exe_result = Nst_run(
        main_func,
        argc - args_start,
        argv + args_start,
        filename,
        opt_level,
        no_default);

    EXIT(exe_result);
}
