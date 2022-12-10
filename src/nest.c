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

#define EXIT(code) \
    do { \
    _nst_del_consts(); \
    _nst_del_strs(); \
    _nst_del_types(); \
    if ( filename != NULL ) { \
        free(src_text.text); \
        free(src_text.lines); \
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
#endif

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
    bool monochrome;
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
        &monochrome,
        &opt_level,
        &command,
        &filename,
        &args_start
    );

    nst_set_color(!monochrome);

    if ( parse_result == -1 )
        return -1;
    else if ( parse_result == 1 )
        return 0;

    _nst_init_types();
    _nst_init_strs();
    _nst_init_consts();

    LList *tokens;
    Nst_Error error = { false, nst_no_pos(), nst_no_pos(), NULL, NULL };
    Nst_SourceText src_text = { NULL, NULL, 0 };

    if ( filename != NULL )
        tokens = nst_ftokenize(filename, &src_text, &error);
    else
    {
        src_text.path = (char *)"<command>";
        src_text.len = strlen(command);
        src_text.text = command;
        src_text.line_count = 1;
        src_text.lines = &src_text.text;
        tokens = nst_tokenize(&src_text, &error);
    }

    if ( tokens == NULL )
    {
        if ( src_text.text == NULL )
            EXIT(0);
        else
            ERROR_EXIT;
    }

    if ( print_tokens )
    {
        for ( LLNode *n = tokens->head; n != NULL; n = n->next )
        {
            nst_print_token(TOK(n->value));
            printf("\n");
        }

        if ( !force_exe && !print_tree && !print_bc )
        {
            LList_destroy(tokens, (LList_item_destructor)nst_destroy_token);
            EXIT(0);
        }
    }

    Nst_Node *ast = nst_parse(tokens, &error);

    if ( opt_level >= 1 && ast != NULL )
        ast = nst_optimize_ast(ast, &error);

    // nst_optimize_ast can delete the ast
    if ( ast == NULL )
        ERROR_EXIT;

    if ( print_tree )
    {
        if ( print_tokens ) printf("\n");
        nst_print_ast(ast);

        if ( !force_exe && !print_bc )
        {
            nst_destroy_node(ast);
            EXIT(0);
        }
    }

    // nst_compile never fails
    Nst_InstructionList *inst_ls = nst_compile(ast, false);

    if ( opt_level >= 2 )
    {
        inst_ls = nst_optimize_bytecode(inst_ls, opt_level == 3, &error);

        if ( inst_ls == NULL )
            ERROR_EXIT;
    }

    if ( print_bc )
    {
        if ( print_tokens || print_tree ) printf("\n");
        nst_print_bytecode(inst_ls, 0);

        if ( !force_exe )
        {
            nst_destroy_inst_list(inst_ls);
            EXIT(0);
        }
    }

    Nst_FuncObj *main_func = FUNC(new_func(0, inst_ls));

    int exe_result = nst_run(
        main_func,
        argc - args_start,
        argv + args_start,
        filename,
        opt_level
    );

    EXIT(exe_result);
}
